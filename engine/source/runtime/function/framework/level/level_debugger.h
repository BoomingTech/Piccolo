#pragma once
#include "runtime/function/framework/object/object.h"
#include "runtime/function/framework/level/level.h"

namespace Piccolo
{
    class LevelDebugger
    {
    public:
        void tick(std::shared_ptr<Level> level);

        //show all bones in a level
        void showAllBones(std::shared_ptr<Level> level);
        //show all bones of a object
        void showBones(std::shared_ptr<Level> level, GObjectID goId);
        //show all bones' name in a level
        void showAllBonesName(std::shared_ptr<Level> level);
        //show all bones' name of a object
        void showBonesName(std::shared_ptr<Level> level, GObjectID goId);
        //show all bindingBox in a level
        void showAllBoundingBox(std::shared_ptr<Level> level);
        //show boundingBox of a object
        void showBoundingBox(std::shared_ptr<Level> level, GObjectID goId);
        //show camera info
        void showCameraInfo(std::shared_ptr<Level> level);
    private:
        void drawBones(std::shared_ptr<GObject> object);
        void drawBonesName(std::shared_ptr<GObject> object);
        void drawBoundingBox(std::shared_ptr<GObject> object);
        void drawCameraInfo(std::shared_ptr<GObject> object);
    };
}