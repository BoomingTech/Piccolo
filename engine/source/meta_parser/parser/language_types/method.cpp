#include "common/precompiled.h"

#include "class.h"
#include "method.h"

Method::Method(const Cursor& cursor, const Namespace& current_namespace, Class* parent) :
    TypeInfo(cursor, current_namespace), m_parent(parent), m_name(cursor.getSpelling())
{}

bool Method::shouldCompile(void) const { return isAccessible(); }

bool Method::isAccessible(void) const
{
    return ((m_parent->m_meta_data.getFlag(NativeProperty::Methods) ||
             m_parent->m_meta_data.getFlag(NativeProperty::All)) &&
            !m_meta_data.getFlag(NativeProperty::Disable)) ||
           (m_parent->m_meta_data.getFlag(NativeProperty::WhiteListMethods) &&
            m_meta_data.getFlag(NativeProperty::Enable));
}
