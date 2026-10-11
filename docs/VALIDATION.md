# Başlangıç doğrulaması — 2026-10-11

Bu rapor ilk özel depo aktarımındaki yerel durumu kaydeder. Önceki geliştirme oturumunun sonuçlarıyla karıştırılmamalıdır.

## Ortam

CachyOS, KDE Wayland; iki aktif ekran; kullanıcı `/dev/uinput` erişimi ve `tr_TR.UTF-8` locale mevcut. Bağımlılıklar zaten kurulu olduğundan ek sistem paketi kurulmadı. Ortam dosyası/API anahtarı gerekmiyor; mevcut kullanıcı kayıtları değiştirilmedi.

| Paket | Yerel sürüm |
| --- | --- |
| Qt6 Base | 6.11.2-3.1 |
| KF6 GlobalAccel | 6.30.0-1.1 |
| LayerShellQt | 6.7.5-1.1 |
| libevdev | 1.13.7-1.1 |
| GCC | 16.2.1+r23+gd564253eb6c8-1 |
| CMake | 4.4.3-2.1 |
| pkgconf | 3.0.7-1.1 |

## Sonuçlar

| Kontrol | Sonuç |
| --- | --- |
| CMake Release yapılandırması | Başarılı |
| Release derleme | Başarılı |
| `--self-test` (ham dokunma modu) | Başarısız: ilk çalışmada 2/20, tekrarda 0/20; exit 2 |
| `--self-test --mouse-test --shortcut-test --multi-output-test` | Başarılı: iki ekranda 10+10 mouse release, toplam 20/20 doğru hedef; exit 0 |
| Kırmızı görünürlük / 20 nokta sınırı / simüle sürükleme-geçiş | İki modda da başarılı |
| `--close-test` | Başarılı: exit 0 |
| Üretilmiş launcher, `desktop-file-validate` | Başarılı |

Çoklu ekran mouse-dönüşüm testinde paket yazımı yaklaşık **0,040425 ms** ölçüldü. Bu tek ölçüm performans garantisi değildir. İsteğe bağlı Vulkan başlığı keşif uyarısı derlemeyi engellemedi.

## Açık hata ve takip

Ham dokunma testinin eksik/hiç olay sayması bu başlangıç aktarımında tekrarlanmıştır. Mouse dönüşüm modu aynı oturumda 20/20 geçti. Kaybın gönderim, compositor yönlendirmesi veya Qt ham dokunma test hedefinde oluştuğu henüz ayrıştırılmadı. Bu yüzden tüm GUI kontrolleri geçti denemez ve uygulamanın her hedefte kayıpsız çalıştığı taahhüt edilemez.

Kullanıcı bu aşamada özellik/davranış değişikliği istemediğinden uygulama kaynağı değiştirilmedi; hata açık takip olarak kaydedildi. Sonraki geliştirme isteğinde önce kontrollü ham dokunma regresyonunu ayrıştır.

Fiziksel üçüncü ekran, farklı ölçekler, gerçek klavyeden kısayol, ağır RAM baskısı ve tekli silme için otomatik assertion bu aktarımda ayrıca test edilmedi. Çalışma klasöründe GUI testleri sona erdikten sonra normal uygulama otomatik başlatılmadı.
