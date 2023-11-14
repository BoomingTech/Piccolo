#pragma once
#include <vector>

#include "function/animation/skeleton.h"
#include "resource/res_type/components/animation.h"

namespace Piccolo
{
    constexpr int32_t INDEX_NONE = -1;
	REFLECTION_TYPE(CAnimInstanceBase)
	CLASS(CAnimInstanceBase, WhiteListFields)
	{
		REFLECTION_BODY(CAnimInstanceBase)
	public:
		CAnimInstanceBase() = default;
		virtual ~CAnimInstanceBase() = default;

		virtual void TickAnimation(float delta_time)
		{
		}


		virtual const AnimationResult& GetResult() { return m_result; }
        virtual Skeleton* GetSkeleton() { return nullptr; }

		const std::vector<Matrix4x4>& GetComponentResult()
		{
			return m_component_space_transform;
		}

		const std::vector<uint32_t>& GetParentInfo() { return m_parent_info; }

		virtual Transform GetRootMotion();
        virtual bool HasRootMotion();

    protected:
        AnimationResult m_result;
        std::vector<Matrix4x4> m_component_space_transform;
        std::vector<uint32_t>   m_parent_info;
	};
}
