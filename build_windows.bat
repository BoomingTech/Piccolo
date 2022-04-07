@echo off

cmake -S engine/ -B build -A x64
cmake --build build --config Release