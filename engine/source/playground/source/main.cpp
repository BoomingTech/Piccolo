#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>

#include "runtime/engine.h"

// https://gcc.gnu.org/onlinedocs/cpp/Stringizing.html
#define PICCOLO_XSTR(s) PICCOLO_STR(s)
#define PICCOLO_STR(s) #s

int main(int argc, char** argv)
{
    std::filesystem::path executable_path(argv[0]);
    std::filesystem::path config_file_path = executable_path.parent_path() / "PiccoloPlayGround.ini";

    Piccolo::PiccoloEngine* engine = new Piccolo::PiccoloEngine();

    engine->startEngine(config_file_path.generic_string());
    engine->initialize();

    engine->run();

    engine->clear();
    engine->shutdownEngine();

    return 0;
}
