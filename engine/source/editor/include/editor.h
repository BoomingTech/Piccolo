#pragma once

#include "runtime/core/math/vector2.h"

#include <memory>

namespace Piccolo
{
    class EditorUI;
    class PiccoloEngine;

    class PiccoloEditor 
    {
        friend class EditorUI;

    public:
        PiccoloEditor();
        virtual ~PiccoloEditor();

        void initialize(PiccoloEngine* engine_runtime);
        void clear();

        void run();

    protected:
        std::shared_ptr<EditorUI> m_editor_ui;
        PiccoloEngine* m_engine_runtime{ nullptr };
    };
} // namespace Piccolo
