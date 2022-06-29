#pragma once

#include "cursor_type.h"

class Cursor
{
public:
    typedef std::vector<Cursor> List;
    
    typedef CXCursorVisitor Visitor;

    Cursor(const CXCursor& handle);

    CXCursorKind getKind(void) const;

    std::string getSpelling(void) const;
    std::string getDisplayName(void) const;

    std::string getSourceFile(void) const;

    bool isDefinition(void) const;

    CursorType getType(void) const;

    List getChildren(void) const;
    void visitChildren(Visitor visitor, void* data = nullptr);

private:
    CXCursor m_handle;
};