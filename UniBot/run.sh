#!/bin/bash

# Compile backend if not already compiled
if [ ! -f backend/unibot ]; then
    echo "Compiling backend..."
    bash compile.sh
fi

# Start backend in background
echo "Starting C++ backend on localhost:18080..."
./backend/unibot &
BACKEND_PID=$!

# Give backend time to start
sleep 2

# Start frontend proxy server on port 5000
echo "Starting frontend proxy server on 0.0.0.0:5000..."
python3 server.py

# Cleanup on exit
trap "kill $BACKEND_PID 2>/dev/null" EXIT
