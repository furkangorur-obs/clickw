# ClickW çalışma kuralları

Bu dosya depo kökündeki bütün dosyalar için geçerlidir. Kullanıcının doğrudan talimatları önceliklidir. Kullanıcı kapsam belirtmişse o kapsamda çalış.

## Önce okunacak belgeler

`README.md`, `PROJECT_SPEC.md`, `ARCHITECTURE.md`, `HANDOFF.md`, `docs/SETUP.md`, `docs/TESTING.md` ve ilgili iş için `docs/GITHUB_WORKFLOW.md` dosyalarını oku. Tamamlanmış işleri ve bilinen eksikleri kontrol et; önceki bir ölçümü yeni doğrulama gibi sunma.

## Çalışma klasörü ve güvenlik

- Önce klasörü, Git durumunu, aktif GitHub hesabını, remote ve dalı kontrol et.
- Klasör boşsa yalnızca kullanıcının belirttiği depoyu klonla. Dolu klasörü silme, ezme veya otomatik başka bir projeye dönüştürme. Doğru depo zaten varsa yeniden klonlama.
- Normal GitHub giriş akışını kullan. Sohbette parola, token veya özel anahtar isteme; bunları çıktılara yazma.
- Mevcut ortam dosyalarını ve kullanıcı ayarlarını koru. Boş örnek dosyalarıyla üzerlerine yazma.
- Yedekler, kayıtlı hedef konumları, günlükler, derleme çıktıları ve kullanıcı verileri Git'e girmemeli. `.gitignore` kontrolüne ek olarak staged dosyaları incele.
- Sistem izinlerini, `/dev/uinput` erişimini, KDE panel düzenini veya başka projeleri otomatik değiştirme. Uygulamayı root olarak çalıştırma.

## Geliştirmeye hazırlama isteği

Bu istek özellik geliştirme veya yayın yetkisi değildir. Gerçek teknolojiye ve mevcut bağımlılık bildirimlerine göre hazırlık yap: C++17, CMake, Qt6, KDE Frameworks 6 GlobalAccel, LayerShellQt, libevdev. Node/Python paket yöneticisi veya olmayan bir kilit dosyası ekleme. CMake bağımlılıkları buluyorsa yeniden paket kurma. Eksik anahtar gerektirmeyen hazırlığı tamamla; bu uygulama şu an dış servis anahtarı gerektirmez.

Uygun derleme/test kontrollerinden sonra klasörü, dalı, commit'i, kontrol sonuçlarını ve başlatma komutunu bildir; yeni geliştirme isteğini bekle. Canlıya yayın, yeni özellik veya başka proje üzerinde işlem yapma.

## Uygulamaya özel değişmezler

- En fazla 20 kırmızı, 24×24 mantıksal piksel boyutunda artı oluşturulur.
- Silme sondan başlar; her basış yalnızca bir artıyı kaldırır.
- Artılar sürüklemede ve monitör geçişinde görünür kalır. Sabit ekran katmanlarını sürükleme sırasında yeniden oluşturma.
- Fiziksel imleci hareket ettirme veya fiziksel mouse olaylarını engelleme.
- Giriş paketlerine nokta başına zamanlayıcı/bekleme ekleme. Bağımsız dokunma/bırakma çiftleri kullan; çoklu parmak jesti dört/20 bağımsız tık yerine geçmez.
- Mutlak 0,1 ms veya yüksek yükte kesin eşzamanlılık garantisi verme. Kernel'e gönderim ölçümü hedef uygulamanın tepki süresi değildir.
- X ve Kapat, katmanları, iş parçacığını, kısayolu ve sanal aygıtları temizlemeli; uygulama arka planda kalmamalı.
- Monitör düzeni yenilenirken çalışan iş parçacığının kullandığı aygıtları erken yok etme. Mevcut kayıtları koru.

## Kontroller ve devam belgeleri

Kaynak değişikliklerinde `docs/TESTING.md` kapsamındaki ilgili kontrolleri çalıştır. GUI testleri KDE Wayland oturumu ve kullanıcı `/dev/uinput` erişimi ister; bu koşullar yoksa atlanan testi açıkça belirt. Build başarısını gerçek browser işlevi veya üç fiziksel monitör testi olarak sunma. Sadece doküman değişiklikleri için gereksiz ağır test üretme.

`HANDOFF.md` ve gerektiğinde `CHANGELOG.md` dosyalarını güncelle. Yapılan işi, doğrulanan davranışı, bilinen eksikleri ve sonraki kontrolleri ayrı ve açık yaz.

## GitHub'a yükleme isteği

`docs/GITHUB_WORKFLOW.md` akışını uygula. Fetch yap, yerel/uzak geçmişi incele. Force push, geçmiş yeniden yazma, yerel/uzak çalışmayı ezme veya onaysız dal birleştirme yapma. Güvenle çözülemeyen çakışmada dur ve bildir. Sonraki yüklemelerde yeni depo oluşturma, görünürlük değiştirme veya canlıya yayın yapma; bu işlemler ayrı ve açık kullanıcı isteği gerektirir.

Yalnızca incelenmiş sürümlenen dosyaları açıklayıcı commit ile doğru dala yükle. Sonunda depo URL'sini, dalı, commit'i, kontrolleri ve yerel HEAD / origin takip ref'i / uzak dal SHA eşitliğini bildir.
