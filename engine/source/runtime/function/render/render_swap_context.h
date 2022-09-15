#pragma once

#include "runtime/function/particle/emitter_id_allocator.h"
#include "runtime/function/particle/particle_desc.h"
#include "runtime/function/render/render_camera.h"
#include "runtime/function/render/render_object.h"

#include "runtime/resource/res_type/global/global_particle.h"
#include "runtime/resource/res_type/global/global_rendering.h"

#include <cstdint>
#include <deque>
#include <optional>
#include <string>

namespace Piccolo
{
    struct LevelIBLResourceDesc
    {
        SkyBoxIrradianceMap m_skybox_irradiance_map;
        SkyBoxSpecularMap   m_skybox_specular_map;
        std::string         m_brdf_map;
    };

    struct LevelColorGradingResourceDesc
    {
        std::string m_color_grading_map;
    };

    struct LevelResourceDesc
    {
        LevelIBLResourceDesc          m_ibl_resource_desc;
        LevelColorGradingResourceDesc m_color_grading_resource_desc;
    };

    struct CameraSwapData
    {
        std::optional<float>            m_fov_x;
        std::optional<RenderCameraType> m_camera_type;
        std::optional<Matrix4x4>        m_view_matrix;
    };

    struct GameObjectResourceDesc
    {
        std::deque<GameObjectDesc> m_game_object_descs;

        void add(GameObjectDesc& desc);
        void pop();

        bool isEmpty() const;

        GameObjectDesc& getNextProcessObject();
    };

    struct ParticleSubmitRequest
    {
        std::vector<ParticleEmitterDesc> m_emitter_descs;

        void add(ParticleEmitterDesc& desc);

        unsigned int getEmitterCount() const;

        const ParticleEmitterDesc& getEmitterDesc(unsigned int index);
    };

    struct EmitterTickRequest
    {
        std::vector<ParticleEmitterID> m_emitter_indices;
    };

    struct EmitterTransformRequest
    {
        std::vector<ParticleEmitterTransformDesc> m_transform_descs;

        void add(ParticleEmitterTransformDesc& desc);

        void clear();

        unsigned int getEmitterCount() const;

        const ParticleEmitterTransformDesc& getNextEmitterTransformDesc(unsigned int index);
    };

    struct RenderSwapData
    {
        std::optional<LevelResourceDesc>       m_level_resource_desc;
        std::optional<GameObjectResourceDesc>  m_game_object_resource_desc;
        std::optional<GameObjectResourceDesc>  m_game_object_to_delete;
        std::optional<CameraSwapData>          m_camera_swap_data;
        std::optional<ParticleSubmitRequest>   m_particle_submit_request;
        std::optional<EmitterTickRequest>      m_emitter_tick_request;
        std::optional<EmitterTransformRequest> m_emitter_transform_request;

        void addDirtyGameObject(GameObjectDesc&& desc);
        void addDeleteGameObject(GameObjectDesc&& desc);

        void addNewParticleEmitter(ParticleEmitterDesc& desc);
        void addTickParticleEmitter(ParticleEmitterID id);
        void updateParticleTransform(ParticleEmitterTransformDesc& desc);
    };

    enum SwapDataType : uint8_t
    {
        LogicSwapDataType = 0,
        RenderSwapDataType,
        SwapDataTypeCount
    };

    class RenderSwapContext
    {
    public:
        RenderSwapData& getLogicSwapData();
        RenderSwapData& getRenderSwapData();
        void            swapLogicRenderData();
        void            resetLevelRsourceSwapData();
        void            resetGameObjectResourceSwapData();
        void            resetGameObjectToDelete();
        void            resetCameraSwapData();
        void            resetPartilceBatchSwapData();
        void            resetEmitterTickSwapData();
        void            resetEmitterTransformSwapData();

    private:
        uint8_t        m_logic_swap_data_index {LogicSwapDataType};
        uint8_t        m_render_swap_data_index {RenderSwapDataType};
        RenderSwapData m_swap_data[SwapDataTypeCount];

        bool isReadyToSwap() const;
        void swap();
    };
} // namespace Piccolo
