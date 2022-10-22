#pragma once
#include "runtime/function/framework/level/level.h"
#include "runtime/function/framework/object/object.h"

namespace Piccolo
{
    class LevelDebugger
    {
    public:
        void tick(std::shared_ptr<Level> level) const;

        // show all bones in a level
        void showAllBones(std::shared_ptr<Level> level) const;
        // show all bones of a object
        void showBones(std::shared_ptr<Level> level, GObjectID go_id) const;
        // show all bones' name in a level
        void showAllBonesName(std::shared_ptr<Level> level) const;
        // show all bones' name of a object
        void showBonesName(std::shared_ptr<Level> level, GObjectID go_id) const;
        // show all bindingBox in a level
        void showAllBoundingBox(std::shared_ptr<Level> level) const;
        // show boundingBox of a object
        void showBoundingBox(std::shared_ptr<Level> level, GObjectID go_id) const;
        // show camera info
        void showCameraInfo(std::shared_ptr<Level> level) const;

    private:
        void drawBones(std::shared_ptr<GObject> object) const;
        void drawBonesName(std::shared_ptr<GObject> object) const;
        void drawBoundingBox(std::shared_ptr<GObject> object) const;
        void drawCameraInfo(std::shared_ptr<GObject> object) const;
    };
} // namespace Piccolo