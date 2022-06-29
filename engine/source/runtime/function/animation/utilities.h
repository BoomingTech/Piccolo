#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace Piccolo
{
    class Bone;
    class RawBone;
    class SkeletonData;

    template<typename T>
    size_t index(const std::vector<T>& vec, const T& value)
    {
        return std::distance(vec.begin(), std::find(vec.begin(), vec.end(), value));
    }

    template<typename T>
    void append_vector(std::vector<T>& base, const std::vector<T>& addition)
    {
        base.insert(base.end(), addition.begin(), addition.end());
    }

    Bone*                    find_by_index(Bone* bones, int key, int size, bool is_flat = false);
    std::shared_ptr<RawBone> find_by_index(std::vector<std::shared_ptr<RawBone>>& bones, int key, bool is_flat = false);
    int                      find_index_by_name(const SkeletonData& skeleton, const std::string& name);
} // namespace Piccolo