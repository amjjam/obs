#!/bin/bash

# Get directory where this script resides
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PID_DIR="$SCRIPT_DIR/../pids"
LOG_DIR="$SCRIPT_DIR/../logs"

mkdir -p "$PID_DIR" "$LOG_DIR"

# Set library path relative to the script
export LD_LIBRARY_PATH="$SCRIPT_DIR/../lib:$LD_LIBRARY_PATH"

# Commands relative to the script directory
CMD1="$SCRIPT_DIR/../../amjngc/NGC/ngcdcs/src/ngcdcsDtt0 -test 100 -port 8031 -sender-frames /frames:2:200000"
CMD2="$SCRIPT_DIR/../bin/DataProcessor --receiver-frames /frames:2:200000"
CMD3="$SCRIPT_DIR/../bin/FringeTracker --active 1 --receiver-phasors 127.0.0.1:27001 --sender-pspec 127.0.0.1:27006 100 --sender-movements 127.0.0.1:27004 --sender-tracker_status 127.0.0.1:27008 10 1000 --sender-tracker-controller 127.0.0.1:27009 1000 --receiver-tracker-controller 127.0.0.1:27010 --sender-tracker-snr 127.0.0.1:27011"
CMD4="$SCRIPT_DIR/../bin/DelayController --receiver-movements 127.0.0.1:27004 --sender-display 127.0.0.1:27007 100 --sender-delaylines sim 6 127.0.0.1:27003"

start() {
    echo "Starting processes..."

    $CMD1 > "$LOG_DIR/ngcdcsDtt0.out" 2> "$LOG_DIR/ngcdcsDtt0.err" &
    echo $! > "$PID_DIR/ngcdcsDtt0.pid"

    $CMD2 > "$LOG_DIR/DataProcessor.out" 2> "$LOG_DIR/DataProcessor.err" &
    echo $! > "$PID_DIR/DataProcessor.pid"

    $CMD3 > "$LOG_DIR/FringeTracker.out" 2> "$LOG_DIR/FringeTracker.err" &
    echo $! > "$PID_DIR/FringeTracker.pid"

    $CMD4 > "$LOG_DIR/DelayController.out" 2> "$LOG_DIR/DelayController.err" &
    echo $! > "$PID_DIR/DelayController.pid"

    echo "All processes started with logging."
}

stop() {
    echo "Stopping processes..."
    for name in ngcdcsDtt0 DataProcessor FringeTracker DelayController; do
        PID_FILE="$PID_DIR/$name.pid"
        if [[ -f "$PID_FILE" ]]; then
            PID=$(cat "$PID_FILE")
            if kill -0 "$PID" 2>/dev/null; then
                kill "$PID"
                echo "Stopped $name (PID $PID)"
            else
                echo "$name not running (stale PID $PID?)"
            fi
            rm -f "$PID_FILE"
        else
            echo "No PID file for $name"
        fi
    done
}

status() {
    for name in ngcdcsDtt0 DataProcessor FringeTracker DelayController; do
        PID_FILE="$PID_DIR/$name.pid"
        if [[ -f "$PID_FILE" ]]; then
            PID=$(cat "$PID_FILE")
            if kill -0 "$PID" 2>/dev/null; then
                echo "$name is running (PID $PID)"
            else
                echo "$name PID file exists but process not running"
            fi
        else
            echo "$name is not running"
        fi
    done
}

case "$1" in
    start) start ;;
    stop) stop ;;
    status) status ;;
    *) echo "Usage: $0 {start|stop|status}" ;;
esac
