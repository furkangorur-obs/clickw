# ClickW

CachyOS / KDE Wayland üzerinde **20 kırmızı artıyı** ekranlara yerleştirip tek kısayolla hedefleri etkinleştiren yerel masaüstü uygulaması. Fiziksel mouse imlecini hareket ettirmez.

## Derle ve aç

Gerekenler: C++17, CMake ≥3.20, Qt6 Widgets/DBus (≥6.5), KF6GlobalAccel, LayerShellQt 6 ve libevdev. Aktif KDE Wayland oturumu ve kullanıcı `/dev/uinput` erişimi gerekir. Mevcut paketler yeterliyse yeniden kurulum yapma; ayrıntılar [kurulum belgesinde](docs/SETUP.md).

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
./Başlat.sh
```

CMake, gerçek checkout yollarıyla `build/ClickW.desktop` başlatıcısını üretir. Kaynak kökünde daha önce oluşturulmuş yerel `ClickW.desktop` dosyası korunur ve Git'e eklenmez. Uygulama menüsüne ekleme isteğe bağlıdır; KDE panel pinleri değiştirilmez.

## Kullanım

1. **+ oluştur** ile en fazla 20 artıyı tek tek ekle. Her artı aktif monitörün sol üst bölgesinde oluşur.
2. Artıları hedeflerin üzerine sürükle; monitörler arasında taşıyabilirsin. Boyutları 24×24 mantıksal pikseldir.
3. **Son artıyı sil**, her basışta yalnızca son oluşturulan bir artıyı kaldırır.
4. Varsayılan **F8** kısayolunu panelden değiştirebilirsin. İstersen noktaları kilitle, gizle veya paneli küçült.
5. Kısayol kayıtlı noktalara dokunma/bırakma olaylarını paket halinde gönderir; nokta başına bekleme yoktur.
6. **X** veya **Kapat** ile tamamen çık. Artılar, kısayol ve sanal aygıtlar kaldırılır; konumlar sonraki açılış için saklanır.

Ayarlar `~/.config/ClickW/clickw.conf` içinde tutulur ve depoya yüklenmez. Ekran ekleme/çıkarma/düzen değişikliği algılanır. Artılar sürükleme ve giriş gönderimi sırasında çizilmeye devam eder; giriş sırasında artıların küçük mouse alanları geçici saydamdır.

## Çalışma sınırları

Giriş sanal dokunmatik aygıt üzerinden yapılır. Hedef uygulamanın dokunmayı kabul etmesi veya mouse tıklamasına çevirmesi gerekir; her uygulamada gerçek sol tıkla aynı sonuç garanti edilmez. Dokunma pencere odağını değiştirebilir; compositor dokunma sırasında imleci geçici gizleyebilir. Ctrl/Alt/Shift basılı tutulursa hedef uygulama bunları da görebilir; ilk denemeyi zararsız bir düğmede yap.

Paket gönderimi kısa bir worker'da yürür. Ortak 35 ms giriş maskesi hazırlığı ve gönderim sonrası 100 ms maskeyi geri açma süresi vardır; bunlar noktalar arasına gecikme eklemez. Kernel'e yazım ölçümü hedef uygulamanın ağ/işlem/çizim süresi değildir. **Yük altında kesin 0,1 ms veya tamamen eşzamanlı tepki garantisi yoktur.**

Windows/macOS, genel X11 veya başka Wayland compositor'ları mevcut kapsamda değildir. Fiziksel üçüncü monitör, karma ölçek ve ağır RAM baskısındaki kabul durumu [devam notlarında](HANDOFF.md) belirtilir.

## Kontroller

Normal ClickW örneğini kapattıktan sonra KDE Wayland oturumunda:

```bash
./Başlat.sh --self-test
./Başlat.sh --self-test --mouse-test --shortcut-test --multi-output-test
./Başlat.sh --close-test
```

Kontroller kendi test pencerelerini ve izole ayarlarını kullanır. Kısayol testi fiziksel klavye yerine KDE eylemini çağırır. Kapsam ve manuel kabul adımları [TESTING.md](docs/TESTING.md), güncel başlangıç sonuçları [VALIDATION.md](docs/VALIDATION.md) içindedir.

## Proje belgeleri

- [AGENTS.md](AGENTS.md): ajan çalışma kuralları ve kapsam.
- [PROJECT_SPEC.md](PROJECT_SPEC.md): gereksinimler ve kabul ölçütleri.
- [ARCHITECTURE.md](ARCHITECTURE.md): bileşenler, giriş akışı ve yaşam döngüsü.
- [HANDOFF.md](HANDOFF.md): tamamlanan işler ve bilinen eksikler.
- [SETUP.md](docs/SETUP.md): mevcut bilgisayarı geliştirmeye hazırlama.
- [GITHUB_WORKFLOW.md](docs/GITHUB_WORKFLOW.md): mevcut depoya güvenli yükleme.
- [CONTRIBUTING.md](CONTRIBUTING.md), [SECURITY.md](SECURITY.md), [CHANGELOG.md](CHANGELOG.md).

Depo özel olarak başlatılmıştır. Bir yayın/release paketi veya açık kaynak lisansı henüz oluşturulmamıştır.
