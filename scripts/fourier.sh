#!/bin/bash

# Get directory where this script resides
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PID_DIR="$SCRIPT_DIR/../pids"
LOG_DIR="$SCRIPT_DIR/../logs"

mkdir -p "$PID_DIR" "$LOG_DIR"

# Set library path relative to the script
export LD_LIBRARY_PATH="$SCRIPT_DIR/../lib:$LD_LIBRARY_PATH"

# Default IP address
DEFAULT_IP="127.0.0.1"

# Pattern for shared memory cleanup
SHM_PATTERN="/dev/shm/frames*"

# Function to clean shared memory files
clean_shm() {
    echo "Cleaning shared memory files matching $SHM_PATTERN..."
    rm -f $SHM_PATTERN 2>/dev/null
}

# Function to build commands based on IP
build_cmds() {
    local IP_ADDR="$1"

    CMD1="$SCRIPT_DIR/../../amjngc/NGC/ngcdcs/src/ngcdcsDtt0 -sender-frames /frames:2:200000 -ofile counts $HOME/amj/tmp/amjngc.counts"
    CMD2="$SCRIPT_DIR/../bin/DataProcessor --receiver-frames /frames:2:200000 \
         --fringeperiod -10 --sender-tracker 127.0.0.1:27001"
    CMD3="$SCRIPT_DIR/../bin/FringeTracker --active 1 \
        --receiver-phasors 127.0.0.1:27001 \
        --sender-pspec ${IP_ADDR}:27006 100 \
        --sender-movements ${IP_ADDR}:27004 \
        --sender-tracker-stats ${IP_ADDR}:27008 10 1000 \
        --sender-tracker-controller ${IP_ADDR}:27009 1000 \
        --receiver-tracker-controller ${IP_ADDR}:27010 \
        --sender-tracker-snr ${IP_ADDR}:27011 \
	--baseline baseline1 256 1 2.5 4 5 1"
    CMD4="$SCRIPT_DIR/../bin/DelayController \
        --receiver-movements ${IP_ADDR}:27004 \
        --sender-display ${IP_ADDR}:27007 100 \
        --sender-delaylines sim 6 ${IP_ADDR}:27003"
}

start() {
    local IP_ADDR="${1:-$DEFAULT_IP}"
    echo "Starting processes (using IP $IP_ADDR)..."
    clean_shm
    build_cmds "$IP_ADDR"

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
    clean_shm
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
    start)
        shift
        start "$1"
        ;;
    stop)
        stop
        ;;
    status)
        status
        ;;
    *)
        echo "Usage: $0 {start [ip]|stop|status}"
        ;;
esac
