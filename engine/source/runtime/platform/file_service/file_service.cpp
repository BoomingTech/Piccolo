#include "runtime/platform/file_service/file_service.h"

using namespace std;

namespace Pilot
{
    vector<filesystem::path> FileSystem::getFiles(const filesystem::path& directory)
    {
        vector<filesystem::path> files;
        for (auto const& directory_entry : filesystem::recursive_directory_iterator {directory})
        {
            if (directory_entry.is_regular_file())
            {
                files.push_back(directory_entry);
            }
        }
        return files;
    }
} // namespace Pilot