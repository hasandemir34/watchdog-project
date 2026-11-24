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
assert_log() {
    local description=$1
    local pattern=$2

    if grep -Eq -- "$pattern" "$LOG_FILE"; then
        pass "$description"
    else
        fail "$description"
    fi
}

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
    -D_POSIX_C_SOURCE=200809L -Iinclude -x c - src/process_manager.c src/logger.c \
    -o "$TEST_TMP_DIR/normal_exit_test" <<'EOF'
#include <stdio.h>
#include <time.h>
#include "process_manager.h"
#include "logger.h"

int main(int argc, char *argv[])
{
    const char *arguments[4];
    ChildProcess child = {0};
    pid_t old_pid;
    int result;

    if (argc != 3 || logger_init(argv[2]) == -1)
        return 1;

    arguments[0] = argv[1];
    arguments[1] = "--exit-after";
    arguments[2] = "1";
    arguments[3] = NULL;

    if (childProcessBaslatAvecArguments(&child, argv[1], arguments) != 0)
    {
        logger_close();
        return 1;
    }

    old_pid = child.pid;

    do
    {
        struct timespec delay = {0, 100000000L};
        nanosleep(&delay, NULL);
        result = childProcessIzleVeYenidenBaslat(&child);
    } while (result == CHILD_PROCESS_RUNNING);

    if (result == CHILD_PROCESS_EXITED && child.restartCount == 0 &&
        child.aktif == 0 && child.pid == -1)
    {
        logger_log(LOG_INFO, "Worker 1 normal sonlandi PID=%ld", (long)old_pid);
        logger_close();
        return 0;
    }

    if (child.aktif)
        childProcessSonlandir(&child);
    logger_close();

    return 2;
}
EOF
then
    NORMAL_LOG="$TEST_TMP_DIR/normal.log"
    if "$TEST_TMP_DIR/normal_exit_test" "$WORKER_BIN" "$NORMAL_LOG" && \
        grep -Eq '^\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\] \[INFO\] Worker 1 normal sonlandi PID=[0-9]+$' "$NORMAL_LOG"; then
        pass "Worker exited with status 0, was not restarted, and normal termination was logged."
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
assert_log "Watchdog startup was logged." 'Watchdog baslatildi PID=[0-9]+'
if [ "$(grep -Ec 'Worker [12] baslatildi PID=[0-9]+' "$LOG_FILE")" -ge 2 ]; then
    pass "Both child process creations were logged."
else
    fail "Both child process creations were not logged."
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
assert_log "SIGUSR1 status report header was logged." '=== PROCESS STATUS REPORT \(SIGUSR1\) ==='
if [ "$(grep -Ec 'Child #[12] \| Name: worker-[12] \| PID: [0-9]+ \| State: RUNNING \| Restarts: 0' "$LOG_FILE")" -ge 2 ]; then
    pass "SIGUSR1 status entries were logged."
else
    fail "SIGUSR1 status entries were not logged."
fi
assert_log "SIGUSR1 status report footer was logged." '=== END OF STATUS REPORT ==='

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
assert_log "Unexpected worker termination was logged." 'Worker [12] beklenmedik sonlandi PID=[0-9]+'
if [ "$(grep -Ec 'Worker [12] restart [1-3]/3 Yeni PID=[0-9]+' "$LOG_FILE")" -ge 3 ]; then
    pass "Worker restart events were logged."
else
    fail "Worker restart events were not logged."
fi
assert_log "Maximum restart limit was logged." 'Restart limiti doldu\.'

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
        if ps -o stat= -p "$cpid" 2>/dev/null | grep -q '^Z'; then
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
assert_log "Graceful shutdown was logged." 'Watchdog guvenli sekilde kapatildi$'

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
        if ps -o stat= -p "$cpid" 2>/dev/null | grep -q '^Z'; then
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

    if ! kill -0 "$WD_PID" 2>/dev/null && \
        ! kill -0 "$FROZEN_CHILD" 2>/dev/null && \
        ! ps -o stat= -p "$FROZEN_CHILD" 2>/dev/null | grep -q '^Z'; then
        pass "Unresponsive worker ($FROZEN_CHILD) terminated via SIGKILL fallback."
    else
        fail "SIGTERM timeout / SIGKILL fallback failed for worker $FROZEN_CHILD."
        kill -SIGKILL "$FROZEN_CHILD" 2>/dev/null || true
        kill -SIGKILL "$WD_PID" 2>/dev/null || true
    fi
else
    fail "No worker process found for SIGKILL fallback test."
fi

# 8. Log timestamp and level format control
info "Test 8: Verifying timestamp format in $LOG_FILE..."
if [ -s "$LOG_FILE" ] && awk 'NF && $0 !~ /^\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\] \[(INFO|WARN|ERROR|STATUS)\] / { exit 1 }' "$LOG_FILE"; then
    pass "All log records have the expected timestamp and level format."
else
    fail "Log timestamp or level format is missing or invalid."
fi

echo ""
echo "========================================"
echo "Test Summary: $PASS_COUNT passed, $FAIL_COUNT failed"
echo "========================================"

exit "$FAIL_COUNT"
