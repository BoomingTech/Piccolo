@echo off

cmake -S . -B build
cmake --build build --config Release

pause