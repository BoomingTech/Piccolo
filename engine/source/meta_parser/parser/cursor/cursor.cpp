#include "common/precompiled.h"
#include "meta/meta_utils.h"

#include "cursor.h"

Cursor::Cursor(const CXCursor& handle) : m_handle(handle) {}

CXCursorKind Cursor::getKind(void) const { return m_handle.kind; }

std::string Cursor::getSpelling(void) const
{
    std::string spelling;

    Utils::toString(clang_getCursorSpelling(m_handle), spelling);

    return spelling;
}

std::string Cursor::getDisplayName(void) const
{
    std::string display_name;

    Utils::toString(clang_getCursorDisplayName(m_handle), display_name);

    return display_name;
}

std::string Cursor::getSourceFile(void) const
{
    auto range = clang_Cursor_getSpellingNameRange(m_handle, 0, 0);

    auto start = clang_getRangeStart(range);

    CXFile   file;
    unsigned line, column, offset;

    clang_getFileLocation(start, &file, &line, &column, &offset);

    std::string filename;

    Utils::toString(clang_getFileName(file), filename);

    return filename;
}

bool Cursor::isDefinition(void) const { return clang_isCursorDefinition(m_handle); }

CursorType Cursor::getType(void) const { return clang_getCursorType(m_handle); }

Cursor::List Cursor::getChildren(void) const
{
    List children;

    auto visitor = [](CXCursor cursor, CXCursor parent, CXClientData data) {
        auto container = static_cast<List*>(data);

        container->emplace_back(cursor);

        if (cursor.kind == CXCursor_LastPreprocessing)
            return CXChildVisit_Break;

        return CXChildVisit_Continue;
    };

    clang_visitChildren(m_handle, visitor, &children);

    return children;
}

void Cursor::visitChildren(Visitor visitor, void* data) { clang_visitChildren(m_handle, visitor, data); }
