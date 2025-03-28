#!/bin/bash

FIFO_PATH="/tmp/cpp_to_py_fifo"

echo "Cleaning build artifacts..."
make clean

echo "⚙️ Building project..."
make

echo "Checking for FIFO at $FIFO_PATH..."
if [[ ! -p "$FIFO_PATH" ]]; then
    echo "FIFO not found. Creating it..."
    mkfifo "$FIFO_PATH"
else
    echo "✅ FIFO already exists."
fi

echo "✅ Build and FIFO setup complete."