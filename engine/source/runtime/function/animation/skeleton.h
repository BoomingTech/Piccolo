#pragma once

#include "runtime/resource/res_type/components/animation.h"

#include "runtime/function/animation/node.h"
#include "runtime/function/animation/pose.h"

namespace Pilot
{
    class SkeletonData;
    class BlendStateWithClipData;

    class Skeleton
    {
    private:
        bool  m_is_flat {false};
        int   m_bone_count {0};
        Bone* m_bones {nullptr};

    public:
        ~Skeleton();

        void            buildSkeleton(const SkeletonData& skeleton_definition);
        void            applyPose(const AnimationPose& pose);
        void            applyAdditivePose(const AnimationPose& pose);
        void            extractPose(AnimationPose& pose);
        void            applyAnimation(const BlendStateWithClipData& blend_state);
        AnimationResult outputAnimationResult();
        void            resetSkeleton();
    };
} // namespace Pilot
