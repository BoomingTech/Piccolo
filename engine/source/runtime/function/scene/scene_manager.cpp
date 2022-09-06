#include "runtime/function/scene/scene_manager.h"

#include "runtime/core/base/macro.h"

#include "runtime/function/render/include/render/framebuffer.h"

#include "runtime/resource/asset_manager/asset_manager.h"
#include "runtime/resource/config_manager/config_manager.h"
#include "runtime/resource/res_type/data/mesh_data.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <algorithm>
#include <filesystem>

namespace Pilot
{
    class SceneBuilder
    {
    public:
        static MeshHandle    loadMesh(const std::string& filename, Vector3& min_bound, Vector3& max_bound);
        static TextureHandle loadTexture(const char* file, bool sRGB = false);
        static TextureHandle loadTextureHDR(const char* file, int desired_channels = 4);

    public:
        static void clear(Scene* curscene)
        {
            curscene->lock();
            if (curscene->m_loaded)
            {
                for (RenderMesh& mesh : curscene->getMeshes())
                {
                    SceneBuffers::destroy(mesh.m_vertexBuffer);
                    SceneBuffers::destroy(mesh.m_indexBuffer);
                    mesh.m_vertexBuffer = PILOT_INVALID_HANDLE;
                    mesh.m_indexBuffer  = PILOT_INVALID_HANDLE;
                }

                for (Material& mat : curscene->getMaterials())
                {
                    if (SceneBuffers::isValid(mat.m_baseColorTexture))
                    {
                        SceneBuffers::destroy(mat.m_baseColorTexture);
                        mat.m_baseColorTexture = PILOT_INVALID_HANDLE;
                    }
                    if (SceneBuffers::isValid(mat.m_metallicRoughnessTexture))
                    {
                        SceneBuffers::destroy(mat.m_metallicRoughnessTexture);
                        mat.m_metallicRoughnessTexture = PILOT_INVALID_HANDLE;
                    }
                    if (SceneBuffers::isValid(mat.m_normalTexture))
                    {
                        SceneBuffers::destroy(mat.m_normalTexture);
                        mat.m_normalTexture = PILOT_INVALID_HANDLE;
                    }
                    if (SceneBuffers::isValid(mat.m_occlusionTexture))
                    {
                        SceneBuffers::destroy(mat.m_occlusionTexture);
                        mat.m_occlusionTexture = PILOT_INVALID_HANDLE;
                    }
                    if (SceneBuffers::isValid(mat.m_emissiveTexture))
                    {
                        SceneBuffers::destroy(mat.m_emissiveTexture);
                        mat.m_emissiveTexture = PILOT_INVALID_HANDLE;
                    }
                }
                curscene->clear();
                curscene->m_pointLights.shutdown();
                curscene->m_pointLights.m_lights.clear();
            }
            curscene->m_minBounds = curscene->m_maxBounds = {0.0f, 0.0f, 0.0f};
            curscene->m_center                            = {0.0f, 0.0f, 0.0f};
            curscene->m_diagonal                          = 0.0f;
            curscene->m_camera                            = std::make_shared<PCamera>();
            curscene->m_loaded                            = false;
            curscene->unlock();
        }
    };

    MeshHandle SceneBuilder::loadMesh(const std::string& filename, Vector3& min_bound, Vector3& max_bound)
    {
        MeshHandle               ret;
        tinyobj::ObjReader       reader;
        tinyobj::ObjReaderConfig reader_config;
        reader_config.vertex_color = false;
        if (!reader.ParseFromFile(filename, reader_config))
        {
            if (!reader.Error().empty())
            {
                LOG_ERROR("loadMesh {} failed, error: {}", filename, reader.Error());
            }
            assert(0);
        }

        if (!reader.Warning().empty())
        {
            LOG_WARN("loadMesh {} warning, warning: {}", filename, reader.Warning());
        }

        auto& attrib = reader.GetAttrib();
        auto& shapes = reader.GetShapes();

        std::vector<Mesh_PosNormalTangentTex0Vertex> mesh_vertices;

        for (size_t s = 0; s < shapes.size(); s++)
        {
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
            {
                size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

                bool with_normal   = true;
                bool with_texcoord = true;

                Vector3 vertex[3];
                Vector3 normal[3];
                Vector2 uv[3];

                // only deals with triangle faces
                if (fv != 3)
                {
                    continue;
                }

                // expanding vertex data is not efficient and is for testing purposes only
                for (size_t v = 0; v < fv; v++)
                {
                    auto idx = shapes[s].mesh.indices[index_offset + v];
                    auto vx  = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                    auto vy  = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                    auto vz  = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                    vertex[v].x = static_cast<float>(vx);
                    vertex[v].y = static_cast<float>(vy);
                    vertex[v].z = static_cast<float>(vz);

                    min_bound.makeFloor(vertex[v]);
                    max_bound.makeCeil(vertex[v]);

                    ret.m_bounding_box.merge(Vector3(vertex[v].x, vertex[v].y, vertex[v].z));

                    if (idx.normal_index >= 0)
                    {
                        auto nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                        auto ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                        auto nz = attrib.normals[3 * size_t(idx.normal_index) + 2];

                        normal[v].x = static_cast<float>(nx);
                        normal[v].y = static_cast<float>(ny);
                        normal[v].z = static_cast<float>(nz);
                    }
                    else
                    {
                        with_normal = false;
                    }

                    if (idx.texcoord_index >= 0)
                    {
                        auto tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                        auto ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];

                        uv[v].x = static_cast<float>(tx);
                        uv[v].y = static_cast<float>(ty);
                    }
                    else
                    {
                        with_texcoord = false;
                    }
                }
                index_offset += fv;

                if (!with_normal)
                {
                    Vector3 v0 = vertex[1] - vertex[0];
                    Vector3 v1 = vertex[2] - vertex[1];
                    normal[0]  = v0.crossProduct(v1).normalisedCopy();
                    normal[1]  = normal[0];
                    normal[2]  = normal[0];
                }

                if (!with_texcoord)
                {
                    uv[0] = Vector2(0.5f, 0.5f);
                    uv[1] = Vector2(0.5f, 0.5f);
                    uv[2] = Vector2(0.5f, 0.5f);
                }

                Vector3 tangent {1, 0, 0};
                {
                    Vector3 edge1    = vertex[1] - vertex[0];
                    Vector3 edge2    = vertex[2] - vertex[1];
                    Vector2 deltaUV1 = uv[1] - uv[0];
                    Vector2 deltaUV2 = uv[2] - uv[1];

                    auto divide = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
                    if (divide >= 0.0f && divide < 0.000001f)
                        divide = 0.000001f;
                    else if (divide < 0.0f && divide > -0.000001f)
                        divide = -0.000001f;

                    float df  = 1.0f / divide;
                    tangent.x = df * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                    tangent.y = df * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                    tangent.z = df * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
                    tangent   = (tangent).normalisedCopy();
                }

                for (size_t i = 0; i < 3; i++)
                {
                    Mesh_PosNormalTangentTex0Vertex mesh_vert {};

                    mesh_vert.x = vertex[i].x;
                    mesh_vert.y = vertex[i].y;
                    mesh_vert.z = vertex[i].z;

                    mesh_vert.nx = normal[i].x;
                    mesh_vert.ny = normal[i].y;
                    mesh_vert.nz = normal[i].z;

                    mesh_vert.u = uv[i].x;
                    mesh_vert.v = uv[i].y;

                    mesh_vert.tx = tangent.x;
                    mesh_vert.ty = tangent.y;
                    mesh_vert.tz = tangent.z;

                    mesh_vertices.push_back(mesh_vert);
                }
            }
        }

        uint32_t stride    = sizeof(Mesh_PosNormalTangentTex0Vertex);
        auto     vertexMem = SceneBuffers::alloc(mesh_vertices.size() * stride);

        auto      iMem    = SceneBuffers::alloc(mesh_vertices.size() * sizeof(uint16_t));
        uint16_t* indices = (uint16_t*)iMem->m_data;

        assert(mesh_vertices.size() <= std::numeric_limits<uint16_t>::max()); // take care of the index range, should be
                                                                              // consistent with the index range used by
                                                                              // vulkan

        for (size_t i = 0; i < mesh_vertices.size(); i++)
        {
            ((Mesh_PosNormalTangentTex0Vertex*)(vertexMem->m_data))[i] = mesh_vertices[i];
            indices[i]                                                 = static_cast<uint16_t>(i);
        }

        VertexBufferHandle vbh = SceneBuffers::createVertexBuffer(vertexMem, stride);
        IndexBufferHandle  ibh = SceneBuffers::createIndexBuffer(iMem);

        ret.m_vertex_handle = vbh;
        ret.m_index_handle  = ibh;

        return ret;
    }

    TextureHandle SceneBuilder::loadTexture(const char* file, bool sRGB)
    {
        auto texture = SceneBuffers::allocTexture();
        int  iw, ih, n;
        texture->m_pixels = stbi_load(file, &iw, &ih, &n, 4);

        if (!texture->m_pixels)
            return PILOT_INVALID_HANDLE;

        texture->m_width        = iw;
        texture->m_height       = ih;
        texture->m_format       = (sRGB) ? PILOT_PIXEL_FORMAT::PILOT_PIXEL_FORMAT_R8G8B8A8_SRGB :
                                           PILOT_PIXEL_FORMAT::PILOT_PIXEL_FORMAT_R8G8B8A8_UNORM;
        texture->m_depth        = 1;
        texture->m_array_layers = 1;
        texture->m_mip_levels   = 1;
        texture->m_type         = PILOT_IMAGE_TYPE::PILOT_IMAGE_TYPE_2D;
        return SceneBuffers::createTexture(texture);
    }

    TextureHandle SceneBuilder::loadTextureHDR(const char* file, int desired_channels)
    {
        auto texture = SceneBuffers::allocTexture();
        int  iw, ih, n;
        texture->m_pixels = stbi_loadf(file, &iw, &ih, &n, desired_channels);

        if (!texture->m_pixels)
            return PILOT_INVALID_HANDLE;

        texture->m_width  = iw;
        texture->m_height = ih;
        switch (desired_channels)
        {
            case 2:
                texture->m_format = PILOT_PIXEL_FORMAT::PILOT_PIXEL_FORMAT_R32G32_FLOAT;
                break;
            case 4:
                texture->m_format = PILOT_PIXEL_FORMAT::PILOT_PIXEL_FORMAT_R32G32B32A32_FLOAT;
                break;
            default:
                // three component format is not supported in some vulkan driver implementations
                throw std::runtime_error("unsupported channels number");
                break;
        }
        texture->m_depth        = 1;
        texture->m_array_layers = 1;
        texture->m_mip_levels   = 1;
        texture->m_type         = PILOT_IMAGE_TYPE::PILOT_IMAGE_TYPE_2D;
        return SceneBuffers::createTexture(texture);
    }

    void SceneManager::initialize()
    {
        GlobalRenderingRes global_rendering_res;

        const auto& global_rendering_res_url = ConfigManager::getInstance().getGlobalRenderingResUrl();
        AssetManager::getInstance().loadAsset(global_rendering_res_url, global_rendering_res);
        setSceneOnce(global_rendering_res);
    }

    int SceneManager::tick(FrameBuffer* buffer)
    {
        buffer->m_uistate->m_editor_camera = m_scene->m_camera;
        return 0;
    }
    const SceneMemory* SceneManager::memoryFromHandle(SceneResourceHandle handle)
    {
        return SceneBuffers::memoryFromHandle(handle);
    }
    const SceneImage* SceneManager::imageFromHandle(TextureHandle handle)
    {
        return SceneBuffers::imageFromHandle(handle);
    }

    void SceneManager::addSceneObject(const GameObjectDesc& go_desc) { m_go_descs.push_back(go_desc); }

    void SceneManager::syncSceneObjects()
    {
        m_scene->lock();
        m_scene->clear();

        releaseMeshHandles();
        releaseMaterialHandles();
        releaseSkeletonBindingHandles();

        m_pick_object_mutex.lock();
        m_instance_id_allocator.clear();
        m_component_transform_map.clear();
        m_mesh_id_gobejct_id_map.clear();

        int material_index = 0;
        int blending_index = 0;

        for (const auto& go_desc : m_go_descs)
        {
            for (size_t i = 0; i < go_desc.getComponents().size(); i++)
            {
                const auto& component = go_desc.getComponents()[i];

                auto go_id           = go_desc.getId();
                auto component_index = i;

                ComponentId component_id                = {go_id, component_index};
                m_component_transform_map[component_id] = component.transform_desc;

                // add mesh to scene
                RenderMesh mesh;
                mesh.m_model_matrix = component.transform_desc.transform_matrix;
                mesh.m_material     = material_index++;
                mesh.m_instance_id  = m_instance_id_allocator.allocGuid(component_id);

                if (std::filesystem::path(component.mesh_desc.mesh_file).extension() == ".obj")
                {
                    auto mesh_handle = getOrCreateMeshHandle(component.mesh_desc.mesh_file);

                    mesh.m_vertexBuffer = mesh_handle.m_vertex_handle;
                    mesh.m_indexBuffer  = mesh_handle.m_index_handle;
                    mesh.m_bounding_box = mesh_handle.m_bounding_box;
                }
                else if (std::filesystem::path(component.mesh_desc.mesh_file).extension() == ".json")
                {
                    mesh.m_joint_matrices.resize(component.skeleton_animation_result.transforms.size());
                    for (size_t i = 0; i < component.skeleton_animation_result.transforms.size(); ++i)
                    {
                        mesh.m_joint_matrices[i] = component.skeleton_animation_result.transforms[i].matrix;
                    }

                    getOrCreateSkeletonBindingHandle(component.skeleton_binding_desc.skeleton_binding_file,
                                                     mesh.m_vertexBuffer,
                                                     mesh.m_indexBuffer,
                                                     mesh.m_bounding_box,
                                                     mesh.skeleton_binding_handle);

                    mesh.m_enable_vertex_blending_1 = mesh.m_joint_matrices.size() > 1;
                }
                else
                {
                    //
                }
                mesh.m_guid = m_mesh_allocator.allocGuid(mesh);
                m_scene->addMesh(mesh);
                m_mesh_id_gobejct_id_map[mesh.m_instance_id] = go_desc.getId();

                std::string texture_file0;
                std::string texture_file1;
                std::string texture_file2;
                std::string texture_file3;
                std::string texture_file4;

                if (component.material_desc.with_texture)
                {
                    texture_file0 = component.material_desc.baseColorTextureFile;
                    texture_file1 = component.material_desc.metallicRoughnessTextureFile;
                    texture_file2 = component.material_desc.normalTextureFile;
                    texture_file3 = component.material_desc.occlusionTextureFile;
                    texture_file4 = component.material_desc.emissiveTextureFile;
                }
                else
                {
                    texture_file0 =
                        AssetManager::getInstance().getFullPath("asset/texture/default/albedo.jpg").generic_string();
                    texture_file1 =
                        AssetManager::getInstance().getFullPath("asset/texture/default/mr.jpg").generic_string();
                    texture_file2 =
                        AssetManager::getInstance().getFullPath("asset/texture/default/normal.jpg").generic_string();
                }

                auto texture0 = getOrCreateImageHandle(texture_file0, true);
                auto texture1 = getOrCreateImageHandle(texture_file1);
                auto texture2 = getOrCreateImageHandle(texture_file2);
                auto texture3 = getOrCreateImageHandle(texture_file3);
                auto texture4 = getOrCreateImageHandle(texture_file4);

                // add material to scene
                Material material;
                material.m_baseColorTexture         = texture0;
                material.m_metallicRoughnessTexture = texture1;
                material.m_normalTexture            = texture2;
                material.m_occlusionTexture         = texture3;
                material.m_emissiveTexture          = texture4;
                material.m_guid                     = m_material_allocator.allocGuid(material);
                m_scene->addMaterial(material);
            }
        }

        m_go_descs.clear();

        m_pick_object_mutex.unlock();
        m_scene->unlock();
    }
    void SceneManager::addReleaseMeshHandle(const MeshHandle& mesh_handle)
    {
        std::lock_guard<std::mutex> lock_guard(m_release_handle_mutex);
        if (std::find(m_mesh_handles_to_release.begin(), m_mesh_handles_to_release.end(), mesh_handle) ==
            std::end(m_mesh_handles_to_release))
        {
            m_mesh_handles_to_release.push_back(mesh_handle);
        }
    }
    void SceneManager::addReleaseMaterialHandle(const PMaterialHandle& material_handle)
    {
        std::lock_guard<std::mutex> lock_guard(m_release_handle_mutex);
        if (std::find(m_material_handles_to_release.begin(), m_material_handles_to_release.end(), material_handle) ==
            std::end(m_material_handles_to_release))
        {
            m_material_handles_to_release.push_back(material_handle);
        }
    }
    void SceneManager::addReleaseSkeletonBindingHandle(const SkeletonBindingBufferHandle& handle)
    {
        std::lock_guard<std::mutex> lock_guard(m_release_handle_mutex);
        if (std::find(m_skeleton_binding_handles_to_release.begin(),
                      m_skeleton_binding_handles_to_release.end(),
                      handle) == std::end(m_skeleton_binding_handles_to_release))
        {
            m_skeleton_binding_handles_to_release.push_back(handle);
        }
    }
    ComponentId SceneManager::getSelectedComponentByNodeId(size_t node_id)
    {
        std::lock_guard<std::mutex> lock_guard(m_pick_object_mutex);
        ComponentId                 component_id;
        if (m_instance_id_allocator.getGuidRelatedElement(node_id, component_id))
        {}
        return component_id;
    }
    bool SceneManager::getTransformDescByComponentId(const ComponentId&       component_id,
                                                     GameObjectTransformDesc& out_transform)
    {
        std::lock_guard<std::mutex> lock_guard(m_pick_object_mutex);
        auto                        find_transform = m_component_transform_map.find(component_id);
        if (find_transform != m_component_transform_map.end())
        {
            out_transform.transform_matrix = find_transform->second.transform_matrix;
            return true;
        }
        return false;
    }
    void SceneManager::setMainViewMatrix(const Matrix4x4& view_matrix, PCurrentCameraType type)
    {
        if (m_scene && m_scene->m_camera)
        {
            m_scene->m_camera->setMainViewMatrix(view_matrix, type);
        }
    }
    void SceneManager::setFOV(float fov)
    {
        if (m_scene && m_scene->m_camera)
        {
            m_scene->m_camera->setFOVx(fov);
        }
    }
    const Vector2 SceneManager::getFOV() const
    {
        if (m_scene && m_scene->m_camera)
        {
            return m_scene->m_camera->getFOV();
        }

        return Vector2();
    }
    void SceneManager::setAxisMesh(std::vector<RenderMesh>& axis_meshes)
    {
        if (m_scene)
        {
            m_scene->lock();
            for (auto& mesh : axis_meshes)
            {
                mesh.m_guid = m_mesh_allocator.allocGuid(mesh);
            }
            m_scene->setAxisMesh(axis_meshes);
            m_scene->unlock();
        }
    }
    MeshHandle SceneManager::getOrCreateMeshHandle(const std::string& mesh_file)
    {
        auto find_it = m_mesh_handle_map.find(mesh_file);
        if (find_it != m_mesh_handle_map.end())
        {
            return find_it->second;
        }

        MeshHandle mesh_handle = SceneBuilder::loadMesh(mesh_file, m_scene->m_minBounds, m_scene->m_maxBounds);
        m_mesh_handle_map.insert(std::make_pair(mesh_file, mesh_handle));

        return mesh_handle;
    }
    TextureHandle SceneManager::getOrCreateImageHandle(const std::string& image_file, bool srgb)
    {
        auto find_it = m_image_handle_map.find(image_file);
        if (find_it != m_image_handle_map.end())
        {
            return find_it->second;
        }

        TextureHandle image_handle;
        image_handle = SceneBuilder::loadTexture(image_file.c_str(), srgb);

        m_image_handle_map.insert(std::make_pair(image_file, image_handle));
        m_handle_image_map.insert(std::make_pair(image_handle, image_file));

        return image_handle;
    }
    void SceneManager::getOrCreateSkeletonBindingHandle(const std::string&           skeleton_binding_file,
                                                        VertexBufferHandle&          vertex_handle,
                                                        IndexBufferHandle&           index_handle,
                                                        AxisAlignedBox&              bounding_box,
                                                        SkeletonBindingBufferHandle& skeleton_binding_handle)
    {
        auto find_vertex = m_vertex_handle_map.find(skeleton_binding_file);
        auto find_index  = m_index_handle_map.find(skeleton_binding_file);
        auto find_it     = m_skeleton_binding_handle_map.find(skeleton_binding_file);

        auto                      binding_file = skeleton_binding_file;
        std::shared_ptr<MeshData> bind_data    = std::make_shared<MeshData>();
        if (find_vertex == m_vertex_handle_map.end() || find_index == m_index_handle_map.end() ||
            find_it == m_skeleton_binding_handle_map.end())
        {
            AssetManager::getInstance().loadAsset<MeshData>(binding_file, *bind_data);
        }

        if (find_vertex == m_vertex_handle_map.end())
        {
            size_t             vertex_size = bind_data->vertex_buffer.size() * sizeof(Mesh_PosNormalTangentTex0Vertex);
            const SceneMemory* vertex_data = SceneBuffers::alloc(vertex_size);
            Mesh_PosNormalTangentTex0Vertex* vertex = (Mesh_PosNormalTangentTex0Vertex*)vertex_data->m_data;
            for (size_t i = 0; i < bind_data->vertex_buffer.size(); i++)
            {
                vertex[i].x  = bind_data->vertex_buffer[i].px;
                vertex[i].y  = bind_data->vertex_buffer[i].py;
                vertex[i].z  = bind_data->vertex_buffer[i].pz;
                vertex[i].nx = bind_data->vertex_buffer[i].nx;
                vertex[i].ny = bind_data->vertex_buffer[i].ny;
                vertex[i].nz = bind_data->vertex_buffer[i].nz;
                vertex[i].tx = bind_data->vertex_buffer[i].tx;
                vertex[i].ty = bind_data->vertex_buffer[i].ty;
                vertex[i].tz = bind_data->vertex_buffer[i].tz;
                vertex[i].u  = bind_data->vertex_buffer[i].u;
                vertex[i].v  = bind_data->vertex_buffer[i].v;

                bounding_box.merge(Vector3(vertex[i].x, vertex[i].y, vertex[i].z));
            }
            vertex_handle = SceneBuffers::createVertexBuffer(vertex_data, vertex_size);

            m_vertex_handle_map.insert(std::make_pair(skeleton_binding_file, vertex_handle));
            m_handle_vertex_map.insert(std::make_pair(vertex_handle, skeleton_binding_file));
        }
        else
        {
            vertex_handle = find_vertex->second;

            Mesh_PosNormalTangentTex0Vertex* vertex =
                (Mesh_PosNormalTangentTex0Vertex*)((SceneMemory*)vertex_handle.handle)->m_data;

            for (size_t i = 0;
                 i < ((SceneMemory*)vertex_handle.handle)->m_size / sizeof(Mesh_PosNormalTangentTex0Vertex);
                 i++)
            {
                auto x = vertex[i].x;
                auto y = vertex[i].y;
                auto z = vertex[i].z;

                bounding_box.merge(Vector3(x, y, z));
            }
        }

        if (find_index == m_index_handle_map.end())
        {
            size_t             index_size = bind_data->index_buffer.size() * sizeof(uint16_t);
            const SceneMemory* index_data = SceneBuffers::alloc(index_size);
            uint16_t*          index      = (uint16_t*)index_data->m_data;
            for (size_t i = 0; i < bind_data->index_buffer.size(); i++)
            {
                index[i] = static_cast<uint16_t>(bind_data->index_buffer[i]);
            }
            index_handle = SceneBuffers::createIndexBuffer(index_data);

            m_index_handle_map.insert(std::make_pair(skeleton_binding_file, index_handle));
            m_handle_index_map.insert(std::make_pair(index_handle, skeleton_binding_file));
        }
        else
        {
            index_handle = find_index->second;
        }

        if (find_it == m_skeleton_binding_handle_map.end())
        {
            size_t              data_size    = bind_data->bind.size() * sizeof(Mesh_VertexBinding);
            const SceneMemory*  mem_data     = SceneBuffers::alloc(data_size);
            Mesh_VertexBinding* binding_data = reinterpret_cast<Mesh_VertexBinding*>(mem_data->m_data);
            for (size_t i = 0; i < bind_data->bind.size(); i++)
            {
                binding_data[i].index0  = bind_data->bind[i].index0;
                binding_data[i].index1  = bind_data->bind[i].index1;
                binding_data[i].index2  = bind_data->bind[i].index2;
                binding_data[i].index3  = bind_data->bind[i].index3;
                binding_data[i].weight0 = bind_data->bind[i].weight0;
                binding_data[i].weight1 = bind_data->bind[i].weight1;
                binding_data[i].weight2 = bind_data->bind[i].weight2;
                binding_data[i].weight3 = bind_data->bind[i].weight3;
            }
            skeleton_binding_handle = SceneBuffers::createSkeletonBindingBuffer(mem_data);

            m_skeleton_binding_handle_map.insert(
                std::make_pair(skeleton_binding_file, SkeletonMeshBinding {skeleton_binding_handle, bounding_box}));
            m_handle_skeleton_binding_map.insert(std::make_pair(skeleton_binding_handle, skeleton_binding_file));
        }
        else
        {
            bounding_box            = find_it->second.m_bounding_box;
            skeleton_binding_handle = find_it->second.m_skeleton_binding_handle;
        }
    }
    void SceneManager::releaseMeshHandles()
    {
        std::lock_guard<std::mutex> lock_guard(m_release_handle_mutex);

        if (m_mesh_handles_to_release.size() < s_max_mesh_handle_count)
            return;

        size_t release_count = 0;
        while (1)
        {
            if (m_mesh_handles_to_release.empty())
                break;

            auto& mesh_handle = m_mesh_handles_to_release.front();

            RenderMesh mesh;
            mesh.m_vertexBuffer = mesh_handle.m_vertex_handle;
            mesh.m_indexBuffer  = mesh_handle.m_index_handle;
            m_mesh_allocator.freeElement(mesh);

            auto find_mesh_file = m_handle_mesh_map.find(mesh_handle);
            if (find_mesh_file != m_handle_mesh_map.end())
            {
                m_mesh_handle_map.erase(find_mesh_file->second);
                m_handle_mesh_map.erase(mesh_handle);
            }
            SceneBuffers::destroy(mesh_handle.m_vertex_handle);
            SceneBuffers::destroy(mesh_handle.m_index_handle);

            m_mesh_handles_to_release.pop_front();

            if (++release_count >= s_release_mesh_handle_count)
            {
                break;
            }
        }
    }
    void SceneManager::releaseMaterialHandles()
    {
        std::lock_guard<std::mutex> lock_guard(m_release_handle_mutex);

        if (m_material_handles_to_release.size() < s_max_material_handle_count)
            return;

        size_t release_count = 0;
        while (1)
        {
            if (m_material_handles_to_release.empty())
                break;

            auto& material_handle = m_material_handles_to_release.front();

            Material material;
            material.m_baseColorTexture         = material_handle.m_image_handle0;
            material.m_metallicRoughnessTexture = material_handle.m_image_handle1;
            material.m_normalTexture            = material_handle.m_image_handle2;
            material.m_occlusionTexture         = material_handle.m_image_handle3;
            material.m_emissiveTexture          = material_handle.m_image_handle4;
            m_material_allocator.freeElement(material);

            releaseImageHandle(material_handle.m_image_handle0);
            releaseImageHandle(material_handle.m_image_handle1);
            releaseImageHandle(material_handle.m_image_handle2);
            releaseImageHandle(material_handle.m_image_handle3);
            releaseImageHandle(material_handle.m_image_handle4);

            m_material_handles_to_release.pop_front();

            if (++release_count >= s_release_material_handle_count)
            {
                break;
            }
        }
    }
    void SceneManager::releaseSkeletonBindingHandles()
    {
        std::lock_guard<std::mutex> lock_guard(m_release_handle_mutex);

        if (m_skeleton_binding_handles_to_release.size() < s_max_skeleton_binding_handle_count)
            return;

        size_t release_count = 0;
        while (1)
        {
            if (m_skeleton_binding_handles_to_release.empty())
                break;

            auto& skeleton_binding_handle = m_skeleton_binding_handles_to_release.front();

            auto find_file = m_handle_skeleton_binding_map.find(skeleton_binding_handle);
            if (find_file != m_handle_skeleton_binding_map.end())
            {
                m_skeleton_binding_handle_map.erase(find_file->second);
                m_handle_skeleton_binding_map.erase(skeleton_binding_handle);
            }
            SceneBuffers::destroy(skeleton_binding_handle);

            m_skeleton_binding_handles_to_release.pop_front();

            if (++release_count >= s_release_skeleton_binding_handle_count)
            {
                break;
            }
        }
    }
    void SceneManager::releaseImageHandle(TextureHandle& image_handle)
    {
        auto find_image_file = m_handle_image_map.find(image_handle);
        if (find_image_file != m_handle_image_map.end())
        {
            m_image_handle_map.erase(find_image_file->second);
            m_handle_image_map.erase(image_handle);
            SceneBuffers::destroy(image_handle);
        }
    }
    void SceneManager::setSceneOnce(const GlobalRenderingRes& global_res)
    {
        if (m_scene && m_scene->m_loaded == false)
        {
            m_scene->m_brdfLUT_texture_handle = SceneBuilder::loadTextureHDR(
                AssetManager::getInstance().getFullPath(global_res.m_brdf_map).generic_string().c_str());
            m_scene->m_irradiance_texture_handle[0] =
                SceneBuilder::loadTextureHDR(AssetManager::getInstance()
                                                 .getFullPath(global_res.m_skybox_irradiance_map.m_positive_x_map)
                                                 .generic_string()
                                                 .c_str());
            m_scene->m_irradiance_texture_handle[1] =
                SceneBuilder::loadTextureHDR(AssetManager::getInstance()
                                                 .getFullPath(global_res.m_skybox_irradiance_map.m_negative_x_map)
                                                 .generic_string()
                                                 .c_str());
            m_scene->m_irradiance_texture_handle[2] =
                SceneBuilder::loadTextureHDR(AssetManager::getInstance()
                                                 .getFullPath(global_res.m_skybox_irradiance_map.m_positive_z_map)
                                                 .generic_string()
                                                 .c_str());
            m_scene->m_irradiance_texture_handle[3] =
                SceneBuilder::loadTextureHDR(AssetManager::getInstance()
                                                 .getFullPath(global_res.m_skybox_irradiance_map.m_negative_z_map)
                                                 .generic_string()
                                                 .c_str());
            m_scene->m_irradiance_texture_handle[4] =
                SceneBuilder::loadTextureHDR(AssetManager::getInstance()
                                                 .getFullPath(global_res.m_skybox_irradiance_map.m_positive_y_map)
                                                 .generic_string()
                                                 .c_str());
            m_scene->m_irradiance_texture_handle[5] =
                SceneBuilder::loadTextureHDR(AssetManager::getInstance()
                                                 .getFullPath(global_res.m_skybox_irradiance_map.m_negative_y_map)
                                                 .generic_string()
                                                 .c_str());
            m_scene->m_specular_texture_handle[0] =
                SceneBuilder::loadTextureHDR(AssetManager::getInstance()
                                                 .getFullPath(global_res.m_skybox_specular_map.m_positive_x_map)
                                                 .generic_string()
                                                 .c_str());
            m_scene->m_specular_texture_handle[1] =
                SceneBuilder::loadTextureHDR(AssetManager::getInstance()
                                                 .getFullPath(global_res.m_skybox_specular_map.m_negative_x_map)
                                                 .generic_string()
                                                 .c_str());
            m_scene->m_specular_texture_handle[2] =
                SceneBuilder::loadTextureHDR(AssetManager::getInstance()
                                                 .getFullPath(global_res.m_skybox_specular_map.m_positive_z_map)
                                                 .generic_string()
                                                 .c_str());
            m_scene->m_specular_texture_handle[3] =
                SceneBuilder::loadTextureHDR(AssetManager::getInstance()
                                                 .getFullPath(global_res.m_skybox_specular_map.m_negative_z_map)
                                                 .generic_string()
                                                 .c_str());
            m_scene->m_specular_texture_handle[4] =
                SceneBuilder::loadTextureHDR(AssetManager::getInstance()
                                                 .getFullPath(global_res.m_skybox_specular_map.m_positive_y_map)
                                                 .generic_string()
                                                 .c_str());
            m_scene->m_specular_texture_handle[5] =
                SceneBuilder::loadTextureHDR(AssetManager::getInstance()
                                                 .getFullPath(global_res.m_skybox_specular_map.m_negative_y_map)
                                                 .generic_string()
                                                 .c_str());

            m_scene->m_color_grading_LUT_texture_handle = SceneBuilder::loadTexture(
                AssetManager::getInstance().getFullPath(global_res.m_color_grading_map).generic_string().c_str());

            m_scene->m_sky_color     = global_res.m_sky_color.toVector3();
            m_scene->m_ambient_light = {global_res.m_ambient_light.toVector3()};

            const CameraPose& camera_pose = global_res.m_camera_config.m_pose;

            m_scene->m_camera = std::make_shared<PCamera>();
            m_scene->m_camera->lookAt(camera_pose.m_position, camera_pose.m_target, camera_pose.m_up);
            m_scene->m_camera->m_zfar  = global_res.m_camera_config.m_z_far;
            m_scene->m_camera->m_znear = global_res.m_camera_config.m_z_near;
            m_scene->m_camera->setAspect(global_res.m_camera_config.m_aspect.x / global_res.m_camera_config.m_aspect.y);

            m_scene->m_directional_light.m_direction = global_res.m_directional_light.m_direction.normalisedCopy();
            m_scene->m_directional_light.m_color     = global_res.m_directional_light.m_color.toVector3();

            m_scene->m_loaded = true;
        }
    }

    const size_t SceneManager::getGObjectIDByMeshID(size_t mesh_id) const
    {
        auto iter = m_mesh_id_gobejct_id_map.find(mesh_id);
        if (iter == m_mesh_id_gobejct_id_map.end())
        {
            return PILOT_INVALID_GOBJECT_ID;
        }

        return iter->second;
    }

} // namespace Pilot
