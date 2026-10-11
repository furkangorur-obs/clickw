# Doğrulama

## Gereksinimler

GUI testleri aktif KDE Wayland oturumu, kullanıcı D-Bus erişimi ve `/dev/uinput` yazma izni ister. Normal ClickW örneğini testten önce X/Kapat ile kapat; tek örnek kilidi vardır. Normal kullanıcının ayarları test sırasında izole QSettings yoluyla korunur.

## Komutlar

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
./Başlat.sh --self-test
./Başlat.sh --self-test --mouse-test --shortcut-test --multi-output-test
./Başlat.sh --close-test
```

| Kontrol | Gerçekte doğruladığı |
| --- | --- |
| Derleme | Kaynak ve CMake bağımlılıklarının mevcut ortamda derlenmesi |
| `--self-test` | 20 hedef, koordinatlar, kırmızı pikseller, görünür giriş bölgeleri, limit, simüle sürükleme ve mevcut ekranlar arasında geçiş |
| `--mouse-test` | Kontrollü Qt hedefinin dokunmayı mouse release olayına çevirmesi |
| `--shortcut-test` | Ayrı test bileşenine Ctrl+Shift+F9 kaydı ve KDE D-Bus üzerinden kısayol eylemi çağrısı |
| `--multi-output-test` | Hedefleri bağlı ekranlara dağıtarak 20 olayın ulaşması |
| `--close-test` | Panel close yolunun canvas/aygıt temizliği ve uygulamadan çıkışı |

Testler kendi pencerelerine giriş gönderir. Kısayol testi fiziksel klavyeden basış üretmez. Fiziksel mouse tıkları mouse-test sayacına dahil edilmez. `BATCH SUBMISSION MS` sadece worker'ın aygıtlara yazım süresidir; hedef tepki süresi veya ağır bellek yükünde garanti değildir.

## Manuel kabul

- Artıları tek tek oluştur; silme düğmesine her basışta son artının kaybolduğunu ve diğer konumların kaldığını kontrol et. Bu silme davranışı için otomatik assertion henüz yok.
- Bir artıyı birkaç kez taşı; iki/üç monitör, ekran boşlukları ve farklı ölçeklerde kaybolmadığını kontrol et.
- Gerçek F8 veya kullanıcı kısayolunu hedef uygulamada zararsız düğmelerle dene. Desteklenmeyen uygulamada dokunma dönüşümünü zorunlu kabul etme.
- Ekran ekle/çıkar; yanlış monitöre tıklama oluşmadığını kontrol et. Bu hotplug akışı otomatik olarak kapsamlı test edilmedi.
- X ve Kapat ile çık; `pgrep -a -x clickw` işlem göstermemeli, KWin'de ClickW Touch aygıtı kalmamalı.
- Normal ayarların yeniden açılışta korunduğunu kontrol et.

Başarısızlıkta komut, exit code, ilgili çıktı, sürümler ve ekran düzenini kaydet; günlüklerden özel konum/hesap verilerini paylaşmadan önce çıkar. Headless build başarısını GUI kabul testi yerine sunma.

İlk depo aktarımında ham dokunma modu başarısız, mouse-dönüşüm / iki ekran modu başarılı bulundu. Güncel bilinen regresyon `docs/VALIDATION.md` içindedir; yalnızca geçen modu tüm giriş yollarının kanıtı olarak kullanma.
