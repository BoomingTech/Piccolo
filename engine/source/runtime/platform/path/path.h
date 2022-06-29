#pragma once

#include <filesystem>
#include <string>
#include <tuple>
#include <vector>

namespace Piccolo
{
    class Path
    {
    public:
        static const std::filesystem::path getRelativePath(const std::filesystem::path& directory,
                                                    const std::filesystem::path& file_path) ;

        static const std::vector<std::string> getPathSegments(const std::filesystem::path& file_path) ;

        static const std::tuple<std::string, std::string, std::string>
        getFileExtensions(const std::filesystem::path& file_path);

        static const std::string getFilePureName(const std::string);
    };
} // namespace Piccolo