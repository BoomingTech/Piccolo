#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>

#include "runtime/engine.h"

#include "editor/include/editor.h"

#include "runtime/core/profile/instrumentor.h"

// https://gcc.gnu.org/onlinedocs/cpp/Stringizing.html
#define PICCOLO_XSTR(s) PICCOLO_STR(s)
#define PICCOLO_STR(s) #s

int main(int argc, char** argv)
{
    PICCOLO_PROFILE_BEGIN_SESSION("initialize", "initialize.json");

    std::filesystem::path executable_path(argv[0]);
    std::filesystem::path config_file_path = executable_path.parent_path() / "PiccoloEditor.ini";

    Piccolo::PiccoloEngine* engine = new Piccolo::PiccoloEngine();

    engine->startEngine(config_file_path.generic_string());
    engine->initialize();

    Piccolo::PiccoloEditor* editor = new Piccolo::PiccoloEditor();
    editor->initialize(engine);

    PICCOLO_PROFILE_END_SESSION();

    PICCOLO_PROFILE_BEGIN_SESSION("tick", "tick.json");
    editor->run();
    PICCOLO_PROFILE_END_SESSION();

    PICCOLO_PROFILE_BEGIN_SESSION("clear", "clear.json");

    editor->clear();

    engine->clear();
    engine->shutdownEngine();

    PICCOLO_PROFILE_END_SESSION();

    return 0;
}
