#!/bin/bash

FIFO_PATH="/tmp/cpp_to_py_fifo"

if [[ ! -p "$FIFO_PATH" ]]; then
    echo "Creating FIFO at $FIFO_PATH..."
    mkfifo "$FIFO_PATH"
else
    echo "✅ FIFO already exists."
fi 

echo "Starting Python server..."
python3 ./model/transmitter.py &
PYTHON_PID=$!

echo "Starting C++ daemon..."
sudo ./build/main.exe &     # Replace with actual binary name
CPP_PID=$!

echo "System running. Press Ctrl+C to stop."

trap "echo 'Stopping...'; kill $CPP_PID $PYTHON_PID; exit" INT

wait
