#include "common/precompiled.h"

#include "class.h"

BaseClass::BaseClass(const Cursor& cursor) : name(Utils::getTypeNameWithoutNamespace(cursor.getType())) {}

Class::Class(const Cursor& cursor, const Namespace& current_namespace) :
    TypeInfo(cursor, current_namespace), m_name(cursor.getDisplayName()),
    m_qualified_name(Utils::getTypeNameWithoutNamespace(cursor.getType())),
    m_display_name(Utils::getNameWithoutFirstM(m_qualified_name))
{
    Utils::replaceAll(m_name, " ", "");
    Utils::replaceAll(m_name, "Piccolo::", "");

    for (auto& child : cursor.getChildren())
    {
        switch (child.getKind())
        {
            case CXCursor_CXXBaseSpecifier: {
                auto base_class = new BaseClass(child);

                m_base_classes.emplace_back(base_class);
            }
            break;
            // field
            case CXCursor_FieldDecl:
                m_fields.emplace_back(new Field(child, current_namespace, this));
                break;
            default:
                break;
        }
    }
}

bool Class::shouldCompile(void) const { return shouldCompileFilds(); }

bool Class::shouldCompileFilds(void) const
{
    return m_meta_data.getFlag(NativeProperty::All) || m_meta_data.getFlag(NativeProperty::Fields) ||
           m_meta_data.getFlag(NativeProperty::WhiteListFields);
}

std::string Class::getClassName(void) { return m_name; }

bool Class::isAccessible(void) const { return m_enabled; }