#pragma once

#include "runtime/function/render/render_camera.h"
#include "runtime/function/render/render_object.h"
#include "runtime/resource/res_type/global/global_rendering.h"

#include <cstdint>
#include <deque>
#include <optional>
#include <string>

namespace Pilot
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

        void                       add(GameObjectDesc desc);
        bool                       isEmpty() const;
        GameObjectDesc             getNextProcessObject();
        void                       popProcessObject();
    };

    struct RenderSwapData
    {
        std::optional<LevelResourceDesc>      m_level_resource_desc;
        std::optional<GameObjectResourceDesc> m_game_object_resource_desc;
        std::optional<GameObjectResourceDesc> m_game_object_to_delete;
        std::optional<CameraSwapData>         m_camera_swap_data;

        void addDirtyGameObject(GameObjectDesc desc);
        void addDeleteGameObject(GameObjectDesc desc);
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

    private:
        uint8_t        m_logic_swap_data_index {LogicSwapDataType};
        uint8_t        m_render_swap_data_index {RenderSwapDataType};
        RenderSwapData m_swap_data[SwapDataTypeCount];

        bool isReadyToSwap() const;
        void swap();
    };
} // namespace Pilot
