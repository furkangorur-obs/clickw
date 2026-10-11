#include <QtWidgets>
#include <QtDBus>
#include <KGlobalAccel>
#include <LayerShellQt/Window>
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>
#include <linux/input.h>
#include <memory>
#include <vector>
#include <unistd.h>
#include <errno.h>

// Only touch events are emitted. The physical pointer is never warped or grabbed.
class TouchDevice {
    libevdev *dev = nullptr;
    libevdev_uinput *ui = nullptr;
    int tracking = 0;
public:
    QString output, deviceName;
    bool mapped = false;
    TouchDevice(const QString &screen) : output(screen), deviceName("ClickW Touch " + screen + " " + QString::number(QCoreApplication::applicationPid())) {
        dev = libevdev_new();
        if (!dev) throw std::runtime_error("Giriş aygıtı oluşturulamadı.");
        libevdev_set_name(dev, deviceName.toUtf8().constData());
        libevdev_set_id_bustype(dev, BUS_USB);
        libevdev_set_id_vendor(dev, 0x1209);
        libevdev_set_id_product(dev, 0xc174);
        libevdev_enable_property(dev, INPUT_PROP_DIRECT);
        libevdev_enable_event_code(dev, EV_KEY, BTN_TOUCH, nullptr);
        auto axis = [this](int code, int max) {
            input_absinfo info{}; info.maximum = max; info.resolution = 100; if(code==ABS_MT_TRACKING_ID)info.value=-1;
            libevdev_enable_event_code(dev, EV_ABS, code, &info);
        };
        axis(ABS_X, 65535); axis(ABS_Y, 65535);
        axis(ABS_MT_SLOT, 0); axis(ABS_MT_TRACKING_ID, 65535);
        axis(ABS_MT_POSITION_X, 65535); axis(ABS_MT_POSITION_Y, 65535);
        int rc = libevdev_uinput_create_from_device(dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &ui);
        if (rc < 0) { libevdev_free(dev); dev = nullptr; throw std::runtime_error("/dev/uinput açılamadı. Sanal dokunmatik giriş izni gerekli."); }
    }
    ~TouchDevice() { if (ui) libevdev_uinput_destroy(ui); if (dev) libevdev_free(dev); }
    void write(unsigned type, unsigned code, int value) {
        if (libevdev_uinput_write_event(ui, type, code, value) < 0)
            throw std::runtime_error("Dokunma gönderilemedi.");
    }
    bool map() {
        QDBusInterface manager("org.kde.KWin", "/org/kde/KWin/InputDevice", "org.kde.KWin.InputDeviceManager");
        QStringList names = manager.property("devicesSysNames").toStringList();
        for (const auto &name : names) {
            QDBusInterface d("org.kde.KWin", "/org/kde/KWin/InputDevice/" + name, "org.kde.KWin.InputDevice");
            if (d.property("name").toString() != deviceName) continue;
            if (!d.setProperty("outputName", output) || !d.setProperty("mapToWorkspace", false)) return false;
            mapped = d.property("outputName").toString() == output && !d.property("mapToWorkspace").toBool() && d.property("touch").toBool();
            return mapped;
        }
        return false;
    }
    void appendTap(std::vector<input_event> &events,const QPoint &point,const QRect &screen) {
        if(!mapped)throw std::runtime_error("Monitör eşlemesi hazır değil.");
        auto event=[&events](unsigned short type,unsigned short code,int value){input_event e{};e.type=type;e.code=code;e.value=value;events.push_back(e);};
        int x=qBound(0,qRound((point.x()-screen.x())*65535.0/screen.width()),65535);
        int y=qBound(0,qRound((point.y()-screen.y())*65535.0/screen.height()),65535);
        event(EV_KEY,BTN_TOUCH,1);event(EV_ABS,ABS_MT_SLOT,0);
        event(EV_ABS,ABS_MT_TRACKING_ID,(++tracking)%65535);
        event(EV_ABS,ABS_MT_POSITION_X,x);event(EV_ABS,ABS_MT_POSITION_Y,y);
        event(EV_ABS,ABS_X,x);event(EV_ABS,ABS_Y,y);event(EV_SYN,SYN_REPORT,0);
        event(EV_KEY,BTN_TOUCH,0);event(EV_ABS,ABS_MT_SLOT,0);
        event(EV_ABS,ABS_MT_TRACKING_ID,-1);event(EV_SYN,SYN_REPORT,0);
    }
    void sendBatch(const std::vector<input_event> &events) {
        const char *data=reinterpret_cast<const char*>(events.data());
        size_t left=events.size()*sizeof(input_event);
        while(left) {
            ssize_t written=::write(libevdev_uinput_get_fd(ui),data,left);
            if(written<0 && errno==EINTR)continue;
            if(written<=0)throw std::runtime_error("Tıklama paketi gönderilemedi.");
            data+=written;left-=size_t(written);
        }
    }
    void up() {
        write(EV_KEY, BTN_TOUCH, 0);
        write(EV_ABS,ABS_MT_SLOT,0);write(EV_ABS,ABS_MT_TRACKING_ID,-1);
        write(EV_SYN, SYN_REPORT, 0);
    }
};

static constexpr int MaxMarkers=20;
struct Marker {
    static constexpr int size=24, half=12;
    int number;
    QPoint center, local;
    QPointer<QScreen> output;
    QString outputName;
    bool locked=false, visible=true;
    explicit Marker(int n):number(n){}
    void place(QPoint p) {
        auto *s=QGuiApplication::screenAt(p);
        if(!s) {
            int distance=INT_MAX;
            for(auto *candidate:QGuiApplication::screens()) {
                auto r=candidate->geometry();
                QPoint nearest(qBound(r.left(),p.x(),r.right()),qBound(r.top(),p.y(),r.bottom()));
                int d=(nearest-p).manhattanLength();if(d<distance){distance=d;s=candidate;}
            }
        }
        if(!s)return;
        auto r=s->geometry();
        center=QPoint(qBound(r.left()+half,p.x(),r.right()-half+1),qBound(r.top()+half,p.y(),r.bottom()-half+1));
        output=s;outputName=s->name();local=center-r.topLeft();
    }
};

// Fixed surfaces: pointer positions remain relative to the same output throughout
// a drag, even when the marker crosses to another output. No surface is moved.
class Canvas : public QWidget {
public:
    QPointer<QScreen> output;
    std::function<std::vector<Marker*>()> points;
    std::function<bool()> inputEnabled;
    std::function<void()> changed, saved;
    Marker *drag=nullptr;
    QPoint offset;
    Canvas(QScreen *s):output(s) {
        setWindowFlags(Qt::FramelessWindowHint|Qt::Window|Qt::WindowDoesNotAcceptFocus);
        setAttribute(Qt::WA_TranslucentBackground);setAttribute(Qt::WA_ShowWithoutActivating);
        setStyleSheet("background:transparent");setCursor(Qt::SizeAllCursor);
        setFixedSize(s->geometry().size());winId();windowHandle()->setScreen(s);
        auto *layer=LayerShellQt::Window::get(windowHandle());
        layer->setScope("clickw-markers");layer->setScreen(s);
        layer->setLayer(LayerShellQt::Window::LayerOverlay);
        layer->setAnchors(LayerShellQt::Window::Anchors(LayerShellQt::Window::AnchorTop)|LayerShellQt::Window::AnchorLeft);
        layer->setExclusiveZone(-1);layer->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityNone);
        layer->setActivateOnShow(false);
    }
    void refresh() {
        QRegion region;
        if(points && inputEnabled && inputEnabled())for(auto *m:points())if(m->visible && m->output==output)
            region+=QRect(m->local-QPoint(Marker::half,Marker::half),QSize(Marker::size,Marker::size));
        // Empty QRegion resets the mask to the whole window: use an out-of-bounds
        // region instead, so an empty overlay never intercepts desktop input.
        if(region.isEmpty())region=QRegion(QRect(-10,-10,1,1));
        windowHandle()->setMask(region);update();
    }
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);p.setCompositionMode(QPainter::CompositionMode_Source);p.fillRect(rect(),Qt::transparent);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);p.setRenderHint(QPainter::Antialiasing);
        if(!points)return;
        for(auto *m:points())if(m->visible && m->output==output) {
            QPoint a=m->local; p.setPen(QPen(Qt::white,5,Qt::SolidLine,Qt::RoundCap));
            p.drawLine(a-QPoint(9,0),a+QPoint(9,0));p.drawLine(a-QPoint(0,9),a+QPoint(0,9));
            p.setPen(QPen(QColor("#ff2020"),3,Qt::SolidLine,Qt::RoundCap));
            p.drawLine(a-QPoint(9,0),a+QPoint(9,0));p.drawLine(a-QPoint(0,9),a+QPoint(0,9));
        }
    }
    void mousePressEvent(QMouseEvent *e) override {
        if(!output || !inputEnabled() || e->button()!=Qt::LeftButton)return;
        QPoint position=output->geometry().topLeft()+e->position().toPoint();
        for(auto *m:points())if(m->visible && !m->locked && m->output==output && QRect(m->center-QPoint(12,12),QSize(24,24)).contains(position)) {
            drag=m;offset=position-m->center;break;
        }
    }
    void mouseMoveEvent(QMouseEvent *e) override {
        if(drag && output) {drag->place(output->geometry().topLeft()+e->position().toPoint()-offset);if(changed)changed();}
    }
    void mouseReleaseEvent(QMouseEvent*) override {if(drag){drag=nullptr;if(saved)saved();}}
};

class Panel : public QWidget {
public:
    QSettings settings;
    std::vector<std::unique_ptr<TouchDevice>> devices;
    std::vector<std::unique_ptr<Marker>> markers;
    std::vector<std::unique_ptr<Canvas>> canvases;
    QTimer topology;
    QLabel *status;
    QAction *action;
    QKeySequenceEdit *keys;
    QCheckBox *lock;
    QCheckBox *visible;
    QPushButton *create;
    QTimer mapping;
    bool busy=false, ready=false, closing=false;
    QPointer<QThread> worker;
    qint64 lastBatchNs=0;
    int mapTries=0;
    std::function<void()> completed;
    Panel(bool test=false) {
        setWindowTitle("ClickW · 20 Nokta"); setMinimumSize(420,550); resize(420,550);
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(16,16,16,16); layout->setSpacing(8);
        auto *title = new QLabel("<h2>＋ ClickW</h2>"); layout->addWidget(title);
        title->setMinimumHeight(42);
        auto *help = new QLabel("+ oluştur ile en fazla 20 kırmızı artı ekle.\nArtıları sürükle; kısayol tüm noktaları çok hızlı tıklar.\nMouse imlecinin konumu değiştirilmez.");
        help->setWordWrap(true); help->setMinimumHeight(60); layout->addWidget(help);
        layout->addWidget(new QLabel("Tıklama kısayolu"));
        keys=new QKeySequenceEdit(QKeySequence(settings.value("shortcut","F8").toString()));
        keys->setMaximumSequenceLength(1); layout->addWidget(keys);
        auto *apply=new QPushButton("Kısayolu kaydet"); layout->addWidget(apply);
        lock=new QCheckBox("Noktaları kilitle"); layout->addWidget(lock);
        visible=new QCheckBox("Artı işaretlerini göster"); visible->setChecked(true); layout->addWidget(visible);
        create=new QPushButton("+ oluştur (0 / 20)"); layout->addWidget(create);
        auto *clear=new QPushButton("Son artıyı sil"); layout->addWidget(clear);
        status=new QLabel("Dokunmatik giriş hazırlanıyor…"); status->setWordWrap(true); layout->addWidget(status);
        auto *note=new QLabel("Tıklamalar bekleme eklenmeden tek paketle gönderilir. Mouse imleci hareket ettirilmez.");
        note->setWordWrap(true); note->setStyleSheet("color:#9aa6b2;font-size:11px"); layout->addWidget(note);
        auto *minimize=new QPushButton("Paneli küçült"); layout->addWidget(minimize);
        connect(minimize,&QPushButton::clicked,this,&QWidget::showMinimized);
        auto *quit=new QPushButton("Kapat"); layout->addWidget(quit);
        action=new QAction("Noktalara dokun",this); action->setObjectName("tap-four"); action->setAutoRepeat(false);
        if(test) action->setProperty("componentName","clickw-selftest");
        connect(action,&QAction::triggered,this,[this]{ trigger(); });
        connect(apply,&QPushButton::clicked,this,[this]{ setShortcut(); });
        connect(quit,&QPushButton::clicked,this,&QWidget::close);
        connect(qApp,&QCoreApplication::aboutToQuit,this,[this]{shutdown();});
        connect(lock,&QCheckBox::toggled,this,[this](bool v){ for(auto &m:markers)m->locked=v;refreshCanvases(); });
        connect(visible,&QCheckBox::toggled,this,[this](bool v){ for(auto &m:markers)m->visible=v;refreshCanvases(); });
        connect(create,&QPushButton::clicked,this,[this]{addPoint();});
        connect(clear,&QPushButton::clicked,this,[this]{
            if(busy || markers.empty())return;
            auto *last=markers.back().get();
            for(auto &c:canvases)if(c->drag==last)c->drag=nullptr;
            markers.pop_back();savePoints();updateCreate();refreshCanvases();
            status->setText(QString("Son artı silindi · %1 / 20").arg(markers.size()));
        });
        if(test) for(int i=0;i<MaxMarkers;i++) addPoint(false);
        else for(int i=0, count=qBound(0,settings.value("markerCount",0).toInt(),MaxMarkers);i<count;i++) {
            addPoint(false); auto &m=*markers.back();m.place(settings.value(QString("point%1").arg(i),m.center).toPoint());
            const QString name=settings.value(QString("screen%1").arg(i)).toString();
            for(auto *s:QGuiApplication::screens())if(s->name()==name)m.place(s->geometry().topLeft()+settings.value(QString("local%1").arg(i),m.local).toPoint());
        }
        connect(&mapping,&QTimer::timeout,this,[this]{
            ready=!devices.empty(); for(auto &d:devices) if(!d->mapped) ready=d->map() && ready;
            if(ready) {mapping.stop(); auto shortcuts=KGlobalAccel::self()->shortcut(action); status->setText(shortcuts.isEmpty() ? "Giriş hazır. Bir kısayol kaydet." : "Hazır · "+shortcuts.first().toString());}
            else if(++mapTries>30) {mapping.stop();status->setText("Monitör eşlemesi kurulamadı. Programı yeniden aç.");}
        }); mapping.start(100);
        if(!test) setShortcut();
        topology.setSingleShot(true);
        connect(&topology,&QTimer::timeout,this,[this]{if(busy){topology.start(75);return;}syncScreens();});
        auto schedule=[this]{ready=false;topology.start(75);};
        connect(qApp,&QGuiApplication::screenAdded,this,[this,schedule](QScreen *s){watchScreen(s);schedule();});
        connect(qApp,&QGuiApplication::screenRemoved,this,[schedule](QScreen*){schedule();});
        for(auto *s:QGuiApplication::screens())watchScreen(s);
        syncScreens();

    }
    ~Panel() { shutdown(); }
    void shutdown() {
        if(closing)return;
        closing=true;ready=false;mapping.stop();topology.stop();
        KGlobalAccel::self()->removeAllShortcuts(action);
        canvases.clear();savePoints();settings.sync();
        if(worker){worker->wait();delete worker;worker=nullptr;}
        devices.clear();
    }
    void closeEvent(QCloseEvent *event) override {
        shutdown();event->accept();QCoreApplication::quit();
    }
    void updateCreate() {create->setText(QString("+ oluştur (%1 / 20)").arg(markers.size()));create->setEnabled(markers.size()<MaxMarkers && !busy);}
    void addPoint(bool save=true) {
        if(busy || markers.size()>=MaxMarkers)return;
        auto *screen=this->screen();
        QDBusInterface kwin("org.kde.KWin","/KWin","org.kde.KWin");
        QDBusReply<QString> active=kwin.call("activeOutputName");
        for(auto *s:QGuiApplication::screens())if(active.isValid() && s->name()==active.value())screen=s;
        auto m=std::make_unique<Marker>(int(markers.size())+1);
        m->place(screen->geometry().topLeft()+QPoint(120+(int(markers.size())%6)*50,120+(int(markers.size())/6)*50));
        m->locked=lock->isChecked();markers.push_back(std::move(m));visible->setChecked(true);refreshCanvases();
        if(save)savePoints();
        updateCreate();
        status->setText(QString("Kırmızı artı oluşturuldu: %1 / 20 · Ekranın sol üstünde").arg(markers.size()));
    }
    void refreshCanvases(){for(auto &c:canvases)c->refresh();}
    void watchScreen(QScreen *s){connect(s,&QScreen::geometryChanged,this,[this]{ready=false;topology.start(75);});}
    void syncScreens() {
        ready=false;mapping.stop();canvases.clear();devices.clear();
        for(auto &m:markers) {
            QScreen *s=nullptr;for(auto *candidate:QGuiApplication::screens())if(candidate->name()==m->outputName)s=candidate;
            if(!s)s=QGuiApplication::primaryScreen();
            m->place(s->geometry().topLeft()+m->local);
        }
        for(auto *s:QGuiApplication::screens()) {
            auto c=std::make_unique<Canvas>(s);
            c->points=[this]{std::vector<Marker*> list;for(auto &m:markers)list.push_back(m.get());return list;};
            c->inputEnabled=[this]{return !busy;};c->changed=[this]{refreshCanvases();};c->saved=[this]{savePoints();refreshCanvases();};
            c->refresh();c->show();canvases.push_back(std::move(c));
        }
        try{for(auto *s:QGuiApplication::screens())devices.push_back(std::make_unique<TouchDevice>(s->name()));}
        catch(const std::exception &e){status->setText(e.what());devices.clear();return;}
        mapTries=0;mapping.start(100);
    }
    void savePoints() {settings.setValue("markerCount",int(markers.size()));for(size_t i=0;i<markers.size();i++) {
        settings.setValue(QString("point%1").arg(i),markers[i]->center);
        settings.setValue(QString("screen%1").arg(i),markers[i]->outputName);
        settings.setValue(QString("local%1").arg(i),markers[i]->local);
    }}
    void setShortcut() {
        QKeySequence seq=keys->keySequence();
        if(seq.isEmpty()) {status->setText("Önce bir kısayol gir.");return;}
        auto combination=seq[0];
        if(combination.keyboardModifiers().testFlag(Qt::ControlModifier) && combination.keyboardModifiers().testFlag(Qt::AltModifier) && combination.key()>=Qt::Key_F1 && combination.key()<=Qt::Key_F12) {
            status->setText("Ctrl+Alt+F tuşları sistem konsoluna ayrılmıştır. Başka bir kısayol seç.");return;
        }
        if(!KGlobalAccel::isGlobalShortcutAvailable(seq,action->property("componentName").toString().isEmpty() ? "clickw" : action->property("componentName").toString())) {status->setText("Bu kısayol başka bir uygulamada kullanılıyor.");return;}
        if(!KGlobalAccel::self()->setShortcut(action,{seq},KGlobalAccel::NoAutoloading)) {status->setText("Kısayol kaydedilemedi.");return;}
        settings.setValue("shortcut",seq.toString()); status->setText("Kısayol kaydedildi · "+seq.toString());
    }
    void restore() {busy=false;refreshCanvases();updateCreate();}
    void trigger() {
        if(busy || closing) return;
        if(markers.empty()){status->setText("Önce + oluştur ile bir artı ekle.");return;}
        if(!ready) {status->setText("Dokunmatik giriş henüz hazır değil.");return;}
        for(auto &c:canvases)if(c->drag)return;
        busy=true;updateCreate();refreshCanvases();
        // Only the overlay input-region commit needs settling; there are no
        // per-point timers, sleeps, or GUI event-loop hops in the input burst.
        QTimer::singleShot(35,Qt::PreciseTimer,this,[this]{tapBatch();});
    }
    void tapBatch() {
        if(closing)return;
        if(!ready){status->setText("Monitör düzeni değişti; tekrar dene.");restore();return;}
        struct Packet {TouchDevice *device;std::vector<input_event> events;};
        std::vector<Packet> packets;
        try {
            for(auto &d:devices) {
                Packet packet{d.get(),{}};packet.events.reserve(markers.size()*12);
                for(auto &m:markers)if(m->output && m->output->name()==d->output)d->appendTap(packet.events,m->center,m->output->geometry());
                if(!packet.events.empty())packets.push_back(std::move(packet));
            }
        }catch(const std::exception &e){status->setText(e.what());restore();return;}
        auto error=std::make_shared<QString>();auto elapsed=std::make_shared<qint64>(0);
        worker=QThread::create([packets=std::move(packets),error,elapsed]{
            QElapsedTimer timer;timer.start();
            try {for(auto &p:packets)p.device->sendBatch(p.events);}
            catch(const std::exception &e){*error=QString::fromUtf8(e.what());for(auto &p:packets)try{p.device->up();}catch(...){} }
            *elapsed=timer.nsecsElapsed();
        });
        connect(worker,&QThread::finished,this,[this,error,elapsed]{
            if(closing)return;
            lastBatchNs=*elapsed;
            status->setText(error->isEmpty() ? QString("%1 noktaya tıklama paketi gönderildi.").arg(markers.size()) : *error);
            // Keep overlays click-through while KWin consumes the already
            // queued burst. This wait does not space out any input events.
            QTimer::singleShot(100,Qt::PreciseTimer,this,[this]{if(closing)return;restore();if(completed)completed();});
        });
        connect(worker,&QThread::finished,worker,&QObject::deleteLater);
        worker->start();
    }
};

// Real compositor integration check; generated touches only target these private test windows.
class TestTarget : public QWidget {
public:
    int touches=0, clicks=0, maxContacts=0;
    QList<QPoint> hits;
    QList<qint64> times;
    QElapsedTimer clock;
    QPoint origin;
    TestTarget(QScreen *s, bool mouseTest=false) {
        clock.start();origin=s->geometry().topLeft()+QPoint(80,80);
        setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowDoesNotAcceptFocus);
        setAttribute(Qt::WA_AcceptTouchEvents,!mouseTest);
        resize(600,560); winId(); windowHandle()->setScreen(s);
        auto *l=LayerShellQt::Window::get(windowHandle());
        l->setScreen(s); l->setScope("clickw-test"); l->setLayer(LayerShellQt::Window::LayerOverlay);
        l->setAnchors(LayerShellQt::Window::Anchors(LayerShellQt::Window::AnchorTop)|LayerShellQt::Window::AnchorLeft);
        l->setMargins(QMargins(80,80,0,0)); l->setExclusiveZone(-1);
        l->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityNone);
        show();
    }
    bool event(QEvent *e) override {
        if(e->type()==QEvent::TouchBegin || e->type()==QEvent::TouchUpdate) {
            auto *t=static_cast<QTouchEvent*>(e);
            for(auto &p:t->points())if(p.state()==QEventPoint::State::Pressed){touches++;hits<<origin+p.position().toPoint();times<<clock.nsecsElapsed();}
            maxContacts=qMax(maxContacts,int(t->points().size()));
            e->accept();update();return true;
        }
        if(e->type()==QEvent::TouchUpdate || e->type()==QEvent::TouchEnd) {e->accept();return true;}
        return QWidget::event(e);
    }
    void mouseReleaseEvent(QMouseEvent *e) override {if(e->source()==Qt::MouseEventNotSynthesized)return; clicks++;hits<<origin+e->position().toPoint();times<<clock.nsecsElapsed();update();}
    void paintEvent(QPaintEvent*) override {QPainter p(this);p.fillRect(rect(),QColor("#183746"));p.setPen(Qt::white);p.drawText(rect(),Qt::AlignCenter,QString("ClickW kontrollü test alanı\nDokunma: %1 · Tık: %2").arg(touches).arg(clicks));}
};

int main(int argc,char **argv) {
    qInstallMessageHandler([](QtMsgType, const QMessageLogContext &, const QString &message){ fprintf(stderr,"%s\n",message.toUtf8().constData()); });
    QApplication app(argc,argv); app.setApplicationName("clickw");app.setOrganizationName("ClickW");
    app.setApplicationDisplayName("ClickW");
    app.setStyleSheet("QWidget{background:#17212b;color:#edf4fa;font-size:13px} QPushButton{background:#28394a;border:1px solid #3f5366;border-radius:7px;padding:8px} QPushButton:hover{background:#355065} QKeySequenceEdit{padding:8px;border:1px solid #55768c;border-radius:6px} QCheckBox{padding:5px}");
    bool test=app.arguments().contains("--self-test");
    bool mouseTest=app.arguments().contains("--mouse-test");
    bool shortcutTest=app.arguments().contains("--shortcut-test");
    bool closeTest=app.arguments().contains("--close-test");
    bool multiOutputTest=app.arguments().contains("--multi-output-test");
    QLockFile singleton(QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation)+"/clickw.lock");
    if(!singleton.tryLock()) {QMessageBox::information(nullptr,"ClickW","ClickW zaten açık.");return 1;}
    if(test || closeTest) {QSettings::setPath(QSettings::NativeFormat,QSettings::UserScope,QDir::tempPath()+"/clickw-selftest-"+QString::number(app.applicationPid()));}
    Panel panel(test || closeTest);
    if(closeTest){panel.show();QTimer::singleShot(500,&panel,[&]{panel.close();qInfo()<<"CLOSE TEST"<<(panel.canvases.empty() && panel.devices.empty() && panel.closing ? "PASS" : "FAIL");});}
    std::vector<std::unique_ptr<TestTarget>> targets;
    if(test) {
        if(multiOutputTest)for(auto *s:app.screens())targets.push_back(std::make_unique<TestTarget>(s,mouseTest));
        else targets.push_back(std::make_unique<TestTarget>(app.primaryScreen(),mouseTest));
        for(int i=0;i<MaxMarkers;i++) {
            auto *s=multiOutputTest ? app.screens()[i%app.screens().size()] : app.primaryScreen();
            int index=multiOutputTest ? i/app.screens().size() : i;
            panel.markers[i]->place(s->geometry().topLeft()+QPoint(180+(index%4)*85,180+(index/4)*80));
        }
        panel.refreshCanvases();
        auto *marker=panel.markers[0].get();QPoint before=marker->center;
        Canvas *canvas=nullptr;for(auto &c:panel.canvases)if(c->output==marker->output)canvas=c.get();
        QPoint pressPoint=marker->local;
        QMouseEvent press(QEvent::MouseButtonPress,QPointF(pressPoint),QPointF(pressPoint),Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
        QApplication::sendEvent(canvas,&press);
        for(int step=1;step<=10;step++) {
            QPoint pos=pressPoint+QPoint(step*2,step);
            QMouseEvent move(QEvent::MouseMove,QPointF(pos),QPointF(pos),Qt::NoButton,Qt::LeftButton,Qt::NoModifier);
            QApplication::sendEvent(canvas,&move);
        }
        QMouseEvent release(QEvent::MouseButtonRelease,QPointF(pressPoint),QPointF(pressPoint),Qt::LeftButton,Qt::NoButton,Qt::NoModifier);
        QApplication::sendEvent(canvas,&release);
        if(marker->center!=before+QPoint(20,10) || !canvas->isVisible()) {qWarning()<<"DRAG FAIL";return 5;}
        if(app.screens().size()>1) {
            auto *other=app.screens().first()==marker->output ? app.screens().last() : app.screens().first();
            pressPoint=marker->local;
            QMouseEvent p(QEvent::MouseButtonPress,QPointF(pressPoint),QPointF(pressPoint),Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);QApplication::sendEvent(canvas,&p);
            QPoint destination=other->geometry().topLeft()+QPoint(200,200);
            QPoint local=destination-canvas->output->geometry().topLeft();
            QMouseEvent m(QEvent::MouseMove,QPointF(local),QPointF(local),Qt::NoButton,Qt::LeftButton,Qt::NoModifier);QApplication::sendEvent(canvas,&m);
            QApplication::sendEvent(canvas,&release);
            if(marker->center!=destination || marker->output!=other){qWarning()<<"CROSS OUTPUT FAIL";return 6;}
            marker->place(before+QPoint(20,10));panel.refreshCanvases();
        }
        panel.addPoint(false);if(panel.markers.size()!=MaxMarkers){qWarning()<<"LIMIT FAIL";return 7;}
        qInfo()<<"DRAG AND CROSS OUTPUT PASS; LIMIT"<<MaxMarkers;
        QTimer::singleShot(2000,&panel,[&]{
            for(auto &m:panel.markers) {
                Canvas *c=nullptr;for(auto &candidate:panel.canvases)if(candidate->output==m->output)c=candidate.get();
                if(!c || !c->windowHandle()->isExposed() || !c->windowHandle()->mask().contains(m->local)) {qWarning()<<"VISIBLE REGION FAIL";app.exit(8);return;}
                QColor pixel=c->grab().toImage().pixelColor(m->local);
                if(pixel.red()<200 || pixel.green()>80) {qWarning()<<"RED PIXEL FAIL"<<pixel;app.exit(8);return;}
            }
            qInfo()<<"ALL 20 MARKERS VISIBLE AND RED";
            if(shortcutTest) {
                panel.keys->setKeySequence(QKeySequence("Ctrl+Shift+F9"));
                panel.setShortcut();
                auto bound=KGlobalAccel::self()->shortcut(panel.action);
                qInfo()<<"SHORTCUT"<<bound;
                if(bound.isEmpty()) {app.exit(4);return;}
                QDBusInterface component("org.kde.kglobalaccel","/component/clickw_selftest","org.kde.kglobalaccel.Component");
                auto reply=component.call("invokeShortcut","tap-four");
                if(reply.type()==QDBusMessage::ErrorMessage) {qWarning()<<reply.errorMessage();app.exit(4);}
            } else panel.trigger();
        });
        panel.completed=[&]{QTimer::singleShot(150,&panel,[&]{
            int count=0;bool exact=true;
            for(auto &t:targets){count+=mouseTest ? t->clicks : t->touches;qInfo()<<"TARGET"<<t->screen()->name()<<"touches"<<t->touches<<"clicks"<<t->clicks<<"maxContacts"<<t->maxContacts<<"positions"<<t->hits;if(t->times.size()>1)qInfo()<<"TARGET EVENT SPAN MS"<<(t->times.last()-t->times.first())/1000000.0;}
            for(auto &m:panel.markers){bool found=false;for(auto &t:targets)for(auto p:t->hits)if((p-m->center).manhattanLength()<=3)found=true;exact &= found;}
            qInfo()<<"BATCH SUBMISSION MS"<<panel.lastBatchNs/1000000.0;
            qInfo()<<"SELFTEST"<<((count==MaxMarkers && exact) ? "PASS" : "FAIL")<<"touches"<<count<<"positions"<<exact;
            app.exit((count==MaxMarkers && exact) ? 0:2);
        });};
        QTimer::singleShot(8000,&app,[&]{qInfo()<<"SELFTEST TIMEOUT"<<panel.status->text();app.exit(3);});
    } else if(!closeTest) {
        panel.show();
        if(app.arguments().contains("--preview")) QTimer::singleShot(1000,&panel,[&]{panel.grab().save("panel-preview.png");app.quit();});
    }
    return app.exec();
}
