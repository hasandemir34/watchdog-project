#!/usr/bin/env bash
set -u

WATCHDOG_BIN="./watchdog"
WORKER_BIN="./workers/worker"
LOG_FILE="watchdog.log"
PASS_COUNT=0
FAIL_COUNT=0
TEST_TMP_DIR=""

pass() { echo "[PASS] $1"; PASS_COUNT=$((PASS_COUNT + 1)); }
fail() { echo "[FAIL] $1"; FAIL_COUNT=$((FAIL_COUNT + 1)); }
info() { echo "[INFO] $1"; }

if [ ! -x "$WATCHDOG_BIN" ]; then
    echo "[FAIL] watchdog binary not found. Run 'make' first."
    exit 1
fi

cleanup() {
    if [ -n "${WD_PID:-}" ] && kill -0 "$WD_PID" 2>/dev/null; then
        pkill -SIGKILL -P "$WD_PID" 2>/dev/null || true
        kill -SIGKILL "$WD_PID" 2>/dev/null || true
    fi
    if [ -n "$TEST_TMP_DIR" ]; then
        rm -rf "$TEST_TMP_DIR"
    fi
}
trap cleanup EXIT

rm -f "$LOG_FILE"

# 1. Normal worker termination must be reported without a restart.
info "Test 1: Verifying normal worker termination and no restart..."
TEST_TMP_DIR=$(mktemp -d)
if ${CC:-gcc} -std=c11 -Wall -Wextra -Wpedantic -Werror \
    -D_POSIX_C_SOURCE=200809L -Iinclude -x c - src/process_manager.c \
    -o "$TEST_TMP_DIR/normal_exit_test" <<'EOF'
#include <stdio.h>
#include <time.h>
#include "process_manager.h"

int main(int argc, char *argv[])
{
    const char *arguments[4];
    ChildProcess child = {0};
    int result;

    if (argc != 2)
        return 1;

    arguments[0] = argv[1];
    arguments[1] = "--exit-after";
    arguments[2] = "1";
    arguments[3] = NULL;

    if (childProcessBaslatAvecArguments(&child, argv[1], arguments) != 0)
        return 1;

    do
    {
        struct timespec delay = {0, 100000000L};
        nanosleep(&delay, NULL);
        result = childProcessIzleVeYenidenBaslat(&child);
    } while (result == CHILD_PROCESS_RUNNING);

    if (result != CHILD_PROCESS_EXITED || child.restartCount != 0 ||
        child.aktif != 0 || child.pid != -1)
    {
        childProcessSonlandir(&child);
        return 2;
    }

    return 0;
}
EOF
then
    if "$TEST_TMP_DIR/normal_exit_test" "$WORKER_BIN"; then
        pass "Worker exited with status 0 and was not restarted."
    else
        fail "Normal worker termination was not handled as expected."
    fi
else
    fail "Could not compile the normal termination test harness."
fi

# 2. Normal çalışma ve en az 2 worker kontrolü
info "Test 2: Starting watchdog and verifying child processes..."
"$WATCHDOG_BIN" &
WD_PID=$!
sleep 2

CHILDREN=$(pgrep -P "$WD_PID" || true)
CHILD_COUNT=$(echo "$CHILDREN" | wc -w)

if [ "$CHILD_COUNT" -ge 2 ]; then
    pass "Watchdog (PID $WD_PID) spawned $CHILD_COUNT workers: $CHILDREN"
else
    fail "Expected at least 2 workers, found $CHILD_COUNT."
fi

# 3. SIGUSR1: Her iki worker PID'sinin logda doğrulanması
info "Test 3: Sending SIGUSR1 and verifying both worker PIDs in $LOG_FILE..."
kill -SIGUSR1 "$WD_PID"
sleep 1

if [ ! -f "$LOG_FILE" ]; then
    fail "Log file $LOG_FILE was not created after SIGUSR1."
else
    ALL_PIDS_FOUND=1
    for cpid in $CHILDREN; do
        if ! grep -q "$cpid" "$LOG_FILE"; then
            fail "Worker PID $cpid not found in SIGUSR1 log."
            ALL_PIDS_FOUND=0
        fi
    done

    if [ "$ALL_PIDS_FOUND" -eq 1 ] && [ "$CHILD_COUNT" -ge 2 ]; then
        pass "All worker PIDs ($CHILDREN) verified in SIGUSR1 log."
    fi
fi

# 4. Aynı worker'ı takip ederek tam 3 restart + 4. kapanışta limit kontrolü
info "Test 4: Tracking a single worker for exact 3 restarts and limit enforcement..."
INITIAL_COUNT=$(pgrep -P "$WD_PID" | wc -w || true)
UNTOUCHED_PID=$(pgrep -P "$WD_PID" | head -n 1 || true)
ACTUAL_RESTARTS=0

for attempt in 1 2 3 4; do
    TARGET_CHILD=$(pgrep -P "$WD_PID" | grep -v "^${UNTOUCHED_PID}$" | head -n 1 || true)
    if [ -z "$TARGET_CHILD" ]; then
        info "Target worker is no longer running at attempt $attempt."
        break
    fi

    info "Attempt $attempt: Killing target worker PID $TARGET_CHILD with SIGKILL"
    kill -SIGKILL "$TARGET_CHILD"
    sleep 2

    NEW_CHILD=$(pgrep -P "$WD_PID" | grep -v "^${UNTOUCHED_PID}$" | head -n 1 || true)
    if [ -n "$NEW_CHILD" ] && [ "$NEW_CHILD" != "$TARGET_CHILD" ]; then
        ACTUAL_RESTARTS=$((ACTUAL_RESTARTS + 1))
        info "Target worker restarted with new PID $NEW_CHILD (restart #$ACTUAL_RESTARTS)"
    else
        info "Target worker did not restart after attempt $attempt."
    fi
done

CURRENT_COUNT=$(pgrep -P "$WD_PID" | wc -w || true)
EXPECTED_COUNT=$((INITIAL_COUNT - 1))

if [ "$ACTUAL_RESTARTS" -eq 3 ] && [ "$CURRENT_COUNT" -eq "$EXPECTED_COUNT" ]; then
    pass "Same worker restarted exactly 3 times and stopped after 4th termination (active workers: $CURRENT_COUNT)."
else
    fail "Restart limit check failed (actual restarts: $ACTUAL_RESTARTS, active workers: $CURRENT_COUNT, expected: $EXPECTED_COUNT)."
fi

# 5. SIGTERM shutdown ve child temizliği
info "Test 5: Sending SIGTERM and confirming child termination..."
BEFORE_TERM=$(pgrep -P "$WD_PID" || true)
kill -SIGTERM "$WD_PID"
sleep 2

if ! kill -0 "$WD_PID" 2>/dev/null; then
    STILL_ALIVE=0
    for cpid in $BEFORE_TERM; do
        if kill -0 "$cpid" 2>/dev/null; then
            STILL_ALIVE=1
        fi
    done
    if [ "$STILL_ALIVE" -eq 0 ]; then
        pass "Watchdog and all children terminated cleanly after SIGTERM."
    else
        fail "Child processes remained alive after SIGTERM."
    fi
else
    fail "Watchdog did not terminate on SIGTERM."
    kill -SIGKILL "$WD_PID" 2>/dev/null || true
fi

# 6. Otomatik SIGINT shutdown ve child temizliği
info "Test 6: Testing automated SIGINT shutdown..."
"$WATCHDOG_BIN" &
WD_PID=$!
sleep 2

BEFORE_INT=$(pgrep -P "$WD_PID" || true)
kill -SIGINT "$WD_PID"
sleep 2

if ! kill -0 "$WD_PID" 2>/dev/null; then
    STILL_ALIVE=0
    for cpid in $BEFORE_INT; do
        if kill -0 "$cpid" 2>/dev/null; then
            STILL_ALIVE=1
        fi
    done
    if [ "$STILL_ALIVE" -eq 0 ]; then
        pass "Watchdog and all children terminated cleanly after SIGINT."
    else
        fail "Child processes remained alive after SIGINT."
    fi
else
    fail "Watchdog did not terminate on SIGINT."
    kill -SIGKILL "$WD_PID" 2>/dev/null || true
fi

# 7. SIGTERM timeout ve SIGKILL fallback testi
info "Test 7: Testing SIGTERM timeout and SIGKILL fallback..."
"$WATCHDOG_BIN" &
WD_PID=$!
sleep 2

FROZEN_CHILD=$(pgrep -P "$WD_PID" | head -n 1 || true)
if [ -n "$FROZEN_CHILD" ]; then
    kill -SIGSTOP "$FROZEN_CHILD"
    kill -SIGTERM "$WD_PID"

    for _ in 1 2 3 4 5 6 7 8 9 10; do
        if ! kill -0 "$WD_PID" 2>/dev/null; then
            break
        fi
        sleep 1
    done

    if ! kill -0 "$WD_PID" 2>/dev/null && ! kill -0 "$FROZEN_CHILD" 2>/dev/null; then
        pass "Unresponsive worker ($FROZEN_CHILD) terminated via SIGKILL fallback."
    else
        fail "SIGTERM timeout / SIGKILL fallback failed for worker $FROZEN_CHILD."
        kill -SIGKILL "$FROZEN_CHILD" 2>/dev/null || true
        kill -SIGKILL "$WD_PID" 2>/dev/null || true
    fi
else
    fail "No worker process found for SIGKILL fallback test."
fi

# 8. Log zaman damgası kontrolü
info "Test 8: Verifying timestamp format in $LOG_FILE..."
if [ -f "$LOG_FILE" ] && grep -qE '^\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\]' "$LOG_FILE"; then
    pass "Log timestamps verified."
else
    fail "Log timestamps missing or invalid."
fi

echo ""
echo "========================================"
echo "Test Summary: $PASS_COUNT passed, $FAIL_COUNT failed"
echo "========================================"

exit "$FAIL_COUNT"
