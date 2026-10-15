# Devam notları

## Mevcut durum

ClickW, CachyOS/KDE Wayland için çalışan C++17 masaüstü uygulamasıdır. Kaynak `main.cpp`, build CMake üzerinden yürür. İlk GitHub aktarımı kullanıcı tarafından özel `clickw` deposu olarak yetkilendirilmiştir. 2026-10-16 tarihinde kullanıcının açık isteğiyle herkese açılmıştır. Kaynaklar ve tek commit’lik başlangıç geçmişi anahtar/token, özel ortam dosyası, kullanıcı verisi ve yedek açısından incelenmiş; yapılan kontrolde bu içerikler bulunmamıştır. Sonraki yüklemelerde `docs/GITHUB_WORKFLOW.md` geçerlidir.

## Tamamlanan işler

- En fazla 20 kırmızı, 24×24 piksel artı; tek tek oluşturma, kilitleme ve gizleme.
- Ekran başına sabit saydam katman ile sürüklemede kaybolma sorununun giderilmesi ve ekranlar arasında taşıma.
- QSettings ile konum/kısayol saklama; ekran ekleme/çıkarma/geometri değişikliğini algılama.
- Nokta başına zamanlayıcıların kaldırılması; monitörlere göre olay paketleri ve GUI dışındaki kısa worker işi.
- Panelin X/Kapat ile kapatılması sonrasında katmanların uygulamayı açık tutmasının giderilmesi; kısayol/worker/aygıt temizliği.
- Silmenin sondan başlayarak her basışta bir artı kaldırması.
- Başlangıç spec, mimari, agent talimatları, kurulum, test ve GitHub yükleme belgeleri.

## Doğrulama ve sınırlar

Önceki uygulama geliştirme oturumunda 20 hedef, iki monitöre mouse dönüşümü, görünür kırmızı pikseller, simüle sürükleme/ekran geçişi ve kapanma testleri geçti. Önceki iki monitörlü paket gönderimi ölçümü yaklaşık 0,067 ms idi; bu tarihsel ölçüm bir performans garantisi değildir.

İlk depo aktarımının güncel kontrol sonuçları `docs/VALIDATION.md` içinde kaydedilir. Fiziksel klavye, üç fiziksel monitör, karma DPI, geniş hotplug matrisi ve yoğun RAM/swap baskısında hedef uygulamaların tepki süresi ayrıca doğrulanmalıdır. Silme davranışı derlendi ve kaynak üzerinden incelendi; otomatik silme assertion'ı henüz yok.

## Bilinen eksikler

- İlk depo aktarımında ham `--self-test` 2/20 ve tekrarda 0/20 olay sayarak başarısız oldu. Aynı oturumdaki iki ekranlı mouse-dönüşüm testi 20/20 geçti. Kaybın hangi katmanda olduğu ayrıştırılmadı; sonraki hata incelemesinin önceliğidir. Ayrıntı: `docs/VALIDATION.md`.
- Dokunmayı sol mouse tıklamasına çevirmeyen hedef uygulamalar desteklenmeyebilir.
- Kesin eşzamanlılık/0,1 ms tepki garantisi yoktur; compositor, OS scheduler ve hedef uygulama etkiler.
- Kaynak/test kodu tek dosyada; modül ayrımı, ayrı unit test hedefleri ve headless CI henüz yok.
- Sistem bağımlılıkları kilitlenmemiştir; paket/release dağıtımı, sürüm etiketi ve lisans seçimi henüz yapılmadı.
- Minimum sürüm gereksinimleri belgelenmiştir; CMake her kütüphane için alt sürümü açıkça enforce etmez.
- GUI testleri canlı KDE Wayland oturumu gerektirir; başka compositor/platform desteği taahhüt edilmez.

## Sonraki çalışmaya başlama

`AGENTS.md` ve bu dosyayı oku; klasör/hesap/remote/dal durumunu kontrol et. Sadece hazırlık istenmişse setup ve testlerden sonra dur. Kullanıcı ayarlarını, kaynak dışı yerel yedekleri ve mevcut masaüstü düzenini koru. Yeni özellik veya refactor için ayrı talimat bekle.
