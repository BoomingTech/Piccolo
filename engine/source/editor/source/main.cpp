#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>

#include "editor/include/editor.h"
#include "runtime/engine.h"

// https://gcc.gnu.org/onlinedocs/cpp/Stringizing.html
#define PILOT_XSTR(s) PILOT_STR(s)
#define PILOT_STR(s) #s

int main(int argc, char** argv)
{
    std::filesystem::path pilot_root_folder = std::filesystem::path(PILOT_XSTR(PILOT_ROOT_DIR));

    Pilot::EngineInitParams params;
    params.m_root_folder      = pilot_root_folder;
    params.m_config_file_path = pilot_root_folder / "PilotEditor.ini";

    Pilot::PilotEngine::getInstance().startEngine(params);
    Pilot::PilotEngine::getInstance().initialize();

    Pilot::PilotEditor::getInstance().initialize(&(Pilot::PilotEngine::getInstance()));

    Pilot::PilotEditor::getInstance().run();

    Pilot::PilotEditor::getInstance().clear();

    Pilot::PilotEngine::getInstance().clear();
    Pilot::PilotEngine::getInstance().shutdownEngine();

    return 0;
}
