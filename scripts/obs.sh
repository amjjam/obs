#!/bin/bash

# Get directory where this script resides
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PID_DIR="$SCRIPT_DIR/../pids"
LOG_DIR="$SCRIPT_DIR/../logs"

mkdir -p "$PID_DIR" "$LOG_DIR"

# Set library path relative to the script
export LD_LIBRARY_PATH="$SCRIPT_DIR/../lib:$LD_LIBRARY_PATH"

# Applications to run
APPS=(DataProcessorGUI FrameViewer PowerSpectrumViewer)

start() {
    echo "Starting GUI applications..."

    for app in "${APPS[@]}"; do
        CMD="$SCRIPT_DIR/../bin/$app"

        if [[ ! -x "$CMD" ]]; then
            echo "Warning: $app not found or not executable at $CMD"
            continue
        fi

        # Prevent double start
        PID_FILE="$PID_DIR/${app}.pid"
        if [[ -f "$PID_FILE" ]] && kill -0 $(cat "$PID_FILE") 2>/dev/null; then
            echo "$app already running, skipping"
            continue
        fi

        echo "Launching $app..."

        "$CMD" > "$LOG_DIR/${app}.out" 2> "$LOG_DIR/${app}.err" &
        PID=$!

        echo $PID > "$PID_FILE"
        echo "$app started (PID $PID)"
    done

    echo "All GUI applications started."
}

stop() {
    echo "Stopping GUI applications..."

    for app in "${APPS[@]}"; do
        PID_FILE="$PID_DIR/${app}.pid"

        if [[ -f "$PID_FILE" ]]; then
            PID=$(cat "$PID_FILE")

            if kill -0 "$PID" 2>/dev/null; then
                kill "$PID"
                echo "Stopped $app (PID $PID)"
            else
                echo "$app not running (stale PID $PID?)"
            fi

            rm -f "$PID_FILE"
        else
            echo "No PID file for $app"
        fi
    done
}

status() {
    for app in "${APPS[@]}"; do
        PID_FILE="$PID_DIR/${app}.pid"

        if [[ -f "$PID_FILE" ]]; then
            PID=$(cat "$PID_FILE")

            if kill -0 "$PID" 2>/dev/null; then
                echo "$app is running (PID $PID)"
            else
                echo "$app PID file exists but process not running"
            fi
        else
            echo "$app is not running"
        fi
    done
}

case "$1" in
    start)
        start
        ;;
    stop)
        stop
        ;;
    status)
        status
        ;;
    *)
        echo "Usage: $0 {start|stop|status}"
        ;;
esac
