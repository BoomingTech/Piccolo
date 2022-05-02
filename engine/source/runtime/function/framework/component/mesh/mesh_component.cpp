#include "runtime/function/scene/scene_manager.h"

#include "runtime/resource/asset_manager/asset_manager.h"
#include "runtime/resource/res_type/data/material.h"

#include "runtime/function/framework/component/animation/animation_component.h"
#include "runtime/function/framework/component/mesh/mesh_component.h"
#include "runtime/function/framework/component/transform/transform_component.h"
#include "runtime/function/framework/object/object.h"

namespace Pilot
{
    MeshComponent::MeshComponent(const MeshComponentRes& mesh_res, GObject* parent_object) :
        Component(parent_object), m_mesh_res(mesh_res)
    {
        AssetManager& asset_manager = AssetManager::getInstance();

        m_raw_meshes.resize(mesh_res.m_sub_meshes.size());

        size_t raw_mesh_count = 0;
        for (const SubMeshRes& sub_mesh : mesh_res.m_sub_meshes)
        {
            GameObjectComponentDesc& meshComponent = m_raw_meshes[raw_mesh_count];
            meshComponent.mesh_desc.mesh_file = asset_manager.getFullPath(sub_mesh.m_obj_file_ref).generic_string();

            meshComponent.material_desc.with_texture = sub_mesh.m_material.empty() == false;

            if (meshComponent.material_desc.with_texture)
            {
                MaterialRes material_res;
                asset_manager.loadAsset(sub_mesh.m_material, material_res);

                meshComponent.material_desc.baseColorTextureFile =
                    asset_manager.getFullPath(material_res.m_base_colour_texture_file).generic_string();
                meshComponent.material_desc.metallicRoughnessTextureFile =
                    asset_manager.getFullPath(material_res.m_metallic_roughness_texture_file).generic_string();
                meshComponent.material_desc.normalTextureFile =
                    asset_manager.getFullPath(material_res.m_normal_texture_file).generic_string();
                meshComponent.material_desc.occlusionTextureFile =
                    asset_manager.getFullPath(material_res.m_occlusion_texture_file).generic_string();
                meshComponent.material_desc.emissiveTextureFile =
                    asset_manager.getFullPath(material_res.m_emissive_texture_file).generic_string();
            }

            auto object_space_transform                   = sub_mesh.m_transform.getMatrix();
            meshComponent.transform_desc.transform_matrix = object_space_transform;

            ++raw_mesh_count;
        }
    }

    void MeshComponent::tick(float delta_time)
    {
        const TransformComponent* transform_component = m_parent_object->tryGetComponentConst(TransformComponent);
        const AnimationComponent* animation_component = m_parent_object->tryGetComponentConst(AnimationComponent);

        std::vector<GameObjectComponentDesc> mesh_components;
        Pilot::SkeletonAnimationResult       animResult;
        animResult.transforms.push_back({Matrix4x4::IDENTITY});
        if (animation_component != nullptr)
        {
            for (auto& node : animation_component->getResult().node)
            {
                Pilot::SkeletonAnimationResultTransform tmp {Matrix4x4(node.transform)};
                animResult.transforms.push_back(tmp);
            }
        }
        for (GameObjectComponentDesc& mesh_component : m_raw_meshes)
        {
            if (animation_component)
            {
                mesh_component.with_animation                              = true;
                mesh_component.skeleton_animation_result                   = animResult;
                mesh_component.skeleton_binding_desc.skeleton_binding_file = mesh_component.mesh_desc.mesh_file;
            }
            Matrix4x4 object_transform_matrix = mesh_component.transform_desc.transform_matrix;

            mesh_component.transform_desc.transform_matrix = transform_component->getMatrix() * object_transform_matrix;
            mesh_components.push_back(mesh_component);

            mesh_component.transform_desc.transform_matrix = object_transform_matrix;
        }

        SceneManager::getInstance().addSceneObject(GameObjectDesc {m_parent_object->getID(), mesh_components});
    }
} // namespace Pilot