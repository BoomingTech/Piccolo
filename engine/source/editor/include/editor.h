#pragma once

#include "runtime/core/base/public_singleton.h"
#include "runtime/core/math/vector2.h"

#include <memory>

namespace Pilot
{
    class EditorUI;
    class EditorSceneManager;
    class PilotEngine;


    class PilotEditor : public PublicSingleton<PilotEditor>
    {
        friend class EditorUI;
        friend class PublicSingleton<PilotEditor>;

    public:
        virtual ~PilotEditor();

        void initialize(PilotEngine* engine_runtime);
        void clear();

        void run();

    protected:
        PilotEditor();

        std::shared_ptr<EditorUI> m_editor_ui;
        PilotEngine* m_engine_runtime{ nullptr };
    };
} // namespace Pilot
