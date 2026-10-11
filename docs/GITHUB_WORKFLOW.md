# GitHub'a güvenli yükleme

Bu akış mevcut depoya değişiklik yüklemek içindir. Yeni depo oluşturma, görünürlük değiştirme, onaysız dal birleştirme veya canlı yayın içermez. İlk depo oluşturma ayrı kullanıcı talimatıyla yapılmıştır.

1. Doğru klasörü, `gh auth status` ile hesabı, remote URL'sini ve aktif dalı doğrula. Yanlış checkout veya belirsiz hesap/remote varsa dur.
2. `git status --short`, diff ve sürümlenen dosyaları incele. Gerçek anahtarlar, özel `.env`, kullanıcı ayarları, nokta kayıtları, yedekler, bağımlılık klasörleri ve çıktıları ekleme. Yayın için gereken kaynak, ikon, başlatma betiği, CMake, launcher şablonu ve dokümantasyonu koru.
3. `HANDOFF.md`, gerektiğinde `CHANGELOG.md` ve ilgili kurulum/mimari belgelerini son duruma göre güncelle. Bilinen eksikleri ve test sınırlarını yaz.
4. `git fetch origin` ile uzak çalışmayı al. `git log --oneline --left-right HEAD...origin/<dal>` ile ayrışmayı incele. Yerel/uzak çalışmayı ezme. Geçmişi yeniden yazma, force push veya `reset --hard` ile kayıp yaratma. Onaysız merge/rebase yapma. Güvenle çözülemeyen çakışmada durup kullanıcıya bildir.
5. İlgili build ve testleri çalıştır. KDE Wayland GUI koşulları yoksa atlanan kontrolleri açıkça bildir.
6. İncelenmiş dosyaları seçerek stage et; `git diff --cached --check`, staged diff ve dosya listesiyle son kontrolü yap. Açıklayıcı commit oluştur.
7. Doğrulanmış dalı normal push ile yükle. Push non-fast-forward nedeniyle reddedilirse tekrar fetch/inceleme yap; force push kullanma.
8. `git fetch origin` sonrasında yerel HEAD, origin takip dalı ve `git ls-remote origin refs/heads/<dal>` SHA'larının eşitliğini doğrula. Çalışma ağacının durumunu bildir.

GitHub girişi gerekirse normal CLI/browser girişini kullan; sohbette token veya parola isteme. Commit kimliğinde mümkünse hesabın GitHub noreply adresini kullan. Rapor depo URL'si, dal, commit, kontrol sonuçları ve eşleşme doğrulamasını içermelidir.
