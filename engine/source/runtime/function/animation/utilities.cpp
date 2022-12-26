#include "runtime/function/animation/utilities.h"

#include "runtime/function/animation/node.h"

namespace Piccolo
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
            const auto it = std::find_if(bones.begin(), bones.end(), [&](const auto& i) { return i->m_index == key; });
            if (it != bones.end())
                return *it;
        }
        return nullptr;
    }

    int find_index_by_name(const SkeletonData& skeleton, const std::string& name)
    {
        const auto it = std::find_if(skeleton.m_bones_map.begin(), skeleton.m_bones_map.end(), [&](const auto& i) { return i.m_name == name; });
        if (it != skeleton.m_bones_map.end())
            return it->m_index;
        return std::numeric_limits<int>::max();
    }
} // namespace Piccolo
