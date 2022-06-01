#pragma once
#include <functional>
#include <json11.hpp>
#include <vector>
namespace Pilot
{
    class AnimationFSM
    {
        // std::vector<AnimationFSMNode> m_nodes;
        enum class States
        {
            _idle,
            _walk_start,
            _walk_run,
            _walk_stop,
            _jump_start_from_idle,
            _jump_loop_from_idle,
            _jump_end_from_idle,
            _jump_start_from_walk_run,
            _jump_loop_from_walk_run,
            _jump_end_from_walk_run,
            _count
        };
        
        States m_state {States::_idle};

    public:
        AnimationFSM();
        bool        update(const json11::Json::object& signals);
        std::string getCurrentClipBaseName() const;
    };
} // namespace Pilot