@echo off
del build
del bin
cmake -S . -B build
//cmake --build build --config Release

pause