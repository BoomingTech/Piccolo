#pragma once

#include "common/namespace.h"
#include "cursor/cursor.h"

namespace Utils
{

    void toString(const CXString& str, std::string& output);

    std::string getQualifiedName(const CursorType& type);

    std::string getQualifiedName(const std::string& display_name, const Namespace& current_namespace);

    std::string getQualifiedName(const Cursor& cursor, const Namespace& current_namespace);

    std::string formatQualifiedName(std::string& source_string);

    fs::path makeRelativePath(const fs::path& from, const fs::path& to);

    void fatalError(const std::string& error);

    template<typename A, typename B>
    bool rangeEqual(A startA, A endA, B startB, B endB);

    std::vector<std::string> split(std::string input, std::string pat);

    std::string getFileName(std::string path);

    std::string getNameWithoutFirstM(std::string& name);

    std::string getTypeNameWithoutNamespace(const CursorType& type);

    std::string getNameWithoutContainer(std::string name);

    std::string getStringWithoutQuot(std::string input);

    std::string replace(std::string& source_string, std::string sub_string, const std::string new_string);

    std::string replace(std::string& source_string, char taget_char, const char new_char);

    std::string toUpper(std::string& source_string);

    std::string join(std::vector<std::string> context_list, std::string separator);

    std::string trim(std::string& source_string, const std::string trim_chars);

    std::string loadFile(std::string path);

    void saveFile(const std::string& outpu_string, const std::string& output_file);

    void replaceAll(std::string& resource_str, std::string sub_str, std::string new_str);

    unsigned long formatPathString(const std::string& path_string, std::string& out_string);

    std::string convertNameToUpperCamelCase(const std::string& name, std::string pat);

} // namespace Utils

#include "meta_utils.hpp"
