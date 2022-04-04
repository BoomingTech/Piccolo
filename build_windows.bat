@echo off

cmake -S engine/ -B build
cmake --build build --config Release