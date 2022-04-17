#pragma once

#include "runtime/core/base/public_singleton.h"

#include <filesystem>
#include <string>
#include <tuple>
#include <vector>

namespace Pilot
{
    class Path : public PublicSingleton<Path>
    {
    public:
        static const std::filesystem::path getRelativePath(const std::filesystem::path& directory,
                                                    const std::filesystem::path& file_path) ;

        static const std::vector<std::string> getPathSegments(const std::filesystem::path& file_path) ;

        const std::tuple<std::string, std::string, std::string>
        getFileExtensions(const std::filesystem::path& file_path) const;

        const std::string getFilePureName(const std::string) const;
    };
} // namespace Pilot