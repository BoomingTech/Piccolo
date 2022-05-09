#include "runtime/function/framework/level/level.h"

#include "runtime/core/base/macro.h"

#include "runtime/resource/asset_manager/asset_manager.h"
#include "runtime/resource/res_type/common/level.h"

#include "runtime/engine.h"
#include "runtime/function/character/character.h"
#include "runtime/function/framework/object/object.h"
#include "runtime/function/scene/scene_manager.h"

#include <limits>

namespace Pilot
{
    Level::~Level() { clear(); }

    void Level::clear()
    {
        if (m_current_active_character)
        {
            delete m_current_active_character;
            m_current_active_character = nullptr;
        }

        for (auto& id_gobject_pair : m_gobjects)
        {
            GObject* gobject = id_gobject_pair.second;
            assert(gobject);
            if (gobject)
            {
                delete gobject;
            }
        }
        m_gobjects.clear();
    }

    const size_t Level::createObject(const ObjectInstanceRes& object_instance_res)
    {
        size_t   gobject_id = PILOT_INVALID_GOBJECT_ID;
        GObject* gobject    = new GObject(m_next_gobject_id);
        assert(gobject);
        if (gobject == nullptr)
        {
            LOG_ERROR("cannot allocate memory for new gobject");
        }
        else
        {
            bool is_loaded = gobject->load(object_instance_res);
            assert(is_loaded);
            if (is_loaded)
            {
                gobject_id = m_next_gobject_id;
                m_gobjects.emplace(gobject_id, gobject);
                ++m_next_gobject_id;
            }
            else
            {
                LOG_ERROR("loading object " + object_instance_res.m_name + " failed");
                delete gobject;
            }
        }
        return gobject_id;
    }

    bool Level::load(const std::string& level_res_url)
    {
        LOG_INFO("loading level: {}", level_res_url);

        m_level_res_url = level_res_url;

        LevelRes level_res;
        const bool is_load_success = AssetManager::getInstance().loadAsset(level_res_url, level_res);
        if (is_load_success == false)
        {
            return false;
        }

        for (const ObjectInstanceRes& object_instance_res : level_res.m_objects)
        {
            createObject(object_instance_res);
        }

        if (level_res.m_character_index >= 0 && level_res.m_character_index < m_gobjects.size())
        {
            GObject* character_object  = m_gobjects[level_res.m_character_index];
            m_current_active_character = new Character(character_object);
        }

        m_is_loaded = true;

        LOG_INFO("level load succeed");

        return true;
    }

    void Level::unload()
    {
        clear();
        LOG_INFO("unload level: {}", m_level_res_url);
    }

    bool Level::save()
    {
        LOG_INFO("saving level: {}", m_level_res_url);
        LevelRes output_level_res;

        const size_t                    object_cout    = m_gobjects.size();
        std::vector<ObjectInstanceRes>& output_objects = output_level_res.m_objects;
        output_objects.resize(object_cout);

        size_t object_index = 0;
        for (const auto& id_object_pair : m_gobjects)
        {
            assert(id_object_pair.second);
            if (id_object_pair.second)
            {
                id_object_pair.second->save(output_objects[object_index]);
                ++object_index;
            }
        }

        const bool is_save_success = AssetManager::getInstance().saveAsset(output_level_res, m_level_res_url);

        if (is_save_success == false)
        {
            LOG_ERROR("failed to save {}", m_level_res_url);
        }
        else
        {
            LOG_INFO("level save succeed");
        }

        return is_save_success;
    }

    void Level::tick(float delta_time)
    {
        if (!m_is_loaded)
        {
            return;
        }

        for (const auto& id_object_pair : m_gobjects)
        {
            assert(id_object_pair.second);
            if (id_object_pair.second)
            {
                id_object_pair.second->tick(delta_time);
            }
        }
        if (m_current_active_character && g_is_editor_mode == false)
        {
            m_current_active_character->tick();
        }
        SceneManager::getInstance().syncSceneObjects();
    }

    GObject* Level::getGObjectByID(size_t go_id) const
    {
        auto iter = m_gobjects.find(go_id);
        if (iter != m_gobjects.end())
        {
            return iter->second;
        }

        return nullptr;
    }

    void Level::deleteGObjectByID(size_t go_id)
    {
        auto iter = m_gobjects.find(go_id);
        if (iter != m_gobjects.end())
        {
            GObject* object = iter->second;
            assert(object);
            if (object)
            {
                if (m_current_active_character && m_current_active_character->getObject() == object)
                {
                    m_current_active_character->setObject(nullptr);
                }
            }
            delete object;
        }

        m_gobjects.erase(go_id);
    }

} // namespace Pilot
