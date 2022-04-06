#include "runtime/function/animation/utilities.h"

#include "runtime/function/animation/node.h"

namespace Pilot
{
    Bone* find_by_index(Bone* bones, int key, int size, bool is_flat)
    {
        if (key == std::numeric_limits<int>::max())
            return nullptr;
        if (is_flat)
        {
            if (key >= size)
                return nullptr;
            else
                return &bones[key];
        }
        else
        {
            for (int i = 0; i < size; i++)
            {
                if (bones[i].getID() == key)
                    return &bones[i];
            }
        }
        return nullptr;
    }

    std::shared_ptr<RawBone> find_by_index(std::vector<std::shared_ptr<RawBone>>& bones, int key, bool is_flat)
    {
        if (key == std::numeric_limits<int>::max())
            return nullptr;
        if (is_flat)
        {
            return bones[key];
        }
        else
        {
            for (auto iter : bones)
            {
                if (iter->index == key)
                {
                    return iter;
                }
            }
        }
        return nullptr;
    }

    int find_index_by_name(const SkeletonData& skeleton, std::string name)
    {
        for (auto& iter : skeleton.bones_map)
        {
            if (iter.name == name)
                return iter.index;
        }
        return std::numeric_limits<int>::max();
    }
} // namespace Pilot
