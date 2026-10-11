# Güvenlik ve kullanıcı verileri

ClickW yerel input üreten bir masaüstü uygulamasıdır; ağ servisi, backend veya API anahtarı kullanmaz. `/dev/uinput` erişimi masaüstüne olay gönderebildiği için uygulamayı yalnızca güvenilen yerel kullanıcı kapsamında çalıştır. GUI'yi root olarak çalıştırma veya cihazı herkese yazılabilir yapma.

Kısayol ve hedef konumları kullanıcının QSettings dosyasındadır. Bunlar, yerel yedekler, ekran görüntüleri, `.env` ve anahtar dosyaları Git'e yüklenmemelidir. Paylaşılan test çıktıları veya issue metinlerinde özel konum/hesap bilgilerini çıkar.

İzin, input yönlendirme veya veri sızıntısı sorunu bulunursa özel depoda proje sahibine bildir; gerçek anahtarları veya kullanıcı verilerini rapora ekleme. GitHub izinleri/vault verisiyle otomatik ek yetki edinme. Proje için ayrı bir kamuya açık güvenlik iletişim adresi henüz tanımlanmadı.
