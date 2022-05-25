#include "runtime/function/render/render_system.h"

#include "runtime/core/base/macro.h"

#include "runtime/resource/asset_manager/asset_manager.h"
#include "runtime/resource/config_manager/config_manager.h"

#include "runtime/function/render/render_camera.h"
#include "runtime/function/render/render_pipeline.h"
#include "runtime/function/render/render_resource.h"
#include "runtime/function/render/render_resource_base.h"
#include "runtime/function/render/render_scene.h"
#include "runtime/function/render/rhi/vulkan/vulkan_rhi.h"
#include "runtime/function/render/window_system.h"

namespace Pilot
{
    RenderSystem::~RenderSystem() {}

    void RenderSystem::initialize(RenderSystemInitInfo init_info)
    {
        std::shared_ptr<ConfigManager> config_manager = g_runtime_global_context.m_config_manager;
        ASSERT(config_manager);
        std::shared_ptr<AssetManager> asset_manager = g_runtime_global_context.m_asset_manager;
        ASSERT(asset_manager);

        // render context initialize
        RHIInitInfo rhi_init_info;
        rhi_init_info.window_system = init_info.window_system;

        m_rhi = std::make_shared<VulkanRHI>();
        m_rhi->initialize(rhi_init_info);

        // global rendering resource
        GlobalRenderingRes global_rendering_res;
        const std::string& global_rendering_res_url = config_manager->getGlobalRenderingResUrl();
        asset_manager->loadAsset(global_rendering_res_url, global_rendering_res);

        // upload ibl, color grading textures
        LevelResourceDesc level_resource_desc;
        level_resource_desc.ibl_resource_desc.skybox_irradiance_map = global_rendering_res.m_skybox_irradiance_map;
        level_resource_desc.ibl_resource_desc.skybox_specular_map   = global_rendering_res.m_skybox_specular_map;
        level_resource_desc.ibl_resource_desc.brdf_map              = global_rendering_res.m_brdf_map;
        level_resource_desc.color_grading_resource_desc.color_grading_map = global_rendering_res.m_color_grading_map;

        m_render_resource = std::make_shared<RenderResource>();
        m_render_resource->uploadGlobalRenderResource(m_rhi, level_resource_desc);
        m_render_resource->setVisibleNodesReference();

        // setup render camera
        const CameraPose& camera_pose = global_rendering_res.m_camera_config.m_pose;
        m_render_camera               = std::make_shared<RenderCamera>();
        m_render_camera->lookAt(camera_pose.m_position, camera_pose.m_target, camera_pose.m_up);
        m_render_camera->m_zfar  = global_rendering_res.m_camera_config.m_z_far;
        m_render_camera->m_znear = global_rendering_res.m_camera_config.m_z_near;
        m_render_camera->setAspect(global_rendering_res.m_camera_config.m_aspect.x /
                                   global_rendering_res.m_camera_config.m_aspect.y);

        // setup render scene
        m_render_scene                  = std::make_shared<RenderScene>();
        m_render_scene->m_ambient_light = {global_rendering_res.m_ambient_light.toVector3()};
        m_render_scene->m_directional_light.m_direction =
            global_rendering_res.m_directional_light.m_direction.normalisedCopy();
        m_render_scene->m_directional_light.m_color = global_rendering_res.m_directional_light.m_color.toVector3();

        // initialize render pipeline
        RenderPipelineInitInfo pipeline_init_info;
        pipeline_init_info.render_resource = m_render_resource;

        m_render_pipeline        = std::make_shared<RenderPipeline>();
        m_render_pipeline->m_rhi = m_rhi;
        m_render_pipeline->initialize(pipeline_init_info);

        // descriptor layout in main camera pass will be used when uploading resource
        std::static_pointer_cast<RenderResource>(m_render_resource)->m_main_camera_pass =
            m_render_pipeline->m_main_camera_pass;
    }

    void RenderSystem::tick()
    {
        // process swap data between logic and render contexts
        processSwapData();

        // prepare render command context
        m_rhi->prepareContext();

        // update per-frame buffer
        m_render_resource->updatePerFrameBuffer(m_render_scene, m_render_camera);

        // update per-frame visible objects
        m_render_resource->updateVisibleObjects(m_render_scene, m_render_camera);

        // prepare pipeline's render passes data
        m_render_pipeline->preparePassData(m_render_resource);

        // render one frame
        if (m_render_pipeline_type == RENDER_PIPELINE_TYPE::FORWARD_PIPELINE)
        {
            m_render_pipeline->forwardRender(m_rhi, m_render_resource);
        }
        else if (m_render_pipeline_type == RENDER_PIPELINE_TYPE::DEFERRED_PIPELINE)
        {
            m_render_pipeline->deferredRender(m_rhi, m_render_resource);
        }
        else
        {
            LOG_ERROR(__FUNCTION__, "unsupported render pipeline type");
        }
    }

    void RenderSystem::swapLogicRenderData() { m_swap_context.swapLogicRenderData(); }

    RenderSwapContext& RenderSystem::getSwapContext() { return m_swap_context; }

    std::shared_ptr<RenderCamera> RenderSystem::getRenderCamera() const { return m_render_camera; }

    void RenderSystem::updateEngineContentViewport(float offset_x, float offset_y, float width, float height)
    {
        std::static_pointer_cast<VulkanRHI>(m_rhi)->_viewport.x        = offset_x;
        std::static_pointer_cast<VulkanRHI>(m_rhi)->_viewport.y        = offset_y;
        std::static_pointer_cast<VulkanRHI>(m_rhi)->_viewport.width    = width;
        std::static_pointer_cast<VulkanRHI>(m_rhi)->_viewport.height   = height;
        std::static_pointer_cast<VulkanRHI>(m_rhi)->_viewport.minDepth = 0.0f;
        std::static_pointer_cast<VulkanRHI>(m_rhi)->_viewport.maxDepth = 1.0f;

        m_render_camera->setAspect(width / height);
    }

    uint32_t RenderSystem::getGuidOfPickedMesh(const Vector2& picked_uv)
    {
        return m_render_pipeline->getGuidOfPickedMesh(picked_uv);
    }

    GObjectID RenderSystem::getGObjectIDByMeshID(uint32_t mesh_id) const
    {
        auto find_it = m_mesh_object_id_map.find(mesh_id);
        if (find_it != m_mesh_object_id_map.end())
        {
            return find_it->second;
        }
        return GObjectID();
    }

    void RenderSystem::createAxis(std::array<RenderEntity, 3> axis_entities, std::array<RenderMeshData, 3> mesh_datas)
    {
        for (int i = 0; i < axis_entities.size(); i++)
        {
            m_render_resource->uploadGameObjectRenderResource(m_rhi, axis_entities[i], mesh_datas[i]);
        }
    }

    void RenderSystem::setVisibleAxis(std::optional<RenderEntity> axis)
    {
        m_render_scene->m_render_axis = axis;

        if (axis.has_value())
        {
            std::static_pointer_cast<RenderPipeline>(m_render_pipeline)->setAxisVisibleState(true);
        }
        else
        {
            std::static_pointer_cast<RenderPipeline>(m_render_pipeline)->setAxisVisibleState(false);
        }
    }

    void RenderSystem::setSelectedAxis(size_t selected_axis)
    {
        std::static_pointer_cast<RenderPipeline>(m_render_pipeline)->setSelectedAxis(selected_axis);
    }

    GuidAllocator<GameObjectPartId>& RenderSystem::getGOInstanceIdAllocator() { return m_instance_id_allocator; }

    GuidAllocator<MeshSourceDesc>& RenderSystem::getMeshAssetIdAllocator() { return m_mesh_asset_id_allocator; }

    void RenderSystem::clearForLevelReloading()
    {
        m_instance_id_allocator.clear();
        m_mesh_object_id_map.clear();
        m_render_scene->m_render_entities.clear();
    }

    void RenderSystem::setRenderPipelineType(RENDER_PIPELINE_TYPE pipeline_type)
    {
        m_render_pipeline_type = pipeline_type;
    }

    void RenderSystem::initializeUIRenderBackend(WindowUI* window_ui)
    {
        m_render_pipeline->initializeUIRenderBackend(window_ui);
    }

    void RenderSystem::processSwapData()
    {
        RenderSwapData& swap_data = m_swap_context.getRenderSwapData();

        std::shared_ptr<AssetManager> asset_manager = g_runtime_global_context.m_asset_manager;
        ASSERT(asset_manager);

        // TODO: update global resources if needed
        if (swap_data.level_resource_desc.has_value())
        {
            m_render_resource->uploadGlobalRenderResource(m_rhi, *swap_data.level_resource_desc);

            // reset level resource swap data to a clean state
            m_swap_context.resetLevelRsourceSwapData();
        }

        // update game object if needed
        if (swap_data.game_object_resource_desc.has_value())
        {
            while (!swap_data.game_object_resource_desc->isEmpty())
            {
                GameObjectDesc gobject = swap_data.game_object_resource_desc->getNextProcessObject();

                for (size_t i = 0; i < gobject.getObjectParts().size(); i++)
                {
                    const auto&      component = gobject.getObjectParts()[i];
                    GameObjectPartId part_id   = {gobject.getId(), i};

                    bool is_entity_in_scene = m_instance_id_allocator.hasElement(part_id);

                    RenderEntity render_entity;
                    render_entity.m_instance_id  = static_cast<uint32_t>(m_instance_id_allocator.allocGuid(part_id));
                    render_entity.m_model_matrix = component.transform_desc.transform_matrix;

                    addInstanceIdToMap(render_entity.m_instance_id, gobject.getId());

                    // mesh properties
                    MeshSourceDesc mesh_source    = {component.mesh_desc.mesh_file};
                    bool           is_mesh_loaded = m_mesh_asset_id_allocator.hasElement(mesh_source);

                    RenderMeshData mesh_data;
                    if (!is_mesh_loaded)
                    {
                        mesh_data = m_render_resource->loadMeshData(mesh_source, render_entity.m_bounding_box);
                    }
                    else
                    {
                        render_entity.m_bounding_box = m_render_resource->getCachedBoudingBox(mesh_source);
                    }

                    render_entity.m_mesh_asset_id = m_mesh_asset_id_allocator.allocGuid(mesh_source);
                    render_entity.m_enable_vertex_blending =
                        component.skeleton_animation_result.transforms.size() > 1; // take care
                    render_entity.m_joint_matrices.resize(component.skeleton_animation_result.transforms.size());
                    for (size_t i = 0; i < component.skeleton_animation_result.transforms.size(); ++i)
                    {
                        render_entity.m_joint_matrices[i] = component.skeleton_animation_result.transforms[i].matrix;
                    }

                    // material properties
                    MaterialSourceDesc material_source;
                    if (component.material_desc.with_texture)
                    {
                        material_source = {component.material_desc.baseColorTextureFile,
                                           component.material_desc.metallicRoughnessTextureFile,
                                           component.material_desc.normalTextureFile,
                                           component.material_desc.occlusionTextureFile,
                                           component.material_desc.emissiveTextureFile};
                    }
                    else
                    {
                        // TODO: move to default material definition json file
                        material_source = {
                            asset_manager->getFullPath("asset/texture/default/albedo.jpg").generic_string(),
                            asset_manager->getFullPath("asset/texture/default/mr.jpg").generic_string(),
                            asset_manager->getFullPath("asset/texture/default/normal.jpg").generic_string(),
                            "",
                            ""};
                    }
                    bool is_material_loaded = m_material_asset_id_allocator.hasElement(material_source);

                    RenderMaterialData material_data;
                    if (!is_material_loaded)
                    {
                        material_data = m_render_resource->loadMaterialData(material_source);
                    }

                    render_entity.m_material_asset_id = m_material_asset_id_allocator.allocGuid(material_source);

                    // create game object on the graphics api side
                    if (!is_mesh_loaded)
                    {
                        m_render_resource->uploadGameObjectRenderResource(m_rhi, render_entity, mesh_data);
                    }

                    if (!is_material_loaded)
                    {
                        m_render_resource->uploadGameObjectRenderResource(m_rhi, render_entity, material_data);
                    }

                    // add object to render scene if needed
                    if (!is_entity_in_scene)
                    {
                        m_render_scene->m_render_entities.push_back(render_entity);
                    }
                    else
                    {
                        for (auto& entity : m_render_scene->m_render_entities)
                        {
                            if (entity.m_instance_id == render_entity.m_instance_id)
                            {
                                entity = render_entity;
                                break;
                            }
                        }
                    }
                }
                // after finished processing, pop this game object
                swap_data.game_object_resource_desc->popProcessObject();
            }

            // reset game object swap data to a clean state
            m_swap_context.resetGameObjectResourceSwapData();
        }

        // remove deleted objects
        if (swap_data.game_object_to_delete.has_value())
        {
            while (!swap_data.game_object_to_delete->isEmpty())
            {
                GameObjectDesc gobject = swap_data.game_object_to_delete->getNextProcessObject();

                for (auto it = m_mesh_object_id_map.begin(); it != m_mesh_object_id_map.end(); it++)
                {
                    if (it->second == gobject.getId())
                    {
                        m_mesh_object_id_map.erase(it);
                        break;
                    }
                }

                GameObjectPartId part_id = {gobject.getId(), 0};
                size_t           find_guid;
                if (m_instance_id_allocator.getElementGuid(part_id, find_guid))
                {
                    for (auto it = m_render_scene->m_render_entities.begin();
                         it != m_render_scene->m_render_entities.end();
                         it++)
                    {
                        if (it->m_instance_id == find_guid)
                        {
                            m_render_scene->m_render_entities.erase(it);
                            break;
                        }
                    }
                }

                swap_data.game_object_to_delete->popProcessObject();
            }

            m_swap_context.resetGameObjectToDelete();
        }
    }

    void RenderSystem::addInstanceIdToMap(uint32_t instance_id, GObjectID go_id)
    {
        m_mesh_object_id_map[instance_id] = go_id;
    }

} // namespace Pilot
