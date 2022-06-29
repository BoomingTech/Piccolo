#include "runtime/platform/path/path.h"

using namespace std;

namespace Piccolo
{
    const filesystem::path Path::getRelativePath(const filesystem::path& directory,
                                                 const filesystem::path& file_path)
    {
        return file_path.lexically_relative(directory);
    }

    const vector<string> Path::getPathSegments(const filesystem::path& file_path)
    {
        vector<string> segments;
        for (auto iter = file_path.begin(); iter != file_path.end(); ++iter)
        {
            segments.emplace_back(iter->generic_string());
        }
        return segments;
    }

    const tuple<string, string, string> Path::getFileExtensions(const filesystem::path& file_path)
    {
        return make_tuple(file_path.extension().generic_string(),
                          file_path.stem().extension().generic_string(),
                          file_path.stem().stem().extension().generic_string());
    }

    const string Path::getFilePureName(const string file_full_name)
    {
        string file_pure_name = file_full_name;
        auto   pos            = file_full_name.find_first_of('.');
        if (pos != string::npos)
        {
            file_pure_name = file_full_name.substr(0, pos);
        }

        return file_pure_name;
    }
} // namespace Piccolo
