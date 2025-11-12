#!/bin/bash
cd backend
g++ -std=c++17 -I.. $NIX_CFLAGS_COMPILE -o unibot main.cpp -lpthread
echo "✓ Backend compiled successfully"
