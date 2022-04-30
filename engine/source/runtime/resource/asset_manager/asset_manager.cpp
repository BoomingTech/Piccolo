#include "runtime/resource/asset_manager/asset_manager.h"

#include "runtime/core/math/transform.h"

#include "runtime/resource/config_manager/config_manager.h"
#include "runtime/resource/res_type/components/animation.h"
#include "runtime/resource/res_type/components/mesh.h"

#include "runtime/function/framework/component/animation/animation_component.h"
#include "runtime/function/framework/component/camera/camera_component.h"
#include "runtime/function/framework/component/mesh/mesh_component.h"
#include "runtime/function/framework/component/motor/motor_component.h"
#include "runtime/function/framework/component/rigidbody/rigidbody_component.h"
#include "runtime/function/framework/component/transform/transform_component.h"

namespace Pilot
{
    void AssetManager::initialize()
    {
        REGISTER_COMPONENT(AnimationComponent, AnimationComponentRes, false);
        REGISTER_COMPONENT(MeshComponent, MeshComponentRes, true);
        REGISTER_COMPONENT(RigidBodyComponent, RigidBodyComponentRes, false);
        REGISTER_COMPONENT(CameraComponent, CameraComponentRes, false);
        REGISTER_COMPONENT(MotorComponent, MotorComponentRes, false);
    }

    std::filesystem::path AssetManager::getFullPath(const std::string& relative_path) const
    {
        return ConfigManager::getInstance().getRootFolder() / relative_path;
    }
} // namespace Pilot