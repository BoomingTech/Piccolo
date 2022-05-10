#pragma once

#include "runtime/function/framework/object/object_id_allocator.h"

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

        const std::unordered_map<GObjectID, GObject*>& getAllGObjects() const { return m_gobjects; }

        GObject*   getGObjectByID(GObjectID go_id) const;
        Character* getCurrentActiveCharacter() const { return m_current_active_character; }

        GObjectID createObject(const ObjectInstanceRes& object_instance_res);
        void      deleteGObjectByID(GObjectID go_id);

    protected:
        void clear();

        bool        m_is_loaded {false};
        std::string m_level_res_url;

        // all game objects in this level, key: object id, value: object instance
        std::unordered_map<GObjectID, GObject*> m_gobjects;

        Character* m_current_active_character {nullptr};
    };
} // namespace Pilot
