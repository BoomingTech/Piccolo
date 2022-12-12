#pragma once

#include "type_info.h"

class Class;

class Method : public TypeInfo
{

public:
    Method(const Cursor& cursor, const Namespace& current_namespace, Class* parent = nullptr);

    virtual ~Method(void) {}

    bool shouldCompile(void) const;

public:

    Class* m_parent;

    std::string m_name;

    bool isAccessible(void) const;
};