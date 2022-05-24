#pragma once

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
        SkyBoxIrradianceMap skybox_irradiance_map;
        SkyBoxSpecularMap   skybox_specular_map;
        std::string         brdf_map;
    };

    struct LevelColorGradingResourceDesc
    {
        std::string color_grading_map;
    };

    struct LevelResourceDesc
    {
        LevelIBLResourceDesc          ibl_resource_desc;
        LevelColorGradingResourceDesc color_grading_resource_desc;
    };

    struct GameObjectResourceDesc
    {
        std::deque<GameObjectDesc> game_object_descs;
        void                       add(GameObjectDesc desc);
        bool                       isEmpty() const;
        GameObjectDesc             getNextProcessObject();
        void                       popProcessObject();
    };

    struct RenderSwapData
    {
        std::optional<LevelResourceDesc>      level_resource_desc;
        std::optional<GameObjectResourceDesc> game_object_resource_desc;
        std::optional<GameObjectResourceDesc> game_object_to_delete;

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

    private:
        uint8_t        m_logic_swap_data_index {LogicSwapDataType};
        uint8_t        m_render_swap_data_index {RenderSwapDataType};
        RenderSwapData m_swap_data[SwapDataTypeCount];

        bool isReadyToSwap() const;
        void swap();
    };
} // namespace Pilot
