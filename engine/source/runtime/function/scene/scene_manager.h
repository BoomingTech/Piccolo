#pragma once

#include "runtime/core/base/public_singleton.h"
#include "runtime/core/math/math_headers.h"

#include "runtime/resource/res_type/global/global_rendering.h"

#include "runtime/function/render/include/render/framebuffer.h"
#include "runtime/function/scene/scene_allocator.h"
#include "runtime/function/scene/scene_object.h"

#include <deque>

namespace Pilot
{
    class SceneManager final : public PublicSingleton<SceneManager>
    {
        friend class PublicSingleton<SceneManager>;

    protected:
        SceneManager()                             = default;
        std::shared_ptr<Scene>             m_scene = std::make_shared<Scene>();
        std::unordered_map<size_t, size_t> m_mesh_id_gobejct_id_map;

    public:
        void initialize();
        int  clear() { return 0; }

        int tick(FrameBuffer* buffer);

        const SceneMemory* memoryFromHandle(SceneResourceHandle handle);
        const SceneImage*  imageFromHandle(TextureHandle handle);

        std::shared_ptr<Scene> getCurrentScene() const { return m_scene; }

        void        addSceneObject(const GameObjectDesc& go_desc);
        void        syncSceneObjects();
        void        addReleaseMeshHandle(const MeshHandle& mesh_handle);
        void        addReleaseMaterialHandle(const PMaterialHandle& material_handle);
        void        addReleaseSkeletonBindingHandle(const SkeletonBindingBufferHandle& handle);
        ComponentId getSelectedComponentByNodeId(size_t node_id);
        bool getTransformDescByComponentId(const ComponentId& component_id, GameObjectTransformDesc& out_transform);
        void setMainViewMatrix(const Matrix4x4& view_matrix, PCurrentCameraType type = PCurrentCameraType::Editor);
        void setFOV(float fovx);
        void setWindowSize(const Vector2& size) { m_window_size = size; }
        const Vector2& getWindowSize() const { return m_window_size; }
        const Vector2  getFOV() const;

        // for EditorUI
        void         setAxisMesh(std::vector<RenderMesh>& axis_meshes);
        const size_t getGObjectIDByMeshID(size_t mesh_id) const;

    private:
        std::vector<GameObjectDesc> m_go_descs;

        std::unordered_map<std::string, MeshHandle> m_mesh_handle_map;
        std::unordered_map<MeshHandle, std::string> m_handle_mesh_map;

        std::unordered_map<std::string, VertexBufferHandle> m_vertex_handle_map;
        std::unordered_map<VertexBufferHandle, std::string> m_handle_vertex_map;

        std::unordered_map<std::string, IndexBufferHandle> m_index_handle_map;
        std::unordered_map<IndexBufferHandle, std::string> m_handle_index_map;

        std::unordered_map<std::string, TextureHandle> m_image_handle_map;
        std::unordered_map<TextureHandle, std::string> m_handle_image_map;

        std::unordered_map<std::string, SkeletonMeshBinding>         m_skeleton_binding_handle_map;
        std::unordered_map<SkeletonBindingBufferHandle, std::string> m_handle_skeleton_binding_map;

        static const size_t s_max_mesh_handle_count                 = 1000;
        static const size_t s_release_mesh_handle_count             = 100;
        static const size_t s_max_material_handle_count             = 1000;
        static const size_t s_release_material_handle_count         = 100;
        static const size_t s_max_skeleton_binding_handle_count     = 1000;
        static const size_t s_release_skeleton_binding_handle_count = 100;

        std::deque<MeshHandle>                  m_mesh_handles_to_release;
        std::deque<PMaterialHandle>             m_material_handles_to_release;
        std::deque<SkeletonBindingBufferHandle> m_skeleton_binding_handles_to_release;
        std::mutex                              m_release_handle_mutex;
        std::mutex                              m_pick_object_mutex;

        std::unordered_map<ComponentId, GameObjectTransformDesc> m_component_transform_map;

        SceneAllocator<RenderMesh>  m_mesh_allocator;
        SceneAllocator<Material>    m_material_allocator;
        SceneAllocator<ComponentId> m_instance_id_allocator;

        Vector2 m_window_size;

    private:
        MeshHandle    getOrCreateMeshHandle(const std::string& mesh_file);
        TextureHandle getOrCreateImageHandle(const std::string& image_file, bool srgb = false);
        void          getOrCreateSkeletonBindingHandle(const std::string&           skeleton_binding_file,
                                                       VertexBufferHandle&          vertex_handle,
                                                       IndexBufferHandle&           index_handle,
                                                       AxisAlignedBox&              bounding_box,
                                                       SkeletonBindingBufferHandle& skeleton_binding_handle);

        void releaseMeshHandles();
        void releaseMaterialHandles();
        void releaseSkeletonBindingHandles();
        void releaseImageHandle(TextureHandle& image_handle);

        void setSceneOnce(const GlobalRenderingRes& global_res);
    };
} // namespace Pilot
