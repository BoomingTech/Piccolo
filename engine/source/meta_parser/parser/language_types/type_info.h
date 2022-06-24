#pragma once
#include "common/namespace.h"

#include "cursor/cursor.h"

#include "meta/meta_info.h"
#include "parser/parser.h"

class TypeInfo
{
public:
    TypeInfo(const Cursor& cursor, const Namespace& current_namespace);
    virtual ~TypeInfo(void) {}

    const MetaInfo& getMetaData(void) const;

    std::string getSourceFile(void) const;

    Namespace getCurrentNamespace() const;

    Cursor& getCurosr();

protected:
    MetaInfo m_meta_data;

    bool m_enabled;

    std::string m_alias_cn;

    Namespace m_namespace;

private:
    // cursor that represents the root of this language type
    Cursor m_root_cursor;
};