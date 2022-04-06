#include "editor/include/axis.h"

namespace Pilot
{
    EditorTranslationAxis::EditorTranslationAxis()
    {
        // create translation axis render mesh

        const float radius   = 0.031f;
        const int   segments = 12;

        uint32_t stride = sizeof(Mesh_PosNormalTangentTex0Vertex);
        // vertex
        auto translation_axis_vertex = SceneBuffers::alloc((3 * segments + 2) * 3 * stride);

        // x
        for (int i = 0; i < segments; ++i)
        {
            Mesh_PosNormalTangentTex0Vertex& vertex =
                *(Mesh_PosNormalTangentTex0Vertex*)(translation_axis_vertex->m_data + (0 * segments + i) * stride);
            vertex.x = 0.0f;
            vertex.y = sin(i * 2 * Math_PI / segments) * radius;
            vertex.z = cos(i * 2 * Math_PI / segments) * radius;
            vertex.u = 0.0f;

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v                          = 0.0f;
        }

        for (int i = 0; i < segments; ++i)
        {
            Mesh_PosNormalTangentTex0Vertex& vertex =
                *(Mesh_PosNormalTangentTex0Vertex*)(translation_axis_vertex->m_data + (1 * segments + i) * stride);
            vertex.x = 1.5f;
            vertex.y =
                (*(Mesh_PosNormalTangentTex0Vertex*)(translation_axis_vertex->m_data + (0 * segments + i) * stride)).y;
            vertex.z =
                (*(Mesh_PosNormalTangentTex0Vertex*)(translation_axis_vertex->m_data + (0 * segments + i) * stride)).z;
            vertex.u = 0.0f;

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v                          = 0.0f;
        }

        for (int i = 0; i < segments; ++i)
        {
            Mesh_PosNormalTangentTex0Vertex& vertex =
                *(Mesh_PosNormalTangentTex0Vertex*)(translation_axis_vertex->m_data + (2 * segments + i) * stride);
            vertex.x = 1.5f;
            vertex.y =
                (*(Mesh_PosNormalTangentTex0Vertex*)(translation_axis_vertex->m_data + (0 * segments + i) * stride)).y *
                4.5f;
            vertex.z =
                (*(Mesh_PosNormalTangentTex0Vertex*)(translation_axis_vertex->m_data + (0 * segments + i) * stride)).z *
                4.5f;
            vertex.u = 0.0f;

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v                          = 0.0f;
        }
        {
            Mesh_PosNormalTangentTex0Vertex& vertex_0 =
                *(Mesh_PosNormalTangentTex0Vertex*)(translation_axis_vertex->m_data + (3 * segments + 0) * stride);
            vertex_0.x = 1.5f;
            vertex_0.y = 0.0f;
            vertex_0.z = 0.0f;
            vertex_0.u = 0.0f;

            vertex_0.nx = vertex_0.ny = vertex_0.nz = 0.0f;
            vertex_0.tx = vertex_0.ty = vertex_0.tz = 0.0f;
            vertex_0.v                              = 0.0f;

            Mesh_PosNormalTangentTex0Vertex& vertex_1 =
                *(Mesh_PosNormalTangentTex0Vertex*)(translation_axis_vertex->m_data + (3 * segments + 1) * stride);
            vertex_1.x = 1.9f;
            vertex_1.y = 0.0f;
            vertex_1.z = 0.0f;
            vertex_1.u = 0.0f;

            vertex_1.nx = vertex_1.ny = vertex_1.nz = 0.0f;
            vertex_1.tx = vertex_1.ty = vertex_1.tz = 0.0f;
            vertex_1.v                              = 0.0f;
        }

        // y, z
        for (int i = 0; i < 3 * segments + 2; ++i)
        {
            Mesh_PosNormalTangentTex0Vertex& vertex_y =
                *(Mesh_PosNormalTangentTex0Vertex*)(translation_axis_vertex->m_data +
                                                    ((3 * segments + 2) * 1 + i) * stride);
            vertex_y.x = -(*(Mesh_PosNormalTangentTex0Vertex*)(translation_axis_vertex->m_data + i * stride)).y;
            vertex_y.y = (*(Mesh_PosNormalTangentTex0Vertex*)(translation_axis_vertex->m_data + i * stride)).x;
            vertex_y.z = (*(Mesh_PosNormalTangentTex0Vertex*)(translation_axis_vertex->m_data + i * stride)).z;
            vertex_y.u = 1.0f;

            vertex_y.nx = vertex_y.ny = vertex_y.nz = 0.0f;
            vertex_y.tx = vertex_y.ty = vertex_y.tz = 0.0f;
            vertex_y.v                              = 0.0f;

            Mesh_PosNormalTangentTex0Vertex& vertex_z =
                *(Mesh_PosNormalTangentTex0Vertex*)(translation_axis_vertex->m_data +
                                                    ((3 * segments + 2) * 2 + i) * stride);
            vertex_z.x = -(*(Mesh_PosNormalTangentTex0Vertex*)(translation_axis_vertex->m_data + i * stride)).z;
            vertex_z.y = (*(Mesh_PosNormalTangentTex0Vertex*)(translation_axis_vertex->m_data + i * stride)).y;
            vertex_z.z = (*(Mesh_PosNormalTangentTex0Vertex*)(translation_axis_vertex->m_data + i * stride)).x;
            vertex_z.u = 2.0f;

            vertex_z.nx = vertex_z.ny = vertex_z.nz = 0.0f;
            vertex_z.tx = vertex_z.ty = vertex_z.tz = 0.0f;
            vertex_z.v                              = 0.0f;
        }

        m_vertexBuffer = SceneBuffers::createVertexBuffer(translation_axis_vertex, (3 * segments + 2) * 3);

        auto      indexMem   = SceneBuffers::alloc((4 * segments * 3) * 3 * sizeof(uint16_t));
        uint16_t* index_info = (uint16_t*)indexMem->m_data;

        for (int i = 0; i < segments; ++i)
        {
            index_info[0 * segments * 3 + i * 6 + 0] = (uint16_t)(0 * segments + i);
            index_info[0 * segments * 3 + i * 6 + 1] = (uint16_t)1 * segments + i;
            index_info[0 * segments * 3 + i * 6 + 2] = (uint16_t)1 * segments + ((i + 1) % segments);

            index_info[0 * segments * 3 + i * 6 + 3] = (uint16_t)1 * segments + ((i + 1) % segments);
            index_info[0 * segments * 3 + i * 6 + 4] = (uint16_t)0 * segments + ((i + 1) % segments);
            index_info[0 * segments * 3 + i * 6 + 5] = (uint16_t)0 * segments + i;
        }
        for (int i = 0; i < segments; ++i)
        {
            index_info[2 * segments * 3 + i * 3 + 0] = (uint16_t)3 * segments + 0;
            index_info[2 * segments * 3 + i * 3 + 1] = (uint16_t)2 * segments + i;
            index_info[2 * segments * 3 + i * 3 + 2] = (uint16_t)2 * segments + ((i + 1) % segments);
        }
        for (int i = 0; i < segments; ++i)
        {
            index_info[3 * segments * 3 + i * 3 + 0] = (uint16_t)2 * segments + i;
            index_info[3 * segments * 3 + i * 3 + 1] = (uint16_t)3 * segments + 1;
            index_info[3 * segments * 3 + i * 3 + 2] = (uint16_t)2 * segments + ((i + 1) % segments);
        }

        for (int i = 0; i < 4 * segments * 3; ++i)
        {
            index_info[4 * segments * 3 * 1 + i] = (uint16_t)((3 * segments + 2) * 1) + index_info[i];
            index_info[4 * segments * 3 * 2 + i] = (uint16_t)((3 * segments + 2) * 2) + index_info[i];
        }

        m_indexBuffer = SceneBuffers::createIndexBuffer(indexMem);
    }

    EditorRotationAxis::EditorRotationAxis()
    {
        // create rotation axis render mesh

        const float inner_radius = 0.9f;
        const float outer_radius = 1.0f;
        const int   segments     = 24;

        uint32_t stride = sizeof(Mesh_PosNormalTangentTex0Vertex);
        // vertex
        auto rotation_axis_vertex = SceneBuffers::alloc(2 * 3 * segments * stride);
        // inner xy
        for (int i = 0; i < segments; i++)
        {
            Mesh_PosNormalTangentTex0Vertex& vertex =
                *(Mesh_PosNormalTangentTex0Vertex*)(rotation_axis_vertex->m_data + (0 * segments + i) * stride);
            vertex.x = cos(2 * Math_PI / segments * i) * inner_radius;
            vertex.y = sin(2 * Math_PI / segments * i) * inner_radius;
            vertex.z = 0.0f;
            vertex.u = 2.0f;

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v                          = 0.0f;
        }
        // outer xy
        for (int i = 0; i < segments; i++)
        {
            Mesh_PosNormalTangentTex0Vertex& vertex =
                *(Mesh_PosNormalTangentTex0Vertex*)(rotation_axis_vertex->m_data + (1 * segments + i) * stride);
            vertex.x = cos(2 * Math_PI / segments * i) * outer_radius;
            vertex.y = sin(2 * Math_PI / segments * i) * outer_radius;
            vertex.z = 0.0f;
            vertex.u = 2.0f;

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v                          = 0.0f;
        }
        // inner yz
        for (int i = 0; i < segments; i++)
        {
            Mesh_PosNormalTangentTex0Vertex& vertex =
                *(Mesh_PosNormalTangentTex0Vertex*)(rotation_axis_vertex->m_data + (2 * segments + i) * stride);
            vertex.x = 0.0f;
            vertex.y = cos(2 * Math_PI / segments * i) * inner_radius;
            vertex.z = sin(2 * Math_PI / segments * i) * inner_radius;
            vertex.u = 0.0f;

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v                          = 0.0f;
        }
        // outer yz
        for (int i = 0; i < segments; i++)
        {
            Mesh_PosNormalTangentTex0Vertex& vertex =
                *(Mesh_PosNormalTangentTex0Vertex*)(rotation_axis_vertex->m_data + (3 * segments + i) * stride);
            vertex.x = 0.0f;
            vertex.y = cos(2 * Math_PI / segments * i) * outer_radius;
            vertex.z = sin(2 * Math_PI / segments * i) * outer_radius;
            vertex.u = 0.0f;

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v                          = 0.0f;
        }
        // inner xz
        for (int i = 0; i < segments; i++)
        {
            Mesh_PosNormalTangentTex0Vertex& vertex =
                *(Mesh_PosNormalTangentTex0Vertex*)(rotation_axis_vertex->m_data + (4 * segments + i) * stride);
            vertex.x = cos(2 * Math_PI / segments * i) * inner_radius;
            vertex.y = 0.0f;
            vertex.z = sin(2 * Math_PI / segments * i) * inner_radius;
            vertex.u = 1.0f;

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v                          = 0.0f;
        }
        // outer xz
        for (int i = 0; i < segments; i++)
        {
            Mesh_PosNormalTangentTex0Vertex& vertex =
                *(Mesh_PosNormalTangentTex0Vertex*)(rotation_axis_vertex->m_data + (5 * segments + i) * stride);
            vertex.x = cos(2 * Math_PI / segments * i) * outer_radius;
            vertex.y = 0.0f;
            vertex.z = sin(2 * Math_PI / segments * i) * outer_radius;
            vertex.u = 1.0f;

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v                          = 0.0f;
        }

        m_vertexBuffer = SceneBuffers::createVertexBuffer(rotation_axis_vertex, 2 * 3 * segments);

        // index
        auto      indexMem   = SceneBuffers::alloc(2 * 3 * segments * 3 * sizeof(uint16_t));
        uint16_t* index_info = (uint16_t*)indexMem->m_data;

        // xoy inner
        for (int i = 0; i < segments; i++)
        {
            index_info[(3 * i) + 0] = (uint16_t)(i % segments);
            index_info[(3 * i) + 1] = (uint16_t)((i + 1) % segments);
            index_info[(3 * i) + 2] = (uint16_t)(i % segments + segments);
        }
        // xoy outer
        for (int i = 0; i < segments; i++)
        {
            index_info[1 * 3 * segments + (3 * i) + 0] = (uint16_t)(i % segments + segments);
            index_info[1 * 3 * segments + (3 * i) + 1] = (uint16_t)((i + 1) % segments + segments);
            index_info[1 * 3 * segments + (3 * i) + 2] = (uint16_t)((i + 1) % segments);
        }
        // yoz inner
        for (int i = 0; i < segments; i++)
        {
            index_info[2 * 3 * segments + (3 * i) + 0] = (uint16_t)(i % segments + segments * 2);
            index_info[2 * 3 * segments + (3 * i) + 1] = (uint16_t)((i + 1) % segments + segments * 2);
            index_info[2 * 3 * segments + (3 * i) + 2] = (uint16_t)(i % segments + segments * 3);
        }
        // yoz outer
        for (int i = 0; i < segments; i++)
        {
            index_info[3 * 3 * segments + (3 * i) + 0] = (uint16_t)(i % segments + segments * 3);
            index_info[3 * 3 * segments + (3 * i) + 1] = (uint16_t)((i + 1) % segments + segments * 3);
            index_info[3 * 3 * segments + (3 * i) + 2] = (uint16_t)((i + 1) % segments + segments * 2);
        }
        // xoz inner
        for (int i = 0; i < segments; i++)
        {
            index_info[4 * 3 * segments + (3 * i) + 0] = (uint16_t)(i % segments + segments * 4);
            index_info[4 * 3 * segments + (3 * i) + 1] = (uint16_t)((i + 1) % segments + segments * 4);
            index_info[4 * 3 * segments + (3 * i) + 2] = (uint16_t)(i % segments + segments * 5);
        }
        // xoz outer
        for (int i = 0; i < segments; i++)
        {
            index_info[5 * 3 * segments + (3 * i) + 0] = (uint16_t)(i % segments + segments * 5);
            index_info[5 * 3 * segments + (3 * i) + 1] = (uint16_t)((i + 1) % segments + segments * 5);
            index_info[5 * 3 * segments + (3 * i) + 2] = (uint16_t)((i + 1) % segments + segments * 4);
        }

        m_indexBuffer = SceneBuffers::createIndexBuffer(indexMem);
    }

    EditorScaleAxis::EditorScaleAxis()
    {
        const float radius   = 0.031f;
        const int   segments = 12;
        // DynamicArray<Vector4> positions((2 * segments + 8) * 3 + 8);

        uint32_t stride = sizeof(Mesh_PosNormalTangentTex0Vertex);
        // vertex
        auto scale_axis_vertex = SceneBuffers::alloc(((2 * segments + 8) * 3 + 8) * stride);

        for (int i = 0; i < segments; ++i)
        {
            Mesh_PosNormalTangentTex0Vertex& vertex =
                *(Mesh_PosNormalTangentTex0Vertex*)(scale_axis_vertex->m_data + (0 * segments + i) * stride);
            vertex.x = 0.0f;
            vertex.y = sin(i * 2 * Math_PI / segments) * radius;
            vertex.z = cos(i * 2 * Math_PI / segments) * radius;
            vertex.u = 0.0f;

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v                          = 0.0f;
        }
        for (int i = 0; i < segments; ++i)
        {
            Mesh_PosNormalTangentTex0Vertex& vertex =
                *(Mesh_PosNormalTangentTex0Vertex*)(scale_axis_vertex->m_data + (1 * segments + i) * stride);
            vertex.x = 1.6 - radius * 10;
            vertex.y = (*(Mesh_PosNormalTangentTex0Vertex*)(scale_axis_vertex->m_data + (0 * segments + i) * stride)).y;
            vertex.z = (*(Mesh_PosNormalTangentTex0Vertex*)(scale_axis_vertex->m_data + (0 * segments + i) * stride)).z;
            vertex.u = 0.0f;

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v                          = 0.0f;
            // positions[1 * segments + i] = Vector4(1.6 - radius * 10, positions[0 * segments +
            // i].y, positions[0 * segments + i].z, 0);
        }
        {
            Mesh_PosNormalTangentTex0Vertex& vertex0 =
                *(Mesh_PosNormalTangentTex0Vertex*)(scale_axis_vertex->m_data + (2 * segments + 0) * stride);
            vertex0.x = 1.6 - radius * 10;
            vertex0.y = +radius * 5;
            vertex0.z = +radius * 5;
            vertex0.u = 0.0f;

            vertex0.nx = vertex0.ny = vertex0.nz = 0.0f;
            vertex0.tx = vertex0.ty = vertex0.tz = 0.0f;
            vertex0.v                            = 0.0f;
            // positions[2 * segments + 0] = Vector4(1.6 - radius * 10, +radius * 5, +radius * 5,
            // 0);

            Mesh_PosNormalTangentTex0Vertex& vertex1 =
                *(Mesh_PosNormalTangentTex0Vertex*)(scale_axis_vertex->m_data + (2 * segments + 1) * stride);
            vertex1.x = 1.6 - radius * 10;
            vertex1.y = +radius * 5;
            vertex1.z = -radius * 5;
            vertex1.u = 0.0f;

            vertex1.nx = vertex1.ny = vertex1.nz = 0.0f;
            vertex1.tx = vertex1.ty = vertex1.tz = 0.0f;
            vertex1.v                            = 0.0f;
            // positions[2 * segments + 1] = Vector4(1.6 - radius * 10, +radius * 5, -radius * 5,
            // 0);

            Mesh_PosNormalTangentTex0Vertex& vertex2 =
                *(Mesh_PosNormalTangentTex0Vertex*)(scale_axis_vertex->m_data + (2 * segments + 2) * stride);
            vertex2.x = 1.6 - radius * 10;
            vertex2.y = -radius * 5;
            vertex2.z = +radius * 5;
            vertex2.u = 0.0f;

            vertex2.nx = vertex2.ny = vertex2.nz = 0.0f;
            vertex2.tx = vertex2.ty = vertex2.tz = 0.0f;
            vertex2.v                            = 0.0f;
            // positions[2 * segments + 2] = Vector4(1.6 - radius * 10, -radius * 5, +radius * 5,
            // 0);

            Mesh_PosNormalTangentTex0Vertex& vertex3 =
                *(Mesh_PosNormalTangentTex0Vertex*)(scale_axis_vertex->m_data + (2 * segments + 3) * stride);
            vertex3.x = 1.6 - radius * 10;
            vertex3.y = -radius * 5;
            vertex3.z = -radius * 5;
            vertex3.u = 0.0f;

            vertex3.nx = vertex3.ny = vertex3.nz = 0.0f;
            vertex3.tx = vertex3.ty = vertex3.tz = 0.0f;
            vertex3.v                            = 0.0f;
            // positions[2 * segments + 3] = Vector4(1.6 - radius * 10, -radius * 5, -radius * 5,
            // 0);

            Mesh_PosNormalTangentTex0Vertex& vertex4 =
                *(Mesh_PosNormalTangentTex0Vertex*)(scale_axis_vertex->m_data + (2 * segments + 4) * stride);
            vertex4.x = 1.6;
            vertex4.y = +radius * 5;
            vertex4.z = +radius * 5;
            vertex4.u = 0.0f;

            vertex4.nx = vertex4.ny = vertex4.nz = 0.0f;
            vertex4.tx = vertex4.ty = vertex4.tz = 0.0f;
            vertex4.v                            = 0.0f;
            // positions[2 * segments + 4] = Vector4(1.6, +radius * 5, +radius * 5, 0);

            Mesh_PosNormalTangentTex0Vertex& vertex5 =
                *(Mesh_PosNormalTangentTex0Vertex*)(scale_axis_vertex->m_data + (2 * segments + 5) * stride);
            vertex5.x = 1.6;
            vertex5.y = +radius * 5;
            vertex5.z = -radius * 5;
            vertex5.u = 0.0f;

            vertex5.nx = vertex5.ny = vertex5.nz = 0.0f;
            vertex5.tx = vertex5.ty = vertex5.tz = 0.0f;
            vertex5.v                            = 0.0f;
            // positions[2 * segments + 5] = Vector4(1.6, +radius * 5, -radius * 5, 0);

            Mesh_PosNormalTangentTex0Vertex& vertex6 =
                *(Mesh_PosNormalTangentTex0Vertex*)(scale_axis_vertex->m_data + (2 * segments + 6) * stride);
            vertex6.x = 1.6;
            vertex6.y = -radius * 5;
            vertex6.z = +radius * 5;
            vertex6.u = 0.0f;

            vertex6.nx = vertex6.ny = vertex6.nz = 0.0f;
            vertex6.tx = vertex6.ty = vertex6.tz = 0.0f;
            vertex6.v                            = 0.0f;
            // positions[2 * segments + 6] = Vector4(1.6, -radius * 5, +radius * 5, 0);

            Mesh_PosNormalTangentTex0Vertex& vertex7 =
                *(Mesh_PosNormalTangentTex0Vertex*)(scale_axis_vertex->m_data + (2 * segments + 7) * stride);
            vertex7.x = 1.6;
            vertex7.y = -radius * 5;
            vertex7.z = -radius * 5;
            vertex7.u = 0.0f;

            vertex7.nx = vertex7.ny = vertex7.nz = 0.0f;
            vertex7.tx = vertex7.ty = vertex7.tz = 0.0f;
            vertex7.v                            = 0.0f;
            // positions[2 * segments + 7] = Vector4(1.6, -radius * 5, -radius * 5, 0);
        }

        for (int i = 0; i < 2 * segments + 8; ++i)
        {
            Mesh_PosNormalTangentTex0Vertex& vertex1 =
                *(Mesh_PosNormalTangentTex0Vertex*)(scale_axis_vertex->m_data + ((2 * segments + 8) * 1 + i) * stride);
            vertex1.x = -(*(Mesh_PosNormalTangentTex0Vertex*)(scale_axis_vertex->m_data + i * stride)).y;
            vertex1.y = (*(Mesh_PosNormalTangentTex0Vertex*)(scale_axis_vertex->m_data + i * stride)).x;
            vertex1.z = (*(Mesh_PosNormalTangentTex0Vertex*)(scale_axis_vertex->m_data + i * stride)).z;
            vertex1.u = 1.0f;

            vertex1.nx = vertex1.ny = vertex1.nz = 0.0f;
            vertex1.tx = vertex1.ty = vertex1.tz = 0.0f;
            vertex1.v                            = 0.0f;
            // positions[(2 * segments + 8) * 1 + i] = Vector4(-positions[i].y, positions[i].x,
            // positions[i].z, 1);

            Mesh_PosNormalTangentTex0Vertex& vertex2 =
                *(Mesh_PosNormalTangentTex0Vertex*)(scale_axis_vertex->m_data + ((2 * segments + 8) * 2 + i) * stride);
            vertex2.x = -(*(Mesh_PosNormalTangentTex0Vertex*)(scale_axis_vertex->m_data + i * stride)).z;
            vertex2.y = (*(Mesh_PosNormalTangentTex0Vertex*)(scale_axis_vertex->m_data + i * stride)).y;
            vertex2.z = (*(Mesh_PosNormalTangentTex0Vertex*)(scale_axis_vertex->m_data + i * stride)).x;
            vertex2.u = 2.0f;

            vertex2.nx = vertex2.ny = vertex2.nz = 0.0f;
            vertex2.tx = vertex2.ty = vertex2.tz = 0.0f;
            vertex2.v                            = 0.0f;
            // positions[(2 * segments + 8) * 2 + i] = Vector4(-positions[i].z, positions[i].y,
            // positions[i].x, 2);
        }

        int start_vertex_index = (2 * segments + 8) * 3;
        {
            Mesh_PosNormalTangentTex0Vertex& vertex0 =
                *(Mesh_PosNormalTangentTex0Vertex*)(scale_axis_vertex->m_data + (start_vertex_index + 0) * stride);
            vertex0.x = 0.0f;
            vertex0.y = 0.0f;
            vertex0.z = 0.0f;
            vertex0.u = 6.0f;

            vertex0.nx = vertex0.ny = vertex0.nz = 0.0f;
            vertex0.tx = vertex0.ty = vertex0.tz = 0.0f;
            vertex0.v                            = 0.0f;
            // positions[start_vertex_index + 0] = Vector4(0.0f, 0.0f, 0.0f, 6.0f);

            Mesh_PosNormalTangentTex0Vertex& vertex1 =
                *(Mesh_PosNormalTangentTex0Vertex*)(scale_axis_vertex->m_data + (start_vertex_index + 1) * stride);
            vertex1.x = 0.1f;
            vertex1.y = 0.0f;
            vertex1.z = 0.0f;
            vertex1.u = 6.0f;

            vertex1.nx = vertex1.ny = vertex1.nz = 0.0f;
            vertex1.tx = vertex1.ty = vertex1.tz = 0.0f;
            vertex1.v                            = 0.0f;
            // positions[start_vertex_index + 1] = Vector4(0.1f, 0.0f, 0.0f, 6.0f);

            Mesh_PosNormalTangentTex0Vertex& vertex2 =
                *(Mesh_PosNormalTangentTex0Vertex*)(scale_axis_vertex->m_data + (start_vertex_index + 2) * stride);
            vertex2.x = 0.1f;
            vertex2.y = 0.1f;
            vertex2.z = 0.0f;
            vertex2.u = 6.0f;

            vertex2.nx = vertex2.ny = vertex2.nz = 0.0f;
            vertex2.tx = vertex2.ty = vertex2.tz = 0.0f;
            vertex2.v                            = 0.0f;
            // positions[start_vertex_index + 2] = Vector4(0.1f, 0.1f, 0.0f, 6.0f);

            Mesh_PosNormalTangentTex0Vertex& vertex3 =
                *(Mesh_PosNormalTangentTex0Vertex*)(scale_axis_vertex->m_data + (start_vertex_index + 3) * stride);
            vertex3.x = 0.0f;
            vertex3.y = 0.1f;
            vertex3.z = 0.0f;
            vertex3.u = 6.0f;

            vertex3.nx = vertex3.ny = vertex3.nz = 0.0f;
            vertex3.tx = vertex3.ty = vertex3.tz = 0.0f;
            vertex3.v                            = 0.0f;
            // positions[start_vertex_index + 3] = Vector4(0.0f, 0.1f, 0.0f, 6.0f);

            Mesh_PosNormalTangentTex0Vertex& vertex4 =
                *(Mesh_PosNormalTangentTex0Vertex*)(scale_axis_vertex->m_data + (start_vertex_index + 4) * stride);
            vertex4.x = 0.0f;
            vertex4.y = 0.0f;
            vertex4.z = 0.1f;
            vertex4.u = 6.0f;

            vertex4.nx = vertex4.ny = vertex4.nz = 0.0f;
            vertex4.tx = vertex4.ty = vertex4.tz = 0.0f;
            vertex4.v                            = 0.0f;
            // positions[start_vertex_index + 4] = Vector4(0.0f, 0.0f, 0.1f, 6.0f);

            Mesh_PosNormalTangentTex0Vertex& vertex5 =
                *(Mesh_PosNormalTangentTex0Vertex*)(scale_axis_vertex->m_data + (start_vertex_index + 5) * stride);
            vertex5.x = 0.1f;
            vertex5.y = 0.0f;
            vertex5.z = 0.1f;
            vertex5.u = 6.0f;

            vertex5.nx = vertex5.ny = vertex5.nz = 0.0f;
            vertex5.tx = vertex5.ty = vertex5.tz = 0.0f;
            vertex5.v                            = 0.0f;
            // positions[start_vertex_index + 5] = Vector4(0.1f, 0.0f, 0.1f, 6.0f);

            Mesh_PosNormalTangentTex0Vertex& vertex6 =
                *(Mesh_PosNormalTangentTex0Vertex*)(scale_axis_vertex->m_data + (start_vertex_index + 6) * stride);
            vertex6.x = 0.1f;
            vertex6.y = 0.1f;
            vertex6.z = 0.1f;
            vertex6.u = 6.0f;

            vertex6.nx = vertex6.ny = vertex6.nz = 0.0f;
            vertex6.tx = vertex6.ty = vertex6.tz = 0.0f;
            vertex6.v                            = 0.0f;
            // positions[start_vertex_index + 6] = Vector4(0.1f, 0.1f, 0.1f, 6.0f);

            Mesh_PosNormalTangentTex0Vertex& vertex7 =
                *(Mesh_PosNormalTangentTex0Vertex*)(scale_axis_vertex->m_data + (start_vertex_index + 7) * stride);
            vertex7.x = 0.0f;
            vertex7.y = 0.1f;
            vertex7.z = 0.1f;
            vertex7.u = 6.0f;

            vertex7.nx = vertex7.ny = vertex7.nz = 0.0f;
            vertex7.tx = vertex7.ty = vertex7.tz = 0.0f;
            vertex7.v                            = 0.0f;
            // positions[start_vertex_index + 7] = Vector4(0.0f, 0.1f, 0.1f, 6.0f);
        }

        m_vertexBuffer = SceneBuffers::createVertexBuffer(scale_axis_vertex, (2 * segments + 8) * 3 + 8);

        // DynamicArray<UShort> indices(((2 * segments + 12) * 3) * 3 + 3 * 2 * 6);

        // index
        auto      indexMem   = SceneBuffers::alloc((((2 * segments + 12) * 3) * 3 + 3 * 2 * 6) * sizeof(uint16_t));
        uint16_t* index_info = (uint16_t*)indexMem->m_data;
        for (int i = 0; i < segments; ++i)
        {
            index_info[0 * segments * 3 + i * 6 + 0] = (uint16_t)(0 * segments + i);
            index_info[0 * segments * 3 + i * 6 + 1] = (uint16_t)(1 * segments + i);
            index_info[0 * segments * 3 + i * 6 + 2] = (uint16_t)(1 * segments + ((i + 1) % segments));

            index_info[0 * segments * 3 + i * 6 + 3] = (uint16_t)(1 * segments + ((i + 1) % segments));
            index_info[0 * segments * 3 + i * 6 + 4] = (uint16_t)(0 * segments + ((i + 1) % segments));
            index_info[0 * segments * 3 + i * 6 + 5] = (uint16_t)(0 * segments + i);
        }
        {
            index_info[2 * segments * 3 + 0 * 3 + 0] = (uint16_t)(2 * segments + 0);
            index_info[2 * segments * 3 + 0 * 3 + 1] = (uint16_t)(2 * segments + 1);
            index_info[2 * segments * 3 + 0 * 3 + 2] = (uint16_t)(2 * segments + 3);
            index_info[2 * segments * 3 + 1 * 3 + 0] = (uint16_t)(2 * segments + 3);
            index_info[2 * segments * 3 + 1 * 3 + 1] = (uint16_t)(2 * segments + 2);
            index_info[2 * segments * 3 + 1 * 3 + 2] = (uint16_t)(2 * segments + 0);

            index_info[2 * segments * 3 + 2 * 3 + 0] = (uint16_t)(2 * segments + 1);
            index_info[2 * segments * 3 + 2 * 3 + 1] = (uint16_t)(2 * segments + 5);
            index_info[2 * segments * 3 + 2 * 3 + 2] = (uint16_t)(2 * segments + 7);
            index_info[2 * segments * 3 + 3 * 3 + 0] = (uint16_t)(2 * segments + 7);
            index_info[2 * segments * 3 + 3 * 3 + 1] = (uint16_t)(2 * segments + 3);
            index_info[2 * segments * 3 + 3 * 3 + 2] = (uint16_t)(2 * segments + 1);

            index_info[2 * segments * 3 + 4 * 3 + 0] = (uint16_t)(2 * segments + 5);
            index_info[2 * segments * 3 + 4 * 3 + 1] = (uint16_t)(2 * segments + 4);
            index_info[2 * segments * 3 + 4 * 3 + 2] = (uint16_t)(2 * segments + 6);
            index_info[2 * segments * 3 + 5 * 3 + 0] = (uint16_t)(2 * segments + 6);
            index_info[2 * segments * 3 + 5 * 3 + 1] = (uint16_t)(2 * segments + 7);
            index_info[2 * segments * 3 + 5 * 3 + 2] = (uint16_t)(2 * segments + 5);

            index_info[2 * segments * 3 + 6 * 3 + 0] = (uint16_t)(2 * segments + 4);
            index_info[2 * segments * 3 + 6 * 3 + 1] = (uint16_t)(2 * segments + 0);
            index_info[2 * segments * 3 + 6 * 3 + 2] = (uint16_t)(2 * segments + 2);
            index_info[2 * segments * 3 + 7 * 3 + 0] = (uint16_t)(2 * segments + 2);
            index_info[2 * segments * 3 + 7 * 3 + 1] = (uint16_t)(2 * segments + 6);
            index_info[2 * segments * 3 + 7 * 3 + 2] = (uint16_t)(2 * segments + 4);

            index_info[2 * segments * 3 + 8 * 3 + 0] = (uint16_t)(2 * segments + 4);
            index_info[2 * segments * 3 + 8 * 3 + 1] = (uint16_t)(2 * segments + 5);
            index_info[2 * segments * 3 + 8 * 3 + 2] = (uint16_t)(2 * segments + 1);
            index_info[2 * segments * 3 + 9 * 3 + 0] = (uint16_t)(2 * segments + 1);
            index_info[2 * segments * 3 + 9 * 3 + 1] = (uint16_t)(2 * segments + 0);
            index_info[2 * segments * 3 + 9 * 3 + 2] = (uint16_t)(2 * segments + 4);

            index_info[2 * segments * 3 + 10 * 3 + 0] = (uint16_t)(2 * segments + 2);
            index_info[2 * segments * 3 + 10 * 3 + 1] = (uint16_t)(2 * segments + 3);
            index_info[2 * segments * 3 + 10 * 3 + 2] = (uint16_t)(2 * segments + 7);
            index_info[2 * segments * 3 + 11 * 3 + 0] = (uint16_t)(2 * segments + 7);
            index_info[2 * segments * 3 + 11 * 3 + 1] = (uint16_t)(2 * segments + 6);
            index_info[2 * segments * 3 + 11 * 3 + 2] = (uint16_t)(2 * segments + 2);
        }

        for (uint16_t i = 0; i < (2 * segments + 12) * 3; ++i)
        {
            index_info[(2 * segments + 12) * 3 * 1 + i] = (uint16_t)((2 * segments + 8) * 1 + index_info[i]);
            index_info[(2 * segments + 12) * 3 * 2 + i] = (uint16_t)((2 * segments + 8) * 2 + index_info[i]);
        }

        int start_index                     = ((2 * segments + 12) * 3) * 3;
        index_info[start_index + 0 * 3 + 0] = (uint16_t)(start_vertex_index + 0);
        index_info[start_index + 0 * 3 + 1] = (uint16_t)(start_vertex_index + 1);
        index_info[start_index + 0 * 3 + 2] = (uint16_t)(start_vertex_index + 2);
        index_info[start_index + 1 * 3 + 0] = (uint16_t)(start_vertex_index + 0);
        index_info[start_index + 1 * 3 + 1] = (uint16_t)(start_vertex_index + 2);
        index_info[start_index + 1 * 3 + 2] = (uint16_t)(start_vertex_index + 3);

        index_info[start_index + 2 * 3 + 0] = (uint16_t)(start_vertex_index + 4);
        index_info[start_index + 2 * 3 + 1] = (uint16_t)(start_vertex_index + 5);
        index_info[start_index + 2 * 3 + 2] = (uint16_t)(start_vertex_index + 6);
        index_info[start_index + 3 * 3 + 0] = (uint16_t)(start_vertex_index + 4);
        index_info[start_index + 3 * 3 + 1] = (uint16_t)(start_vertex_index + 6);
        index_info[start_index + 3 * 3 + 2] = (uint16_t)(start_vertex_index + 7);

        index_info[start_index + 4 * 3 + 0] = (uint16_t)(start_vertex_index + 0);
        index_info[start_index + 4 * 3 + 1] = (uint16_t)(start_vertex_index + 1);
        index_info[start_index + 4 * 3 + 2] = (uint16_t)(start_vertex_index + 5);
        index_info[start_index + 5 * 3 + 0] = (uint16_t)(start_vertex_index + 0);
        index_info[start_index + 5 * 3 + 1] = (uint16_t)(start_vertex_index + 5);
        index_info[start_index + 5 * 3 + 2] = (uint16_t)(start_vertex_index + 4);

        index_info[start_index + 6 * 3 + 0] = (uint16_t)(start_vertex_index + 3);
        index_info[start_index + 6 * 3 + 1] = (uint16_t)(start_vertex_index + 2);
        index_info[start_index + 6 * 3 + 2] = (uint16_t)(start_vertex_index + 6);
        index_info[start_index + 7 * 3 + 0] = (uint16_t)(start_vertex_index + 3);
        index_info[start_index + 7 * 3 + 1] = (uint16_t)(start_vertex_index + 6);
        index_info[start_index + 7 * 3 + 2] = (uint16_t)(start_vertex_index + 7);

        index_info[start_index + 8 * 3 + 0] = (uint16_t)(start_vertex_index + 4);
        index_info[start_index + 8 * 3 + 1] = (uint16_t)(start_vertex_index + 5);
        index_info[start_index + 8 * 3 + 2] = (uint16_t)(start_vertex_index + 1);
        index_info[start_index + 9 * 3 + 0] = (uint16_t)(start_vertex_index + 1);
        index_info[start_index + 9 * 3 + 1] = (uint16_t)(start_vertex_index + 0);
        index_info[start_index + 9 * 3 + 2] = (uint16_t)(start_vertex_index + 4);

        index_info[start_index + 10 * 3 + 0] = (uint16_t)(start_vertex_index + 2);
        index_info[start_index + 10 * 3 + 1] = (uint16_t)(start_vertex_index + 3);
        index_info[start_index + 10 * 3 + 2] = (uint16_t)(start_vertex_index + 7);
        index_info[start_index + 11 * 3 + 0] = (uint16_t)(start_vertex_index + 7);
        index_info[start_index + 11 * 3 + 1] = (uint16_t)(start_vertex_index + 6);
        index_info[start_index + 11 * 3 + 2] = (uint16_t)(start_vertex_index + 2);

        m_indexBuffer = SceneBuffers::createIndexBuffer(indexMem);
    }

} // namespace Pilot
