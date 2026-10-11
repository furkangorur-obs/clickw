# Geliştirme ortamı hazırlama

## Kapsam

Hazırlık; doğru checkout, bağımlılık keşfi, derleme ve yetkili testlerle sınırlıdır. Özellik değişikliği, canlı yayın veya başka projelerde işlem yapma yetkisi vermez.

## Checkout kontrolü

Hedef klasörü önce listele. Boşsa kullanıcının verdiği depo URL'sini o klasöre klonla. Doluysa dosya silme/ezme; Git varsa `git status`, `git remote -v`, `git branch --show-current` ve `git log -1 --oneline` ile doğru depoyu doğrula. Git yoksa mevcut dosyaları koru ve depo bağlantısını netleştir. İlk özel depo oluşturma yalnızca açık kullanıcı isteğiyle yapılır.

`AGENTS.md`, `README.md`, `HANDOFF.md`, `PROJECT_SPEC.md`, `ARCHITECTURE.md` ve bu belgeyi oku. Ortam dosyalarını, kayıtlı nokta konumlarını ve yerel launcher'ları koru.

## Gerçek bağımlılıklar

- CMake 3.20 veya üzeri ve C++17 destekli derleyici.
- Qt6 Widgets ve DBus. Kaynak `QKeySequenceEdit::setMaximumSequenceLength` gibi API'ler kullandığı için [Qt belgelerine göre](https://doc.qt.io/qt-6/qkeysequenceedit.html#maximumSequenceLength-prop) Qt 6.5 veya üzeri gerekir.
- KDE Frameworks 6 `KF6GlobalAccel`.
- LayerShellQt 6; kullanılan `setScreen`/`setActivateOnShow` API'lerine sahip sürüm. Bu API kümesi için 6.6 veya üzerini kullan.
- libevdev geliştirme başlıkları/kütüphanesi, pkg-config.
- Çalışma ve GUI testleri için KDE Plasma/KWin Wayland, kullanıcı D-Bus oturumu, kullanıcıya açık `/dev/uinput` ve `tr_TR.UTF-8` locale.

Bu proje sistem paketlerine bağlanır; paket yöneticisi kilit dosyası, Node bağımlılığı veya Python uygulama ortamı yoktur. Sürüm sabitleme/lisanslı dağıtım paketi henüz hazırlanmadı. CMake mevcut paketleri buluyorsa yeniden kurulum gerekmez.

CachyOS/Arch'ta eksik paketleri önce belirle. Gerektiğinde işletim sisteminin paket yöneticisiyle ilgili geliştirme paketlerini kur:

```bash
paru -S --needed base-devel cmake pkgconf qt6-base kglobalaccel layer-shell-qt libevdev
```

Bu komutu ihtiyaç varsa kullan; otomatik sürüm yükseltmesi veya ilgisiz paket onarımı yapma. `/dev/uinput` erişimi yoksa sorunu bildir; root GUI, herkese açık chmod veya geniş input yetkisi verme.

## Derleme ve başlatma

Depo kökünden:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
./Başlat.sh
```

CMake eksik kütüphaneyi gösterirse önce o bağımlılığı çöz. Diğer işletim sistemleri için uyumluluk varsayma. API anahtarı gerekmiyor; `.env` oluşturma veya mevcut ortam dosyalarını değiştirme.

## İsteğe bağlı uygulama menüsü

CMake `build/ClickW.desktop` dosyasını kaynak klasörünün gerçek yollarıyla üretir. Önceden kurulu launcher varsa yedeğini al; ardından bu dosyayı kullanıcı uygulama dizinine kopyalayabilirsin. KDE panel pinlerini değiştirme. Kaynak klasörü taşınırsa CMake'i yeniden çalıştırıp launcher'ı güncelle.

## Hazırlık sonunda

`docs/TESTING.md` kontrollerini koşula uygun çalıştır. Klasörü, dalı, commit'i, başarılı/atlanmış kontrolleri ve `./Başlat.sh` komutunu bildir. Setup isteğinde burada dur ve geliştirme talimatını bekle.
