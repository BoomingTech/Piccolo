#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>

#include "runtime/engine.h"

#include "editor/include/editor.h"

// https://gcc.gnu.org/onlinedocs/cpp/Stringizing.html
#define PILOT_XSTR(s) PILOT_STR(s)
#define PILOT_STR(s) #s

int main(int argc, char** argv)
{
    std::filesystem::path executable_path(argv[0]);
    std::filesystem::path config_file_path = executable_path.parent_path() / "PilotEditor.ini";

    Pilot::PilotEngine* engine = new Pilot::PilotEngine();

    engine->startEngine(config_file_path.generic_string());
    engine->initialize();

    Pilot::PilotEditor* editor = new Pilot::PilotEditor();
    editor->initialize(engine);

    editor->run();

    editor->clear();

    engine->clear();
    engine->shutdownEngine();

    return 0;
}
