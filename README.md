# Watchdog — POSIX Process Monitoring System

Watchdog, Linux/POSIX ortamında çalışan ve child worker süreçlerini izleyen bir C uygulamasıdır. Uygulama iki worker başlatır, periyodik olarak durumlarını kontrol eder, beklenmedik sonlanmaları yeniden başlatır ve sinyal tabanlı durum raporu ile güvenli kapanma sağlar.

**Sakarya University | Software Engineering | System Programming | 2026–2027**

## Proje amacı ve mimari

Ana program (`src/main.c`) iki worker sürecini başlatır ve kontrol döngüsünü yürütür. Her worker `fork()` sonrasında `exec()` ile `workers/worker` executable’ına dönüştürülür. Parent process child PID’lerini `ChildProcess` yapılarında tutar.

Mimari akış:

```text
main.c
  ├─ signal_manager.c  → SIGINT, SIGTERM, SIGUSR1, SIGCHLD, SIGALRM flag’leri
  ├─ process_manager.c → fork/exec, waitpid, restart ve shutdown/reap işlemleri
  └─ logger.c          → timestamp’li watchdog.log kayıtları
       └─ workers/worker → izlenen worker executable’ı
```

| Modül | Sorumluluk |
|---|---|
| `src/main.c` | Ana döngü, worker kurulumu, signal flag’lerinin işlenmesi ve logger entegrasyonu |
| `src/signal_manager.c` | Signal handler’ları yalnızca güvenli flag güncellemeleri yapar |
| `src/process_manager.c` | Child oluşturma, `waitpid()`, beklenmedik sonlanma, restart ve shutdown |
| `src/logger.c` | Timestamp’li INFO/WARN/ERROR/STATUS kayıtları ve SIGUSR1 raporu |
| `workers/worker.c` | Worker process executable’ı ve normal sonlanma test modu |
| `tests/test_scenarios.sh` | Build sonrası uçtan uca otomatik test senaryoları |

## Gereksinimler

- Linux veya POSIX uyumlu bir ortam
- GCC veya uyumlu C derleyicisi
- GNU Make
- Bash ve standart process araçları (`pgrep`, `ps`, `kill`, `awk`)

## Derleme ve çalıştırma

Repository kök dizininden:

```bash
make clean
make
./watchdog
```

`make`, `watchdog` binary’sini ve `workers/worker` executable’ını üretir. Watchdog başlarken iki worker oluşturur.

Örnek terminal çıktısı:

```text
Watchdog baslatildi! PID: 12695
Worker 1 baslatildi! PID: 12696
Worker 2 baslatildi! PID: 12697
```

Programı çalıştıran terminalde `Ctrl+C` SIGINT gönderir. Ayrı bir terminalden çalışan Watchdog’un PID’si bulunarak SIGTERM veya SIGUSR1 gönderilebilir:

```bash
WATCHDOG_PID=$(pgrep -n -x watchdog)
kill -SIGUSR1 "$WATCHDOG_PID"
kill -SIGTERM "$WATCHDOG_PID"
```

## Worker yaşam döngüsü

### Başlatma ve izleme

Parent process her worker için `fork()` çağırır. Child process `exec()` ile `./workers/worker` programını çalıştırır. Process manager exec hatalarını pipe üzerinden parent’a bildirir.

`SIGCHLD` ve periyodik `SIGALRM` aynı kontrol flag’ini set eder. Ana döngü bu flag’i işlediğinde `waitpid(..., WNOHANG)` ile child durumunu kontrol eder; `EINTR` durumunda ilgili sistem çağrıları yeniden denenir.

Periyodik kontrol `setitimer()` ile yaklaşık her saniye tetiklenir. Signal handler’ları process manager veya logger çağırmaz; yalnızca `sig_atomic_t` flag’lerini günceller.

### Normal ve beklenmedik sonlanma

- Exit status `0` normal sonlanma kabul edilir ve worker yeniden başlatılmaz.
- Nonzero exit status veya signal ile sonlanma beklenmedik kabul edilir.
- Beklenmedik sonlanma sonrasında aynı worker için en fazla **3 otomatik restart** yapılır.
- Dördüncü beklenmedik sonlanmada restart limiti uygulanır ve worker yeniden başlatılmaz.

Worker’ın normal sonlanma test modu:

```bash
./workers/worker --exit-after 1
```

Bu seçenek worker’ın yaklaşık bir saniye bekleyip exit status `0` ile çıkmasını sağlar. Argüman verilmezse worker’ın varsayılan davranışı değişmez ve süreç çalışmaya devam eder. Watchdog’un normal sonlanma ve restart davranışı otomatik test harness’i tarafından process manager API’si üzerinden doğrulanır; ana program varsayılan olarak sürekli çalışan worker’lar başlatır.

## Sinyaller ve kapanma

- **SIGINT:** Kullanıcı tarafından istenen kapanmayı işaretler.
- **SIGTERM:** Watchdog’un güvenli kapanmasını işaretler.
- **SIGUSR1:** Worker PID, state ve restart count bilgilerini `watchdog.log` içine STATUS kayıtları olarak yazar.
- **SIGCHLD:** Child sonlanması sonrasında kontrol flag’ini set eder.
- **SIGALRM:** `setitimer()` tarafından periyodik kontrol için üretilir.

SIGINT veya SIGTERM istendiğinde ana döngü yeni child kontrolü/restart işlemlerini durdurur ve aktif child’ları kapatmaya geçer. Her child için process manager:

1. `SIGTERM` gönderir.
2. En fazla 200 ms boyunca 10 ms aralıklarla `waitpid(..., WNOHANG)` ile bekler.
3. Child hâlâ çalışıyorsa `SIGKILL` gönderir.
4. Blocking `waitpid()` ile child’ı reap eder ve zombie bırakmadan temizler.

`ESRCH` (process artık yok) ve `ECHILD` (child zaten reaped edilmiş) durumları shutdown akışında beklenen sonlanma durumları olarak ele alınır. Bir child kapanmaya dirençli olsa bile diğer child’lar için cleanup devam eder.

## Logging

Log dosyası çalışma dizininde `watchdog.log` olarak oluşturulur ve append modunda yazılır. Her kayıt şu formattadır:

```text
[YYYY-MM-DD HH:MM:SS] [LEVEL] mesaj
```

Kaydedilen olaylar arasında şunlar bulunur:

- Watchdog startup
- Worker creation
- Normal child termination
- Beklenmedik child termination
- Restart attempt ve yeni PID
- Restart limitinin dolması
- SIGUSR1 status report başlangıcı, child satırları ve bitişi
- Graceful shutdown

Gerçek uygulama mesajlarına örnek:

```text
[2026-11-24 16:10:21] [INFO] Watchdog baslatildi PID=12695
[2026-11-24 16:10:21] [INFO] Worker 1 baslatildi PID=12696
[2026-11-24 16:10:24] [WARN] Worker 2 beklenmedik sonlandi PID=12697
[2026-11-24 16:10:24] [WARN] Worker 2 restart 1/3 Yeni PID=12701
[2026-11-24 16:10:32] [WARN] Worker 2 sonlandi PID=12703. Restart limiti doldu.
[2026-11-24 16:10:35] [STATUS] === PROCESS STATUS REPORT (SIGUSR1) ===
[2026-11-24 16:10:40] [INFO] Watchdog guvenli sekilde kapatildi
```

## Otomatik testler

Tam test akışını çalıştırmak için:

```bash
make test
```

Alternatif olarak binary’ler zaten derlenmişse:

```bash
bash -n tests/test_scenarios.sh
bash tests/test_scenarios.sh
```

Test script’i şu davranışları doğrular:

- İki worker oluşturulması
- Worker’ın `--exit-after` ile exit status `0` vererek normal sonlanması
- Normal sonlanmada restart yapılmaması
- SIGUSR1 status report kayıtları
- Beklenmedik termination sonrası restart
- Aynı worker için tam 3 restart ve limit sonrası durma
- SIGTERM ve SIGINT ile graceful shutdown
- Yanıt vermeyen worker için SIGKILL fallback
- Child’ların reaped edilmesi ve zombie bırakılmaması
- Log event mesajları ile timestamp/level formatı

Son doğrulamada test script’i **17 başarılı, 0 başarısız** assertion üretmiştir. Testler dinamik PID’ler kullanır; sabit PID veya makineye özel mutlak yol kullanmaz.

## Temizlik

Üretilen binary, object dosyaları ve log dosyasını kaldırmak için:

```bash
make clean
```

## Teslim notu

Repository içinde ayrıca assignment report veya submission template dosyası bulunmamaktadır. Harici bir ders raporu isteniyorsa önerilen kısa yapı şudur:

1. Amaç ve gereksinimler
2. Mimari ve kullanılan POSIX çağrıları
3. Signal ve process lifecycle tasarımı
4. Restart ve graceful shutdown davranışı
5. Logging ve test sonuçları
6. Sınırlamalar ve ekip katkıları

**Technologies:** C · Linux · POSIX · GCC · GNU Make · Bash · Git · GitHub
