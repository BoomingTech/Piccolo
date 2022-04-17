#pragma once

#include "runtime/core/base/public_singleton.h"
#include "runtime/core/math/vector2.h"

#include <memory>

namespace Pilot
{
    class EditorUI;
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

        void   onWindowChanged(float pos_x, float pos_y, float width, float height) const;
        size_t onUpdateCursorOnAxis(int axis_mode, const Vector2& cursor_uv, const Vector2& window_size) const;
        size_t getGuidOfPickedMesh(const Vector2& picked_uv) const;

        std::shared_ptr<EditorUI> m_editor_ui;
        PilotEngine*              m_engine_runtime {nullptr};
    };
} // namespace Pilot
