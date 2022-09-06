cmake -S . -B build
cmake --build build --config Release --parallel 
cd bin
./PiccoloEditor
