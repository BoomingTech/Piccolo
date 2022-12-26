#include "runtime/resource/asset_manager/asset_manager.h"

#include "runtime/resource/config_manager/config_manager.h"

#include "runtime/function/global/global_context.h"

#include <filesystem>

namespace Piccolo
{
    std::filesystem::path AssetManager::getFullPath(const std::string& relative_path) const
    {
        return std::filesystem::absolute(g_runtime_global_context.m_config_manager->getRootFolder() / relative_path);
    }

    void AssetManager::readTextFile(const std::filesystem::path& file_path, std::string& content)
    {
        std::ifstream fin(file_path, std::ios::in);
        content = {std::istreambuf_iterator<char>(fin), std::istreambuf_iterator<char>()};
    }

    void AssetManager::readBinaryFile(const std::filesystem::path& file_path, std::vector<unsigned char>& content)
    {
        std::ifstream fin;
        fin.open(file_path, std::ios::in | std::ios::binary);

        size_t begin = fin.tellg();
        size_t end   = begin;

        fin.seekg(0, std::ios_base::end);
        end = fin.tellg();
        fin.seekg(0, std::ios_base::beg);

        size_t file_size = 0;
        file_size        = end - begin;

        content.resize(file_size);
        fin.read(reinterpret_cast<char*>(content.data()), sizeof(file_size));
        fin.close();
    }
} // namespace Piccolo