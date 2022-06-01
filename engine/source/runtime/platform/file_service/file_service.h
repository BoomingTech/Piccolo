#pragma once

#include <filesystem>
#include <vector>

namespace Pilot
{
    class FileSystem 
    {
    public:
        std::vector<std::filesystem::path> getFiles(const std::filesystem::path& directory);
    };
} // namespace Pilot