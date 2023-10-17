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
    protected:
        AnimationResult m_result;
	};
}
