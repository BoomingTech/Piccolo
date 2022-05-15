#pragma once

#include "runtime/function/framework/component/component.h"

#include "runtime/resource/res_type/components/mesh.h"

#include <vector>

// TODO PGameObjectComponentDesc should be converted to StaticMeshData
#include "runtime/function/scene/scene_object.h"

namespace Pilot
{
    REFLECTION_TYPE(MeshComponent)
    CLASS(MeshComponent : public Component, WhiteListFields)
    {
        REFLECTION_BODY(MeshComponent)
    public:
        MeshComponent() {};

        void postLoadResource(std::weak_ptr<GObject> parent_object) override;

        const std::vector<GameObjectComponentDesc>& getRawMeshes() const { return m_raw_meshes; }

        void tick(float delta_time) override;

    private:
        META(Enable)
        MeshComponentRes m_mesh_res;

        std::vector<GameObjectComponentDesc> m_raw_meshes;
    };
} // namespace Pilot
