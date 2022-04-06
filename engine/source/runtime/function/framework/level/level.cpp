#include "runtime/function/framework/level/level.h"

#include "runtime/core/base/macro.h"

#include "runtime/resource/asset_manager/asset_manager.h"
#include "runtime/resource/res_type/common/level.h"

#include "runtime/function/framework/object/object.h"
#include "runtime/function/scene/scene_manager.h"

#include <limits>

namespace Pilot
{
    Level::~Level() { clear(); }

    void Level::clear()
    {
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

    void Level::load(const std::string& level_res_url)
    {
        m_level_res_url = level_res_url;

        AssetManager& asset_manager = AssetManager::getInstance();

        LevelRes level_res;
        asset_manager.loadAsset(asset_manager.getFullPath(level_res_url), level_res);

        for (const ObjectInstanceRes& object_instance_res : level_res.m_objects)
        {
            createObject(object_instance_res);
        }
    }

    void Level::save()
    {
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

        AssetManager& asset_manager = AssetManager::getInstance();
        asset_manager.saveAsset(output_level_res, asset_manager.getFullPath(m_level_res_url));
    }

    void Level::tickAll(float delta_time)
    {
        for (const auto& id_object_pair : m_gobjects)
        {
            assert(id_object_pair.second);
            if (id_object_pair.second)
            {
                id_object_pair.second->tick(delta_time);
            }
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
                object->destory();
            }
        }

        m_gobjects.erase(go_id);
    }

} // namespace Pilot
