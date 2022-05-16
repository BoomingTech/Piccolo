@echo off

rem export compile_commands.json for clangd lsp
cmake -DCMAKE_TRY_COMPILE_TARGET_TYPE="STATIC_LIBRARY" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B .\compile_db_temp -G "Unix Makefiles"
copy .\compile_db_temp\compile_commands.json .
