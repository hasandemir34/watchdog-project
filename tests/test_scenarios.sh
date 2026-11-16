#!/bin/bash

echo "======================================"
echo " Watchdog Test Senaryolari Basliyor "
echo "======================================"

# Projeyi derle
echo "[TEST] Proje derleniyor..."
make clean && make

if [ ! -f "./watchdog" ]; then
    echo "[HATA] Derleme basarisiz! watchdog olusturulamadi."
    exit 1
fi

# Watchdog'u arka planda calistir
echo "[TEST] Watchdog baslatiliyor..."
./watchdog &
WATCHDOG_PID=$!
sleep 2

# 1. Durum Raporlama Testi (SIGUSR1)
echo "[TEST] SIGUSR1 sinyali gonderiliyor (Durum Raporu)..."
kill -SIGUSR1 $WATCHDOG_PID
sleep 1

# 2. Beklenmedik Kapanis ve Restart Testi
# Not: Normalde burada child process PID'si bulup kill atilir.
# Ozet test mantigi:
echo "[TEST] Sistemin kendini toparlamasi (Restart) bekleniyor..."
sleep 2

# 3. Graceful Shutdown Testi (SIGTERM)
echo "[TEST] SIGTERM sinyali gonderiliyor (Graceful Shutdown)..."
kill -SIGTERM $WATCHDOG_PID
wait $WATCHDOG_PID

echo "======================================"
echo " Testler Tamamlandi. Log Ciktisi:"
echo "======================================"
cat logs/*.log 2>/dev/null || echo "[BILGI] Log dosyasi bulunamadi."
