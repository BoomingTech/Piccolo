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
    std::filesystem::path pilot_root_folder = std::filesystem::path(PILOT_XSTR(PILOT_ROOT_DIR));

    Pilot::EngineInitParams params;
    params.m_root_folder      = pilot_root_folder;
    params.m_config_file_path = pilot_root_folder / "PilotEditor.ini";

    Pilot::PilotEngine* engine = new Pilot::PilotEngine();

    engine->startEngine(params);
    engine->initialize();

    Pilot::PilotEditor* editor = new Pilot::PilotEditor();
    editor->initialize(engine);

    editor->run();

    editor->clear();

    engine->clear();
    engine->shutdownEngine();

    return 0;
}
