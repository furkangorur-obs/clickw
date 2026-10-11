# ClickW proje tanımı

## Amaç

CachyOS / KDE Wayland üzerinde kullanıcının sürükleyerek belirlediği ekran noktalarını bir klavye kısayoluyla etkinleştiren yerel masaüstü uygulaması. Fiziksel mouse imleci bulunduğu yerde kalır.

## Mevcut kapsam

| Alan | Davranış |
| --- | --- |
| Noktalar | Kullanıcı tek tek oluşturur; üst sınır 20 |
| Görünüm | Kırmızı artı, beyaz kontur, 24×24 mantıksal piksel |
| Silme | Son oluşturulan artıdan başlayarak basış başına bir adet |
| Yerleştirme | Sürükleme, ekran kenarında sınırlandırma, monitörler arasında geçiş |
| Kısayol | Varsayılan F8; panelden değiştirilebilir; basılı tutma tekrar oluşturmaz |
| Giriş | Monitör başına sanal dokunmatik aygıt; dokunma/bırakma çiftleri paketlenir |
| Görünürlük | Kilitleme/gizleme seçenekleri; giriş sırasında artılar çizilmeye devam eder |
| Monitörler | Ekleme/çıkarma/geometri değişikliğine göre katman ve aygıtlar yenilenir |
| Kayıt | QSettings ile kısayol, nokta sayısı, global/yerel konum ve monitör adı |
| Kapanma | Katmanlar, worker, kısayol ve sanal aygıtlar temizlenir |

## Kabul ölçütleri

1. 20 nokta oluşturulur; 21. oluşturma mevcut listeyi değiştirmez.
2. Silme yalnızca son noktayı kaldırır; diğer konumlar korunur.
3. Sürüklenen nokta görünür kalır ve bağlı ekranlar arasında taşınabilir.
4. Kısayol kayıtlı noktaların giriş çiftlerini kayıpsız gönderir; fiziksel imleç için hareket olayı üretilmez.
5. Nokta başına kasıtlı bekleme yoktur; paket gönderimi GUI dışındaki worker'da yapılır.
6. Normal kapatma sonrasında uygulama, katman ve sanal aygıt kalmaz.
7. Kullanıcının normal ayarları otomatik testler tarafından değiştirilmez.

## Platform ve sınırlar

Mevcut uygulama Linux/KDE Wayland ve KWin'in giriş aygıtı D-Bus arayüzüne yöneliktir. Windows, macOS, genel X11 veya diğer Wayland compositor'ları için destek taahhüdü yoktur. Dokunmanın sol mouse tıklamasına çevrilmesi hedef uygulamaya bağlıdır. Bir browser'ın ağ isteği, JavaScript çalışması veya çizimi bu uygulamanın zamanlama denetiminde değildir.

0,1 ms veya tamamen eşzamanlı hedef uygulama işlemi garanti edilmez. Girişler monitör başına gruplandığı için farklı monitörlerdeki noktaların genel oluşturma sırası korunmak zorunda değildir. Ağ/API anahtarı, sunucu, veritabanı veya canlı dağıtım bulunmaz.

## Doğrulama durumu

20 hedef, kırmızı görünürlük, simüle edilmiş sürükleme/monitör geçişi, iki monitöre giriş gönderimi ve normal kapanma testleri mevcut. Fiziksel klavye basışı, üç fiziksel ekran, farklı DPI düzenleri ve yoğun RAM baskısında uçtan uca gecikme ayrıca kullanıcı ortamında kontrol edilmelidir. Güncel sonuçlar `HANDOFF.md` dosyasında tutulur.
