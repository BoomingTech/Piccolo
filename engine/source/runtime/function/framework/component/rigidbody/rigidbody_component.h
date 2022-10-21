/*
 * @Author: liangliang.ma liangliang.ma@boomingtech.com
 * @Date: 2022-10-21 18:24:39
 * @LastEditors: liangliang.ma liangliang.ma@boomingtech.com
 * @LastEditTime: 2022-10-21 18:39:32
 * @FilePath: \pilot-internal\engine\source\runtime\function\framework\component\rigidbody\rigidbody_component.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

#include "runtime/resource/res_type/components/rigid_body.h"

#include "runtime/function/framework/component/component.h"

namespace Piccolo
{
    REFLECTION_TYPE(RigidBodyComponent)
    CLASS(RigidBodyComponent : public Component, WhiteListFields)
    {
        REFLECTION_BODY(RigidBodyComponent)
    public:
        RigidBodyComponent() = default;
        ~RigidBodyComponent() override;

        void postLoadResource(std::weak_ptr<GObject> parent_object) override;

        void tick(float delta_time) override {}
        void updateGlobalTransform(const Transform& transform, bool is_scale_dirty);
        void getShapeBoundingBoxes(std::vector<AxisAlignedBox> & out_boudning_boxes) const;

    protected:
        void createRigidBody(const Transform& global_transform);
        void removeRigidBody();

        META(Enable)
        RigidBodyComponentRes m_rigidbody_res;

        uint32_t m_rigidbody_id {0xffffffff};
    };
} // namespace Piccolo
