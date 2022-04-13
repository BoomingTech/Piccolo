#pragma once

#include "runtime/core/base/public_singleton.h"

#include <filesystem>
#include <vector>

namespace Pilot
{
    class FileService : public PublicSingleton<FileService>
    {
        friend class PublicSingleton<FileService>;

    public:
        std::vector<std::filesystem::path> getFiles(const std::filesystem::path& directory);

    protected:
        FileService() = default;
    };
} // namespace Pilot