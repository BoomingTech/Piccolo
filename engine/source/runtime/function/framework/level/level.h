#pragma once

#include <string>
#include <unordered_map>

namespace Pilot
{
    class GObject;
    class ObjectInstanceRes;

    class Level
    {
    protected:
        size_t                               m_next_gobject_id {0};
        std::string                          m_level_res_url;
        std::unordered_map<size_t, GObject*> m_gobjects;

    public:
        ~Level();
        void clear();

        void load(const std::string& level_res_url);
        void save();

        void tickAll(float delta_time);

        const std::string& getLevelResUrl() const { return m_level_res_url; }

        const std::unordered_map<size_t, GObject*>& getAllGObjects() const { return m_gobjects; }
        GObject*                                    getGObjectByID(size_t go_id) const;

        const size_t createObject(const ObjectInstanceRes& object_instance_res);
        void         deleteGObjectByID(size_t go_id);
    };
} // namespace Pilot
