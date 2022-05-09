#pragma once

#include <string>
#include <unordered_map>

namespace Pilot
{
    class Character;
    class GObject;
    class ObjectInstanceRes;

    class Level
    {
    public:
        virtual ~Level();

        bool load(const std::string& level_res_url);
        void unload();

        bool save();

        void tick(float delta_time);

        const std::string& getLevelResUrl() const { return m_level_res_url; }

        const std::unordered_map<size_t, GObject*>& getAllGObjects() const { return m_gobjects; }

        GObject*   getGObjectByID(size_t go_id) const;
        Character* getCurrentActiveCharacter() const { return m_current_active_character; }

        const size_t createObject(const ObjectInstanceRes& object_instance_res);
        void         deleteGObjectByID(size_t go_id);

    protected:
        void clear();

        bool                                 m_is_loaded {k_false};
        std::string                          m_level_res_url;
        size_t                               m_next_gobject_id {0};
        std::unordered_map<size_t, GObject*> m_gobjects;

        Character* m_current_active_character;
    };
} // namespace Pilot
