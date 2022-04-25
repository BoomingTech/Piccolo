#pragma once

#include "runtime/core/math/math_headers.h"

#include "render/light.h"
#include "render/material.h"
#include "render/render_camera.h"
#include "render/render_mesh.h"
#include "render/resource_handle.h"

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

namespace Pilot
{
    inline static std::atomic<size_t> CurrentMaxGuid = 0;

    enum class PILOT_PIXEL_FORMAT : size_t
    {
        PILOT_PIXEL_FORMAT_R8G8B8_UNORM = 1,
        PILOT_PIXEL_FORMAT_R8G8B8_SRGB,
        PILOT_PIXEL_FORMAT_R8G8B8A8_UNORM,
        PILOT_PIXEL_FORMAT_R8G8B8A8_SRGB,
        PILOT_PIXEL_FORMAT_R32G32_FLOAT,
        PILOT_PIXEL_FORMAT_R32G32B32_FLOAT,
        PILOT_PIXEL_FORMAT_R32G32B32A32_FLOAT
    };

    enum class PILOT_IMAGE_TYPE : size_t
    {
        PILOT_IMAGE_TYPE_2D
    };

    class SceneMemory
    {
    public:
        size_t m_size;
        char*  m_data;
    };

    class SceneImage
    {
    public:
        size_t m_width;
        size_t m_height;
        size_t m_depth;

        size_t m_mip_levels;
        size_t m_array_layers;

        PILOT_PIXEL_FORMAT m_format;
        PILOT_IMAGE_TYPE   m_type;
        void*              m_pixels;
    };

    class SceneBuffers
    {
    public:
        static void destroy(VertexBufferHandle handle)
        {
            auto scene_memory = (SceneMemory*)handle.handle;
            if (scene_memory)
            {
                if (scene_memory->m_data)
                {
                    delete[] scene_memory->m_data;
                    scene_memory->m_data = nullptr;
                }
                delete scene_memory;
            }
        }
        static bool isValid(VertexBufferHandle) { return false; }
        static void destroy(SkeletonBindingBufferHandle handle)
        {
            auto scene_memory = (SceneMemory*)handle.handle;
            if (scene_memory)
            {
                if (scene_memory->m_data)
                {
                    delete[] scene_memory->m_data;
                    scene_memory->m_data = nullptr;
                }
                delete scene_memory;
            }
        }
        static bool isValid(SkeletonBindingBufferHandle) { return false; }
        static void destroy(IndexBufferHandle handle)
        {
            auto scene_memory = (SceneMemory*)handle.handle;
            if (scene_memory)
            {
                if (scene_memory->m_data)
                {
                    delete[] scene_memory->m_data;
                    scene_memory->m_data = nullptr;
                }
                delete scene_memory;
            }
        }
        static bool isValid(IndexBufferHandle) { return false; }
        static void destroy(TextureHandle handle)
        {
            auto scene_image = (SceneImage*)handle.handle;
            if (scene_image)
            {
                if (scene_image->m_pixels)
                {
                    free(scene_image->m_pixels);
                    scene_image->m_pixels = nullptr;
                }
                delete scene_image;
            }
        }
        static bool               isValid(TextureHandle) { return false; }
        static void               destroy(DynamicVertexBufferHandle handle) {}
        static bool               isValid(DynamicVertexBufferHandle) { return false; }
        static const SceneMemory* alloc(size_t size) { return new SceneMemory {size, new char[size]}; }

        static SceneImage*        allocTexture() { return new SceneImage(); }
        static VertexBufferHandle createVertexBuffer(const SceneMemory* memory, size_t) { return {(size_t)memory}; }

        static IndexBufferHandle           createIndexBuffer(const SceneMemory* memory) { return {(size_t)memory}; }
        static SkeletonBindingBufferHandle createSkeletonBindingBuffer(const SceneMemory* memory)
        {
            return {(size_t)memory};
        }
        static TextureHandle      createTexture(const SceneImage* texture) { return {(size_t)texture}; }
        static const SceneMemory* memoryFromHandle(SceneResourceHandle handle)
        {
            return (const SceneMemory*)handle.handle;
        }
        static const SceneImage* imageFromHandle(TextureHandle handle) { return (const SceneImage*)handle.handle; }
    };

    enum class PRenderPath : int
    {
        Forward,
        Deferred,
        Clustered
    };

    class Scene
    {
    public:
        bool                     m_loaded = false;
        Vector3                  m_minBounds {0, 0, 0};
        Vector3                  m_maxBounds {0, 0, 0};
        Vector3                  m_center {0, 0, 0};
        float                    m_diagonal {0};
        std::shared_ptr<PCamera> m_camera;

        std::vector<std::shared_ptr<PParticleBillbord>> m_particlebillboards;

        // these are not populated by load
        // global textures for IBL
        TextureHandle m_brdfLUT_texture_handle;
        TextureHandle m_irradiance_texture_handle[6];
        TextureHandle m_specular_texture_handle[6];
        
        // these are not populated by load
        // global textures for color grading
        TextureHandle m_color_grading_LUT_texture_handle;

        Vector3           m_sky_color;
        PAmbientLight     m_ambient_light;
        PDirectionalLight m_directional_light;
        PPointLightList   m_pointLights;

        void                           lock() { m_scene_mutex.lock(); }
        void                           unlock() { m_scene_mutex.unlock(); }
        const std::vector<RenderMesh>& getMeshes() const { return m_meshes; }
        std::vector<RenderMesh>&       getMeshes() { return m_meshes; }
        const std::vector<Material>&   getMaterials() const { return m_materials; }
        std::vector<Material>&         getMaterials() { return m_materials; }
        const std::vector<RenderMesh>& getAxisMeshes() const { return m_axis; }
        std::vector<RenderMesh>&       getAxisMeshs() { return m_axis; }
        void                           clear()
        {
            clearMeshes();
            clearMaterials();
        }
        void addMesh(const RenderMesh& mesh) { m_meshes.push_back(mesh); }
        void addMaterial(const Material& material) { m_materials.push_back(material); }
        void clearMeshes() { m_meshes.clear(); }
        void clearMaterials() { m_materials.clear(); }
        void clearAxis() { m_axis.clear(); }
        void setAxisMesh(const std::vector<RenderMesh>& axis_meshes) { m_axis = axis_meshes; }

    private:
        std::mutex m_scene_mutex;

        std::vector<RenderMesh> m_meshes;
        std::vector<Material>   m_materials;
        std::vector<RenderMesh> m_axis;
    };

    class UIState
    {
    public:
        bool        m_writeLog {true};
        PRenderPath m_renderPath {PRenderPath::Forward};

        bool m_fullscreen {false};
        bool m_showUI {true};
        bool m_showConfigWindow {true};
        bool m_showLog {false};

        bool m_showStatsOverlay {false};
        struct
        {
            bool fps;
            bool frameTime;
            bool profiler;
            bool gpuMemory;
        } m_overlays {true, true, true, true};

        std::shared_ptr<PCamera> m_editor_camera;
    };
    struct SceneReleaseHandles
    {
        std::vector<MeshHandle>                  mesh_handles;
        std::vector<PMaterialHandle>             material_handles;
        std::vector<SkeletonBindingBufferHandle> skeleton_binding_handles;
    };
    class FrameBuffer
    {
    public:
        FrameBuffer() : m_uistate(std::make_unique<UIState>()) {}
        size_t logicalFrameIndex {0};

        PRenderPath              m_renderpath {PRenderPath::Clustered};
        std::shared_ptr<Scene>   m_scene;
        std::unique_ptr<UIState> m_uistate;
    };

} // namespace Pilot

template<>
struct std::hash<Pilot::SceneResourceHandle>
{
    size_t operator()(const Pilot::SceneResourceHandle& rhs) const noexcept { return rhs.getHashValue(); }
};
template<>
struct std::hash<Pilot::VertexBufferHandle>
{
    size_t operator()(const Pilot::VertexBufferHandle& rhs) const noexcept { return rhs.getHashValue(); }
};
template<>
struct std::hash<Pilot::IndexBufferHandle>
{
    size_t operator()(const Pilot::IndexBufferHandle& rhs) const noexcept { return rhs.getHashValue(); }
};
template<>
struct std::hash<Pilot::TextureHandle>
{
    size_t operator()(const Pilot::TextureHandle& rhs) const noexcept { return rhs.getHashValue(); }
};
template<>
struct std::hash<Pilot::DynamicVertexBufferHandle>
{
    size_t operator()(const Pilot::DynamicVertexBufferHandle& rhs) const noexcept { return rhs.getHashValue(); }
};
template<>
struct std::hash<Pilot::SkeletonBindingBufferHandle>
{
    size_t operator()(const Pilot::SkeletonBindingBufferHandle& rhs) const noexcept { return rhs.getHashValue(); }
};
template<>
struct std::hash<Pilot::MeshHandle>
{
    size_t operator()(const Pilot::MeshHandle& rhs) const noexcept { return rhs.getHashValue(); }
};
template<>
struct std::hash<Pilot::SkeletonMeshBinding>
{
    size_t operator()(const Pilot::SkeletonMeshBinding& rhs) const noexcept { return rhs.getHashValue(); }
};
template<>
struct std::hash<Pilot::PMaterialHandle>
{
    size_t operator()(const Pilot::PMaterialHandle& rhs) const noexcept { return rhs.getHashValue(); }
};
template<>
struct std::hash<Pilot::RenderMesh>
{
    size_t operator()(const Pilot::RenderMesh& rhs) const noexcept { return rhs.getHashValue(); }
};
template<>
struct std::hash<Pilot::Material>
{
    size_t operator()(const Pilot::Material& rhs) const noexcept { return rhs.getHashValue(); }
};
