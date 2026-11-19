#!/usr/bin/env bash
set -u

WATCHDOG_BIN="./watchdog"
LOG_FILE="watchdog.log"

if [ ! -x "$WATCHDOG_BIN" ]; then
    echo "[FAIL] watchdog binary not found. Run 'make' first."
    exit 1
fi

cleanup() {
    if [ -n "${WD_PID:-}" ] && kill -0 "$WD_PID" 2>/dev/null; then
        kill -SIGKILL "$WD_PID" 2>/dev/null || true
    fi
}
trap cleanup EXIT

rm -f "$LOG_FILE"

# 1. Normal calisma
echo "[INFO] Scenario 1: Starting watchdog..."
"$WATCHDOG_BIN" &
WD_PID=$!
sleep 2

CHILDREN=$(pgrep -P "$WD_PID" || true)
CHILD_COUNT=$(echo "$CHILDREN" | wc -w)

if [ "$CHILD_COUNT" -ge 2 ]; then
    echo "[PASS] Watchdog (PID $WD_PID) started $CHILD_COUNT child processes."
else
    echo "[FAIL] Expected at least 2 child processes, found $CHILD_COUNT."
fi

# 2. SIGUSR1 durum raporu
echo "[INFO] Scenario 2: Sending SIGUSR1..."
kill -SIGUSR1 "$WD_PID"
sleep 1

if [ -f "$LOG_FILE" ] && grep -q "SIGUSR1" "$LOG_FILE"; then
    echo "[PASS] SIGUSR1 status report written to $LOG_FILE."
else
    echo "[FAIL] SIGUSR1 status report not found in $LOG_FILE."
fi

# 3 & 4. Beklenmeyen sonlanma ve restart limiti (max 3)
echo "[INFO] Scenario 3 & 4: Testing unexpected termination and restart limit..."
for attempt in 1 2 3 4; do
    TARGET_CHILD=$(pgrep -P "$WD_PID" | head -n 1 || true)
    if [ -z "$TARGET_CHILD" ]; then
        echo "[INFO] No active worker left at attempt $attempt."
        break
    fi
    echo "[INFO] Attempt $attempt: Sending SIGKILL to worker PID $TARGET_CHILD"
    kill -SIGKILL "$TARGET_CHILD"
    sleep 2
done

if grep -qi "restart" "$LOG_FILE"; then
    echo "[PASS] Restart events recorded in $LOG_FILE."
else
    echo "[FAIL] No restart events found in $LOG_FILE."
fi

# 5. SIGTERM ile graceful shutdown ve orphan/zombie kontrolu
echo "[INFO] Scenario 5: Sending SIGTERM..."
kill -SIGTERM "$WD_PID"
sleep 2

if ! kill -0 "$WD_PID" 2>/dev/null; then
    REMAINING=$(pgrep -P "$WD_PID" || true)
    if [ -z "$REMAINING" ]; then
        echo "[PASS] Watchdog and all children terminated cleanly on SIGTERM."
    else
        echo "[FAIL] Remaining child processes found: $REMAINING"
    fi
else
    echo "[FAIL] Watchdog did not terminate on SIGTERM."
    kill -SIGKILL "$WD_PID" 2>/dev/null || true
fi

# 6. SIGINT (Ctrl+C) ile graceful shutdown
echo "[INFO] Scenario 6: Testing SIGINT shutdown..."
"$WATCHDOG_BIN" &
WD_PID=$!
sleep 2
kill -SIGINT "$WD_PID"
sleep 2

if ! kill -0 "$WD_PID" 2>/dev/null; then
    echo "[PASS] Watchdog terminated cleanly on SIGINT."
else
    echo "[FAIL] Watchdog did not terminate on SIGINT."
    kill -SIGKILL "$WD_PID" 2>/dev/null || true
fi

# 7. Log zaman damgasi dogrulama
echo "[INFO] Scenario 7: Checking log timestamps..."
if grep -qE '^\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\]' "$LOG_FILE"; then
    echo "[PASS] Timestamps match expected format."
else
    echo "[FAIL] Timestamp format invalid or missing."
fi

echo ""
echo "[INFO] Last 15 lines of $LOG_FILE:"
tail -n 15 "$LOG_FILE" || true