#!/usr/bin/env bash
set -u

WATCHDOG_BIN="./watchdog"
LOG_FILE="watchdog.log"

if [ ! -x "$WATCHDOG_BIN" ]; then
    echo "[FAIL] watchdog binary not found. Run 'make' first."
    exit 1
fi

rm -f "$LOG_FILE"

echo "[INFO] Starting watchdog..."
"$WATCHDOG_BIN" &
WD_PID=$!
sleep 2

# 1. Cocuk surec kontrolu
CHILDREN=$(pgrep -P "$WD_PID" || true)
CHILD_COUNT=$(echo "$CHILDREN" | wc -w)

if [ "$CHILD_COUNT" -ge 2 ]; then
    echo "[PASS] Watchdog (PID $WD_PID) started $CHILD_COUNT child processes."
else
    echo "[FAIL] Expected at least 2 child processes, found $CHILD_COUNT."
fi

# 2. SIGUSR1 durum raporu kontrolu
echo "[INFO] Sending SIGUSR1 to watchdog..."
kill -SIGUSR1 "$WD_PID"
sleep 1

if [ -f "$LOG_FILE" ] && grep -q "SIGUSR1" "$LOG_FILE"; then
    echo "[PASS] SIGUSR1 status report written to $LOG_FILE."
else
    echo "[FAIL] SIGUSR1 status report not found in $LOG_FILE."
fi

# 3. SIGTERM ile duzgun kapanis
echo "[INFO] Sending SIGTERM to watchdog..."
kill -SIGTERM "$WD_PID"
sleep 2

if ! kill -0 "$WD_PID" 2>/dev/null; then
    echo "[PASS] Watchdog terminated cleanly on SIGTERM."
else
    echo "[FAIL] Watchdog did not terminate on SIGTERM."
    kill -SIGKILL "$WD_PID" 2>/dev/null || true
fi