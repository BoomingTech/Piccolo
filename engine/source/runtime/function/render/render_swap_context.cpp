#include "runtime/function/render/render_swap_context.h"

#include <utility>

namespace Piccolo
{
    void GameObjectResourceDesc::add(GameObjectDesc& desc) { m_game_object_descs.push_back(desc); }

    bool GameObjectResourceDesc::isEmpty() const { return m_game_object_descs.empty(); }

    GameObjectDesc& GameObjectResourceDesc::getNextProcessObject() { return m_game_object_descs.front(); }

    void GameObjectResourceDesc::pop() { m_game_object_descs.pop_front(); }

    void ParticleSubmitRequest::add(ParticleEmitterDesc& desc) { m_emitter_descs.push_back(desc); }

    unsigned int ParticleSubmitRequest::getEmitterCount() const { return m_emitter_descs.size(); }

    const ParticleEmitterDesc& ParticleSubmitRequest::getEmitterDesc(unsigned int index)
    {
        return m_emitter_descs[index];
    }

    void EmitterTransformRequest::add(ParticleEmitterTransformDesc& desc) { m_transform_descs.push_back(desc); }

    unsigned int EmitterTransformRequest::getEmitterCount() const { return m_transform_descs.size(); }

    const ParticleEmitterTransformDesc& EmitterTransformRequest::getNextEmitterTransformDesc(unsigned int index)
    {
        return m_transform_descs[index];
    }

    RenderSwapData& RenderSwapContext::getLogicSwapData() { return m_swap_data[m_logic_swap_data_index]; }

    RenderSwapData& RenderSwapContext::getRenderSwapData() { return m_swap_data[m_render_swap_data_index]; }

    void RenderSwapContext::swapLogicRenderData()
    {
        if (isReadyToSwap())
        {
            swap();
        }
    }

    bool RenderSwapContext::isReadyToSwap() const
    {
        return !(m_swap_data[m_render_swap_data_index].m_level_resource_desc.has_value() ||
                 m_swap_data[m_render_swap_data_index].m_game_object_resource_desc.has_value() ||
                 m_swap_data[m_render_swap_data_index].m_game_object_to_delete.has_value() ||
                 m_swap_data[m_render_swap_data_index].m_camera_swap_data.has_value() ||
                 m_swap_data[m_render_swap_data_index].m_particle_submit_request.has_value() ||
                 m_swap_data[m_render_swap_data_index].m_emitter_tick_request.has_value() ||
                 m_swap_data[m_render_swap_data_index].m_emitter_transform_request.has_value());
    }

    void RenderSwapContext::resetLevelRsourceSwapData()
    {
        m_swap_data[m_render_swap_data_index].m_level_resource_desc.reset();
    }

    void RenderSwapContext::resetGameObjectResourceSwapData()
    {
        m_swap_data[m_render_swap_data_index].m_game_object_resource_desc.reset();
    }

    void RenderSwapContext::resetGameObjectToDelete()
    {
        m_swap_data[m_render_swap_data_index].m_game_object_to_delete.reset();
    }

    void RenderSwapContext::resetPartilceBatchSwapData()
    {
        m_swap_data[m_render_swap_data_index].m_particle_submit_request.reset();
    }

    void RenderSwapContext::resetCameraSwapData() { m_swap_data[m_render_swap_data_index].m_camera_swap_data.reset(); }

    void RenderSwapContext::resetEmitterTickSwapData()
    {
        m_swap_data[m_render_swap_data_index].m_emitter_tick_request.reset();
    }

    void RenderSwapContext::resetEmitterTransformSwapData()
    {
        m_swap_data[m_render_swap_data_index].m_emitter_transform_request.reset();
    }

    void RenderSwapContext::swap()
    {
        resetLevelRsourceSwapData();
        resetGameObjectResourceSwapData();
        resetGameObjectToDelete();
        resetCameraSwapData();
        resetEmitterTickSwapData();
        resetEmitterTransformSwapData();
        resetPartilceBatchSwapData();
        std::swap(m_logic_swap_data_index, m_render_swap_data_index);
    }

    void RenderSwapData::addDirtyGameObject(GameObjectDesc&& desc)
    {
        if (m_game_object_resource_desc.has_value())
        {
            m_game_object_resource_desc->add(desc);
        }
        else
        {
            GameObjectResourceDesc go_descs;
            go_descs.add(desc);
            m_game_object_resource_desc = go_descs;
        }
    }

    void RenderSwapData::addDeleteGameObject(GameObjectDesc&& desc)
    {
        if (m_game_object_to_delete.has_value())
        {
            m_game_object_to_delete->add(desc);
        }
        else
        {
            GameObjectResourceDesc go_descs;
            go_descs.add(desc);
            m_game_object_to_delete = go_descs;
        }
    }

    void RenderSwapData::addNewParticleEmitter(ParticleEmitterDesc& desc)
    {
        if (m_particle_submit_request.has_value())
        {
            m_particle_submit_request->add(desc);
        }
        else
        {
            ParticleSubmitRequest request;
            request.add(desc);
            m_particle_submit_request = request;
        }
    }

    void RenderSwapData::addTickParticleEmitter(ParticleEmitterID id)
    {
        if (m_emitter_tick_request.has_value())
        {
            m_emitter_tick_request->m_emitter_indices.push_back(id);
        }
        else
        {
            EmitterTickRequest request;
            request.m_emitter_indices.push_back(id);
            m_emitter_tick_request = request;
        }
    }

    void RenderSwapData::updateParticleTransform(ParticleEmitterTransformDesc& desc)
    {
        if (m_emitter_transform_request.has_value())
        {
            m_emitter_transform_request->add(desc);
        }
        else
        {
            EmitterTransformRequest request;
            request.add(desc);
            m_emitter_transform_request = request;
        }
    }
} // namespace Piccolo
