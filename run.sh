#!/bin/bash

FIFO_PATH="/tmp/cpp_to_py_fifo"
PY_SCRIPT="/home/anne/ann-pi/model/transmitter.py"
CPP_BINARY="/home/anne/ann-pi/build/main.exe"

# Create FIFO if it doesn't exist
if [[ ! -p "$FIFO_PATH" ]]; then
    echo "Creating FIFO at $FIFO_PATH..."
    mkfifo "$FIFO_PATH"
else
    echo "✅ FIFO already exists."
fi

# Set CPU governor to performance
echo "Setting CPU governor to performance..."
for CPU in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
    echo performance | sudo tee "$CPU" > /dev/null
done

echo "Starting Python server with real-time priority..."
sudo chrt -f 80 ionice -c1 -n0 python3 "$PY_SCRIPT" &
PYTHON_PID=$!

echo "Starting C++ daemon with real-time priority..."
sudo chrt -f 90 ionice -c1 -n0 "$CPP_BINARY" &
CPP_PID=$!

echo "System running."

trap "echo 'Stopping...'; kill $CPP_PID $PYTHON_PID; exit" INT TERM

wait
