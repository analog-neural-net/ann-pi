#!/bin/bash

FIFO_PATH="/tmp/cpp_to_py_fifo"

# Step 1: Make sure FIFO exists
if [[ ! -p "$FIFO_PATH" ]]; then
    echo "FIFO not found. Creating it..."
    mkfifo "$FIFO_PATH"
else
    echo "✅ FIFO already exists."
fi

# Step 2: Start the Python transmitter in the background
echo "Starting server..."
# python3 ./model/transmitter.py & 
# PYTHON_PID=$!

# Step 3: Run the C++ sender
echo "Running program"
sudo ./build/main.exe   
