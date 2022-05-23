#include "runtime/function/framework/component/mesh/mesh_component.h"

#include "runtime/resource/asset_manager/asset_manager.h"
#include "runtime/resource/res_type/data/material.h"

#include "runtime/function/framework/component/animation/animation_component.h"
#include "runtime/function/framework/component/transform/transform_component.h"
#include "runtime/function/framework/object/object.h"
#include "runtime/function/global/global_context.h"

#include "runtime/function/render/render_swap_context.h"

namespace Pilot
{
    RenderSwapContext* MeshComponent::m_swap_context = nullptr;

    void MeshComponent::postLoadResource(std::weak_ptr<GObject> parent_object)
    {
        m_parent_object = parent_object;

        std::shared_ptr<AssetManager> asset_manager = g_runtime_global_context.m_asset_manager;
        ASSERT(asset_manager);

        m_raw_meshes.resize(m_mesh_res.m_sub_meshes.size());

        size_t raw_mesh_count = 0;
        for (const SubMeshRes& sub_mesh : m_mesh_res.m_sub_meshes)
        {
            GameObjectPartDesc& meshComponent = m_raw_meshes[raw_mesh_count];
            meshComponent.mesh_desc.mesh_file = asset_manager->getFullPath(sub_mesh.m_obj_file_ref).generic_string();

            meshComponent.material_desc.with_texture = sub_mesh.m_material.empty() == false;

            if (meshComponent.material_desc.with_texture)
            {
                MaterialRes material_res;
                asset_manager->loadAsset(sub_mesh.m_material, material_res);

                meshComponent.material_desc.baseColorTextureFile =
                    asset_manager->getFullPath(material_res.m_base_colour_texture_file).generic_string();
                meshComponent.material_desc.metallicRoughnessTextureFile =
                    asset_manager->getFullPath(material_res.m_metallic_roughness_texture_file).generic_string();
                meshComponent.material_desc.normalTextureFile =
                    asset_manager->getFullPath(material_res.m_normal_texture_file).generic_string();
                meshComponent.material_desc.occlusionTextureFile =
                    asset_manager->getFullPath(material_res.m_occlusion_texture_file).generic_string();
                meshComponent.material_desc.emissiveTextureFile =
                    asset_manager->getFullPath(material_res.m_emissive_texture_file).generic_string();
            }

            auto object_space_transform                   = sub_mesh.m_transform.getMatrix();
            meshComponent.transform_desc.transform_matrix = object_space_transform;

            ++raw_mesh_count;
        }
    }

    void MeshComponent::tick(float delta_time)
    {
        if (!m_parent_object.lock())
            return;

        TransformComponent*       transform_component = m_parent_object.lock()->tryGetComponent(TransformComponent);
        const AnimationComponent* animation_component =
            m_parent_object.lock()->tryGetComponentConst(AnimationComponent);

        if (transform_component->isDirty())
        {
            std::vector<GameObjectPartDesc> mesh_components;
            Pilot::SkeletonAnimationResult  animResult;
            animResult.transforms.push_back({Matrix4x4::IDENTITY});
            if (animation_component != nullptr)
            {
                for (auto& node : animation_component->getResult().node)
                {
                    Pilot::SkeletonAnimationResultTransform tmp {Matrix4x4(node.transform)};
                    animResult.transforms.push_back(tmp);
                }
            }
            for (GameObjectPartDesc& mesh_component : m_raw_meshes)
            {
                if (animation_component)
                {
                    mesh_component.with_animation                              = true;
                    mesh_component.skeleton_animation_result                   = animResult;
                    mesh_component.skeleton_binding_desc.skeleton_binding_file = mesh_component.mesh_desc.mesh_file;
                }
                Matrix4x4 object_transform_matrix = mesh_component.transform_desc.transform_matrix;

                mesh_component.transform_desc.transform_matrix =
                    transform_component->getMatrix() * object_transform_matrix;
                mesh_components.push_back(mesh_component);

                mesh_component.transform_desc.transform_matrix = object_transform_matrix;
            }

            m_swap_context->getLogicSwapData().addDirtyGameObject(
                GameObjectDesc {m_parent_object.lock()->getID(), mesh_components});

            transform_component->setDirtyFlag(false);
        }
    }
} // namespace Pilot
