#include "editor/include/axis.h"

namespace Piccolo
{
    EditorTranslationAxis::EditorTranslationAxis()
    {
        // create translation axis render mesh

        const float radius = 0.031f;
        const int   segments = 12;

        uint32_t stride = sizeof(MeshVertexDataDefinition);

        // vertex
        size_t vertex_data_size = (3 * segments + 2) * 3 * stride;
        m_mesh_data.m_static_mesh_data.m_vertex_buffer = std::make_shared<BufferData>(vertex_data_size);
        uint8_t* vertex_data = static_cast<uint8_t*>(m_mesh_data.m_static_mesh_data.m_vertex_buffer->m_data);

        // x
        for (int i = 0; i < segments; ++i)
        {
            MeshVertexDataDefinition& vertex =
                *(MeshVertexDataDefinition*)(vertex_data + (0 * segments + i) * stride);
            vertex.x = 0.0f;
            vertex.y = sin(i * 2 * Math_PI / segments) * radius;
            vertex.z = cos(i * 2 * Math_PI / segments) * radius;
            vertex.u = 0.0f;

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v = 0.0f;
        }

        for (int i = 0; i < segments; ++i)
        {
            MeshVertexDataDefinition& vertex =
                *(MeshVertexDataDefinition*)(vertex_data + (1 * segments + i) * stride);
            vertex.x = 1.5f;
            vertex.y =
                (*(MeshVertexDataDefinition*)(vertex_data + (0 * segments + i) * stride)).y;
            vertex.z =
                (*(MeshVertexDataDefinition*)(vertex_data + (0 * segments + i) * stride)).z;
            vertex.u = 0.0f;

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v = 0.0f;
        }

        for (int i = 0; i < segments; ++i)
        {
            MeshVertexDataDefinition& vertex =
                *(MeshVertexDataDefinition*)(vertex_data + (2 * segments + i) * stride);
            vertex.x = 1.5f;
            vertex.y =
                (*(MeshVertexDataDefinition*)(vertex_data + (0 * segments + i) * stride)).y *
                4.5f;
            vertex.z =
                (*(MeshVertexDataDefinition*)(vertex_data + (0 * segments + i) * stride)).z *
                4.5f;
            vertex.u = 0.0f;

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v = 0.0f;
        }
        {
            MeshVertexDataDefinition& vertex_0 =
                *(MeshVertexDataDefinition*)(vertex_data + (3 * segments + 0) * stride);
            vertex_0.x = 1.5f;
            vertex_0.y = 0.0f;
            vertex_0.z = 0.0f;
            vertex_0.u = 0.0f;

            vertex_0.nx = vertex_0.ny = vertex_0.nz = 0.0f;
            vertex_0.tx = vertex_0.ty = vertex_0.tz = 0.0f;
            vertex_0.v = 0.0f;

            MeshVertexDataDefinition& vertex_1 =
                *(MeshVertexDataDefinition*)(vertex_data + (3 * segments + 1) * stride);
            vertex_1.x = 1.9f;
            vertex_1.y = 0.0f;
            vertex_1.z = 0.0f;
            vertex_1.u = 0.0f;

            vertex_1.nx = vertex_1.ny = vertex_1.nz = 0.0f;
            vertex_1.tx = vertex_1.ty = vertex_1.tz = 0.0f;
            vertex_1.v = 0.0f;
        }

        // y, z
        for (int i = 0; i < 3 * segments + 2; ++i)
        {
            MeshVertexDataDefinition& vertex_y =
                *(MeshVertexDataDefinition*)(vertex_data +
                    ((3 * segments + 2) * 1 + i) * stride);
            vertex_y.x = -(*(MeshVertexDataDefinition*)(vertex_data + i * stride)).y;
            vertex_y.y = (*(MeshVertexDataDefinition*)(vertex_data + i * stride)).x;
            vertex_y.z = (*(MeshVertexDataDefinition*)(vertex_data + i * stride)).z;
            vertex_y.u = 1.0f;

            vertex_y.nx = vertex_y.ny = vertex_y.nz = 0.0f;
            vertex_y.tx = vertex_y.ty = vertex_y.tz = 0.0f;
            vertex_y.v = 0.0f;

            MeshVertexDataDefinition& vertex_z =
                *(MeshVertexDataDefinition*)(vertex_data +
                    ((3 * segments + 2) * 2 + i) * stride);
            vertex_z.x = -(*(MeshVertexDataDefinition*)(vertex_data + i * stride)).z;
            vertex_z.y = (*(MeshVertexDataDefinition*)(vertex_data + i * stride)).y;
            vertex_z.z = (*(MeshVertexDataDefinition*)(vertex_data + i * stride)).x;
            vertex_z.u = 2.0f;

            vertex_z.nx = vertex_z.ny = vertex_z.nz = 0.0f;
            vertex_z.tx = vertex_z.ty = vertex_z.tz = 0.0f;
            vertex_z.v = 0.0f;
        }

        size_t index_data_size                        = (4 * segments * 3) * 3 * sizeof(uint16_t);
        m_mesh_data.m_static_mesh_data.m_index_buffer = std::make_shared<BufferData>(index_data_size);
        uint16_t* index_data = static_cast<uint16_t*>(m_mesh_data.m_static_mesh_data.m_index_buffer->m_data);

        for (int i = 0; i < segments; ++i)
        {
            index_data[0 * segments * 3 + i * 6 + 0] = (uint16_t)(0 * segments + i);
            index_data[0 * segments * 3 + i * 6 + 1] = (uint16_t)1 * segments + i;
            index_data[0 * segments * 3 + i * 6 + 2] = (uint16_t)1 * segments + ((i + 1) % segments);

            index_data[0 * segments * 3 + i * 6 + 3] = (uint16_t)1 * segments + ((i + 1) % segments);
            index_data[0 * segments * 3 + i * 6 + 4] = (uint16_t)0 * segments + ((i + 1) % segments);
            index_data[0 * segments * 3 + i * 6 + 5] = (uint16_t)0 * segments + i;
        }
        for (int i = 0; i < segments; ++i)
        {
            index_data[2 * segments * 3 + i * 3 + 0] = (uint16_t)3 * segments + 0;
            index_data[2 * segments * 3 + i * 3 + 1] = (uint16_t)2 * segments + i;
            index_data[2 * segments * 3 + i * 3 + 2] = (uint16_t)2 * segments + ((i + 1) % segments);
        }
        for (int i = 0; i < segments; ++i)
        {
            index_data[3 * segments * 3 + i * 3 + 0] = (uint16_t)2 * segments + i;
            index_data[3 * segments * 3 + i * 3 + 1] = (uint16_t)3 * segments + 1;
            index_data[3 * segments * 3 + i * 3 + 2] = (uint16_t)2 * segments + ((i + 1) % segments);
        }

        for (int i = 0; i < 4 * segments * 3; ++i)
        {
            index_data[4 * segments * 3 * 1 + i] = (uint16_t)((3 * segments + 2) * 1) + index_data[i];
            index_data[4 * segments * 3 * 2 + i] = (uint16_t)((3 * segments + 2) * 2) + index_data[i];
        }
    }

    EditorRotationAxis::EditorRotationAxis()
    {
        // create rotation axis render mesh

        const float inner_radius = 0.9f;
        const float outer_radius = 1.0f;
        const int   segments = 24;

        uint32_t stride = sizeof(MeshVertexDataDefinition);

        // vertex
        size_t vertex_data_size = 2 * 3 * segments * stride;
        m_mesh_data.m_static_mesh_data.m_vertex_buffer = std::make_shared<BufferData>(vertex_data_size);
        uint8_t* vertex_data = static_cast<uint8_t*>(m_mesh_data.m_static_mesh_data.m_vertex_buffer->m_data);

        // inner xy
        for (int i = 0; i < segments; i++)
        {
            MeshVertexDataDefinition& vertex =
                *(MeshVertexDataDefinition*)(vertex_data + (0 * segments + i) * stride);
            vertex.x = cos(2 * Math_PI / segments * i) * inner_radius;
            vertex.y = sin(2 * Math_PI / segments * i) * inner_radius;
            vertex.z = 0.0f;
            vertex.u = 2.0f;

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v = 0.0f;
        }
        // outer xy
        for (int i = 0; i < segments; i++)
        {
            MeshVertexDataDefinition& vertex =
                *(MeshVertexDataDefinition*)(vertex_data + (1 * segments + i) * stride);
            vertex.x = cos(2 * Math_PI / segments * i) * outer_radius;
            vertex.y = sin(2 * Math_PI / segments * i) * outer_radius;
            vertex.z = 0.0f;
            vertex.u = 2.0f;

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v = 0.0f;
        }
        // inner yz
        for (int i = 0; i < segments; i++)
        {
            MeshVertexDataDefinition& vertex =
                *(MeshVertexDataDefinition*)(vertex_data + (2 * segments + i) * stride);
            vertex.x = 0.0f;
            vertex.y = cos(2 * Math_PI / segments * i) * inner_radius;
            vertex.z = sin(2 * Math_PI / segments * i) * inner_radius;
            vertex.u = 0.0f;

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v = 0.0f;
        }
        // outer yz
        for (int i = 0; i < segments; i++)
        {
            MeshVertexDataDefinition& vertex =
                *(MeshVertexDataDefinition*)(vertex_data + (3 * segments + i) * stride);
            vertex.x = 0.0f;
            vertex.y = cos(2 * Math_PI / segments * i) * outer_radius;
            vertex.z = sin(2 * Math_PI / segments * i) * outer_radius;
            vertex.u = 0.0f;

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v = 0.0f;
        }
        // inner xz
        for (int i = 0; i < segments; i++)
        {
            MeshVertexDataDefinition& vertex =
                *(MeshVertexDataDefinition*)(vertex_data + (4 * segments + i) * stride);
            vertex.x = cos(2 * Math_PI / segments * i) * inner_radius;
            vertex.y = 0.0f;
            vertex.z = sin(2 * Math_PI / segments * i) * inner_radius;
            vertex.u = 1.0f;

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v = 0.0f;
        }
        // outer xz
        for (int i = 0; i < segments; i++)
        {
            MeshVertexDataDefinition& vertex =
                *(MeshVertexDataDefinition*)(vertex_data + (5 * segments + i) * stride);
            vertex.x = cos(2 * Math_PI / segments * i) * outer_radius;
            vertex.y = 0.0f;
            vertex.z = sin(2 * Math_PI / segments * i) * outer_radius;
            vertex.u = 1.0f;

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v = 0.0f;
        }

        // index
        size_t index_data_size = 2 * 3 * segments * 3 * sizeof(uint16_t);
        m_mesh_data.m_static_mesh_data.m_index_buffer = std::make_shared<BufferData>(index_data_size);
        uint16_t* index_data = static_cast<uint16_t*>(m_mesh_data.m_static_mesh_data.m_index_buffer->m_data);

        // xoy inner
        for (int i = 0; i < segments; i++)
        {
            index_data[(3 * i) + 0] = (uint16_t)(i % segments);
            index_data[(3 * i) + 1] = (uint16_t)((i + 1) % segments);
            index_data[(3 * i) + 2] = (uint16_t)(i % segments + segments);
        }
        // xoy outer
        for (int i = 0; i < segments; i++)
        {
            index_data[1 * 3 * segments + (3 * i) + 0] = (uint16_t)(i % segments + segments);
            index_data[1 * 3 * segments + (3 * i) + 1] = (uint16_t)((i + 1) % segments + segments);
            index_data[1 * 3 * segments + (3 * i) + 2] = (uint16_t)((i + 1) % segments);
        }
        // yoz inner
        for (int i = 0; i < segments; i++)
        {
            index_data[2 * 3 * segments + (3 * i) + 0] = (uint16_t)(i % segments + segments * 2);
            index_data[2 * 3 * segments + (3 * i) + 1] = (uint16_t)((i + 1) % segments + segments * 2);
            index_data[2 * 3 * segments + (3 * i) + 2] = (uint16_t)(i % segments + segments * 3);
        }
        // yoz outer
        for (int i = 0; i < segments; i++)
        {
            index_data[3 * 3 * segments + (3 * i) + 0] = (uint16_t)(i % segments + segments * 3);
            index_data[3 * 3 * segments + (3 * i) + 1] = (uint16_t)((i + 1) % segments + segments * 3);
            index_data[3 * 3 * segments + (3 * i) + 2] = (uint16_t)((i + 1) % segments + segments * 2);
        }
        // xoz inner
        for (int i = 0; i < segments; i++)
        {
            index_data[4 * 3 * segments + (3 * i) + 0] = (uint16_t)(i % segments + segments * 4);
            index_data[4 * 3 * segments + (3 * i) + 1] = (uint16_t)((i + 1) % segments + segments * 4);
            index_data[4 * 3 * segments + (3 * i) + 2] = (uint16_t)(i % segments + segments * 5);
        }
        // xoz outer
        for (int i = 0; i < segments; i++)
        {
            index_data[5 * 3 * segments + (3 * i) + 0] = (uint16_t)(i % segments + segments * 5);
            index_data[5 * 3 * segments + (3 * i) + 1] = (uint16_t)((i + 1) % segments + segments * 5);
            index_data[5 * 3 * segments + (3 * i) + 2] = (uint16_t)((i + 1) % segments + segments * 4);
        }
    }

    EditorScaleAxis::EditorScaleAxis()
    {
        const float radius = 0.031f;
        const int   segments = 12;

        uint32_t stride = sizeof(MeshVertexDataDefinition);

        // vertex
        size_t vertex_data_size = ((2 * segments + 8) * 3 + 8) * stride;
        m_mesh_data.m_static_mesh_data.m_vertex_buffer = std::make_shared<BufferData>(vertex_data_size);
        uint8_t* vertex_data = static_cast<uint8_t*>(m_mesh_data.m_static_mesh_data.m_vertex_buffer->m_data);

        for (int i = 0; i < segments; ++i)
        {
            MeshVertexDataDefinition& vertex =
                *(MeshVertexDataDefinition*)(vertex_data + (0 * segments + i) * stride);
            vertex.x = 0.0f;
            vertex.y = sin(i * 2 * Math_PI / segments) * radius;
            vertex.z = cos(i * 2 * Math_PI / segments) * radius;
            vertex.u = 0.0f;

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v = 0.0f;
        }
        for (int i = 0; i < segments; ++i)
        {
            MeshVertexDataDefinition& vertex =
                *(MeshVertexDataDefinition*)(vertex_data + (1 * segments + i) * stride);
            vertex.x = 1.6 - radius * 10;
            vertex.y = (*(MeshVertexDataDefinition*)(vertex_data + (0 * segments + i) * stride)).y;
            vertex.z = (*(MeshVertexDataDefinition*)(vertex_data + (0 * segments + i) * stride)).z;
            vertex.u = 0.0f;

            vertex.nx = vertex.ny = vertex.nz = 0.0f;
            vertex.tx = vertex.ty = vertex.tz = 0.0f;
            vertex.v = 0.0f;
        }
        {
            MeshVertexDataDefinition& vertex0 =
                *(MeshVertexDataDefinition*)(vertex_data + (2 * segments + 0) * stride);
            vertex0.x = 1.6 - radius * 10;
            vertex0.y = +radius * 5;
            vertex0.z = +radius * 5;
            vertex0.u = 0.0f;

            vertex0.nx = vertex0.ny = vertex0.nz = 0.0f;
            vertex0.tx = vertex0.ty = vertex0.tz = 0.0f;
            vertex0.v = 0.0f;

            MeshVertexDataDefinition& vertex1 =
                *(MeshVertexDataDefinition*)(vertex_data + (2 * segments + 1) * stride);
            vertex1.x = 1.6 - radius * 10;
            vertex1.y = +radius * 5;
            vertex1.z = -radius * 5;
            vertex1.u = 0.0f;

            vertex1.nx = vertex1.ny = vertex1.nz = 0.0f;
            vertex1.tx = vertex1.ty = vertex1.tz = 0.0f;
            vertex1.v = 0.0f;

            MeshVertexDataDefinition& vertex2 =
                *(MeshVertexDataDefinition*)(vertex_data + (2 * segments + 2) * stride);
            vertex2.x = 1.6 - radius * 10;
            vertex2.y = -radius * 5;
            vertex2.z = +radius * 5;
            vertex2.u = 0.0f;

            vertex2.nx = vertex2.ny = vertex2.nz = 0.0f;
            vertex2.tx = vertex2.ty = vertex2.tz = 0.0f;
            vertex2.v = 0.0f;

            MeshVertexDataDefinition& vertex3 =
                *(MeshVertexDataDefinition*)(vertex_data + (2 * segments + 3) * stride);
            vertex3.x = 1.6 - radius * 10;
            vertex3.y = -radius * 5;
            vertex3.z = -radius * 5;
            vertex3.u = 0.0f;

            vertex3.nx = vertex3.ny = vertex3.nz = 0.0f;
            vertex3.tx = vertex3.ty = vertex3.tz = 0.0f;
            vertex3.v = 0.0f;

            MeshVertexDataDefinition& vertex4 =
                *(MeshVertexDataDefinition*)(vertex_data + (2 * segments + 4) * stride);
            vertex4.x = 1.6;
            vertex4.y = +radius * 5;
            vertex4.z = +radius * 5;
            vertex4.u = 0.0f;

            vertex4.nx = vertex4.ny = vertex4.nz = 0.0f;
            vertex4.tx = vertex4.ty = vertex4.tz = 0.0f;
            vertex4.v = 0.0f;

            MeshVertexDataDefinition& vertex5 =
                *(MeshVertexDataDefinition*)(vertex_data + (2 * segments + 5) * stride);
            vertex5.x = 1.6;
            vertex5.y = +radius * 5;
            vertex5.z = -radius * 5;
            vertex5.u = 0.0f;

            vertex5.nx = vertex5.ny = vertex5.nz = 0.0f;
            vertex5.tx = vertex5.ty = vertex5.tz = 0.0f;
            vertex5.v = 0.0f;

            MeshVertexDataDefinition& vertex6 =
                *(MeshVertexDataDefinition*)(vertex_data + (2 * segments + 6) * stride);
            vertex6.x = 1.6;
            vertex6.y = -radius * 5;
            vertex6.z = +radius * 5;
            vertex6.u = 0.0f;

            vertex6.nx = vertex6.ny = vertex6.nz = 0.0f;
            vertex6.tx = vertex6.ty = vertex6.tz = 0.0f;
            vertex6.v = 0.0f;

            MeshVertexDataDefinition& vertex7 =
                *(MeshVertexDataDefinition*)(vertex_data + (2 * segments + 7) * stride);
            vertex7.x = 1.6;
            vertex7.y = -radius * 5;
            vertex7.z = -radius * 5;
            vertex7.u = 0.0f;

            vertex7.nx = vertex7.ny = vertex7.nz = 0.0f;
            vertex7.tx = vertex7.ty = vertex7.tz = 0.0f;
            vertex7.v = 0.0f;
        }

        for (int i = 0; i < 2 * segments + 8; ++i)
        {
            MeshVertexDataDefinition& vertex1 =
                *(MeshVertexDataDefinition*)(vertex_data + ((2 * segments + 8) * 1 + i) * stride);
            vertex1.x = -(*(MeshVertexDataDefinition*)(vertex_data + i * stride)).y;
            vertex1.y = (*(MeshVertexDataDefinition*)(vertex_data + i * stride)).x;
            vertex1.z = (*(MeshVertexDataDefinition*)(vertex_data + i * stride)).z;
            vertex1.u = 1.0f;

            vertex1.nx = vertex1.ny = vertex1.nz = 0.0f;
            vertex1.tx = vertex1.ty = vertex1.tz = 0.0f;
            vertex1.v = 0.0f;

            MeshVertexDataDefinition& vertex2 =
                *(MeshVertexDataDefinition*)(vertex_data + ((2 * segments + 8) * 2 + i) * stride);
            vertex2.x = -(*(MeshVertexDataDefinition*)(vertex_data + i * stride)).z;
            vertex2.y = (*(MeshVertexDataDefinition*)(vertex_data + i * stride)).y;
            vertex2.z = (*(MeshVertexDataDefinition*)(vertex_data + i * stride)).x;
            vertex2.u = 2.0f;

            vertex2.nx = vertex2.ny = vertex2.nz = 0.0f;
            vertex2.tx = vertex2.ty = vertex2.tz = 0.0f;
            vertex2.v = 0.0f;
        }

        int start_vertex_index = (2 * segments + 8) * 3;
        {
            MeshVertexDataDefinition& vertex0 =
                *(MeshVertexDataDefinition*)(vertex_data + (start_vertex_index + 0) * stride);
            vertex0.x = 0.0f;
            vertex0.y = 0.0f;
            vertex0.z = 0.0f;
            vertex0.u = 6.0f;

            vertex0.nx = vertex0.ny = vertex0.nz = 0.0f;
            vertex0.tx = vertex0.ty = vertex0.tz = 0.0f;
            vertex0.v = 0.0f;

            MeshVertexDataDefinition& vertex1 =
                *(MeshVertexDataDefinition*)(vertex_data + (start_vertex_index + 1) * stride);
            vertex1.x = 0.1f;
            vertex1.y = 0.0f;
            vertex1.z = 0.0f;
            vertex1.u = 6.0f;

            vertex1.nx = vertex1.ny = vertex1.nz = 0.0f;
            vertex1.tx = vertex1.ty = vertex1.tz = 0.0f;
            vertex1.v = 0.0f;

            MeshVertexDataDefinition& vertex2 =
                *(MeshVertexDataDefinition*)(vertex_data + (start_vertex_index + 2) * stride);
            vertex2.x = 0.1f;
            vertex2.y = 0.1f;
            vertex2.z = 0.0f;
            vertex2.u = 6.0f;

            vertex2.nx = vertex2.ny = vertex2.nz = 0.0f;
            vertex2.tx = vertex2.ty = vertex2.tz = 0.0f;
            vertex2.v = 0.0f;

            MeshVertexDataDefinition& vertex3 =
                *(MeshVertexDataDefinition*)(vertex_data + (start_vertex_index + 3) * stride);
            vertex3.x = 0.0f;
            vertex3.y = 0.1f;
            vertex3.z = 0.0f;
            vertex3.u = 6.0f;

            vertex3.nx = vertex3.ny = vertex3.nz = 0.0f;
            vertex3.tx = vertex3.ty = vertex3.tz = 0.0f;
            vertex3.v = 0.0f;

            MeshVertexDataDefinition& vertex4 =
                *(MeshVertexDataDefinition*)(vertex_data + (start_vertex_index + 4) * stride);
            vertex4.x = 0.0f;
            vertex4.y = 0.0f;
            vertex4.z = 0.1f;
            vertex4.u = 6.0f;

            vertex4.nx = vertex4.ny = vertex4.nz = 0.0f;
            vertex4.tx = vertex4.ty = vertex4.tz = 0.0f;
            vertex4.v = 0.0f;

            MeshVertexDataDefinition& vertex5 =
                *(MeshVertexDataDefinition*)(vertex_data + (start_vertex_index + 5) * stride);
            vertex5.x = 0.1f;
            vertex5.y = 0.0f;
            vertex5.z = 0.1f;
            vertex5.u = 6.0f;

            vertex5.nx = vertex5.ny = vertex5.nz = 0.0f;
            vertex5.tx = vertex5.ty = vertex5.tz = 0.0f;
            vertex5.v = 0.0f;

            MeshVertexDataDefinition& vertex6 =
                *(MeshVertexDataDefinition*)(vertex_data + (start_vertex_index + 6) * stride);
            vertex6.x = 0.1f;
            vertex6.y = 0.1f;
            vertex6.z = 0.1f;
            vertex6.u = 6.0f;

            vertex6.nx = vertex6.ny = vertex6.nz = 0.0f;
            vertex6.tx = vertex6.ty = vertex6.tz = 0.0f;
            vertex6.v = 0.0f;

            MeshVertexDataDefinition& vertex7 =
                *(MeshVertexDataDefinition*)(vertex_data + (start_vertex_index + 7) * stride);
            vertex7.x = 0.0f;
            vertex7.y = 0.1f;
            vertex7.z = 0.1f;
            vertex7.u = 6.0f;

            vertex7.nx = vertex7.ny = vertex7.nz = 0.0f;
            vertex7.tx = vertex7.ty = vertex7.tz = 0.0f;
            vertex7.v = 0.0f;
        }

        // index
        size_t index_data_size = (((2 * segments + 12) * 3) * 3 + 3 * 2 * 6) * sizeof(uint16_t);
        m_mesh_data.m_static_mesh_data.m_index_buffer = std::make_shared<BufferData>(index_data_size);
        uint16_t* index_data = static_cast<uint16_t*>(m_mesh_data.m_static_mesh_data.m_index_buffer->m_data);

        for (int i = 0; i < segments; ++i)
        {
            index_data[0 * segments * 3 + i * 6 + 0] = (uint16_t)(0 * segments + i);
            index_data[0 * segments * 3 + i * 6 + 1] = (uint16_t)(1 * segments + i);
            index_data[0 * segments * 3 + i * 6 + 2] = (uint16_t)(1 * segments + ((i + 1) % segments));

            index_data[0 * segments * 3 + i * 6 + 3] = (uint16_t)(1 * segments + ((i + 1) % segments));
            index_data[0 * segments * 3 + i * 6 + 4] = (uint16_t)(0 * segments + ((i + 1) % segments));
            index_data[0 * segments * 3 + i * 6 + 5] = (uint16_t)(0 * segments + i);
        }
        {
            index_data[2 * segments * 3 + 0 * 3 + 0] = (uint16_t)(2 * segments + 0);
            index_data[2 * segments * 3 + 0 * 3 + 1] = (uint16_t)(2 * segments + 1);
            index_data[2 * segments * 3 + 0 * 3 + 2] = (uint16_t)(2 * segments + 3);
            index_data[2 * segments * 3 + 1 * 3 + 0] = (uint16_t)(2 * segments + 3);
            index_data[2 * segments * 3 + 1 * 3 + 1] = (uint16_t)(2 * segments + 2);
            index_data[2 * segments * 3 + 1 * 3 + 2] = (uint16_t)(2 * segments + 0);

            index_data[2 * segments * 3 + 2 * 3 + 0] = (uint16_t)(2 * segments + 1);
            index_data[2 * segments * 3 + 2 * 3 + 1] = (uint16_t)(2 * segments + 5);
            index_data[2 * segments * 3 + 2 * 3 + 2] = (uint16_t)(2 * segments + 7);
            index_data[2 * segments * 3 + 3 * 3 + 0] = (uint16_t)(2 * segments + 7);
            index_data[2 * segments * 3 + 3 * 3 + 1] = (uint16_t)(2 * segments + 3);
            index_data[2 * segments * 3 + 3 * 3 + 2] = (uint16_t)(2 * segments + 1);

            index_data[2 * segments * 3 + 4 * 3 + 0] = (uint16_t)(2 * segments + 5);
            index_data[2 * segments * 3 + 4 * 3 + 1] = (uint16_t)(2 * segments + 4);
            index_data[2 * segments * 3 + 4 * 3 + 2] = (uint16_t)(2 * segments + 6);
            index_data[2 * segments * 3 + 5 * 3 + 0] = (uint16_t)(2 * segments + 6);
            index_data[2 * segments * 3 + 5 * 3 + 1] = (uint16_t)(2 * segments + 7);
            index_data[2 * segments * 3 + 5 * 3 + 2] = (uint16_t)(2 * segments + 5);

            index_data[2 * segments * 3 + 6 * 3 + 0] = (uint16_t)(2 * segments + 4);
            index_data[2 * segments * 3 + 6 * 3 + 1] = (uint16_t)(2 * segments + 0);
            index_data[2 * segments * 3 + 6 * 3 + 2] = (uint16_t)(2 * segments + 2);
            index_data[2 * segments * 3 + 7 * 3 + 0] = (uint16_t)(2 * segments + 2);
            index_data[2 * segments * 3 + 7 * 3 + 1] = (uint16_t)(2 * segments + 6);
            index_data[2 * segments * 3 + 7 * 3 + 2] = (uint16_t)(2 * segments + 4);

            index_data[2 * segments * 3 + 8 * 3 + 0] = (uint16_t)(2 * segments + 4);
            index_data[2 * segments * 3 + 8 * 3 + 1] = (uint16_t)(2 * segments + 5);
            index_data[2 * segments * 3 + 8 * 3 + 2] = (uint16_t)(2 * segments + 1);
            index_data[2 * segments * 3 + 9 * 3 + 0] = (uint16_t)(2 * segments + 1);
            index_data[2 * segments * 3 + 9 * 3 + 1] = (uint16_t)(2 * segments + 0);
            index_data[2 * segments * 3 + 9 * 3 + 2] = (uint16_t)(2 * segments + 4);

            index_data[2 * segments * 3 + 10 * 3 + 0] = (uint16_t)(2 * segments + 2);
            index_data[2 * segments * 3 + 10 * 3 + 1] = (uint16_t)(2 * segments + 3);
            index_data[2 * segments * 3 + 10 * 3 + 2] = (uint16_t)(2 * segments + 7);
            index_data[2 * segments * 3 + 11 * 3 + 0] = (uint16_t)(2 * segments + 7);
            index_data[2 * segments * 3 + 11 * 3 + 1] = (uint16_t)(2 * segments + 6);
            index_data[2 * segments * 3 + 11 * 3 + 2] = (uint16_t)(2 * segments + 2);
        }

        for (uint16_t i = 0; i < (2 * segments + 12) * 3; ++i)
        {
            index_data[(2 * segments + 12) * 3 * 1 + i] = (uint16_t)((2 * segments + 8) * 1 + index_data[i]);
            index_data[(2 * segments + 12) * 3 * 2 + i] = (uint16_t)((2 * segments + 8) * 2 + index_data[i]);
        }

        int start_index = ((2 * segments + 12) * 3) * 3;
        index_data[start_index + 0 * 3 + 0] = (uint16_t)(start_vertex_index + 0);
        index_data[start_index + 0 * 3 + 1] = (uint16_t)(start_vertex_index + 1);
        index_data[start_index + 0 * 3 + 2] = (uint16_t)(start_vertex_index + 2);
        index_data[start_index + 1 * 3 + 0] = (uint16_t)(start_vertex_index + 0);
        index_data[start_index + 1 * 3 + 1] = (uint16_t)(start_vertex_index + 2);
        index_data[start_index + 1 * 3 + 2] = (uint16_t)(start_vertex_index + 3);

        index_data[start_index + 2 * 3 + 0] = (uint16_t)(start_vertex_index + 4);
        index_data[start_index + 2 * 3 + 1] = (uint16_t)(start_vertex_index + 5);
        index_data[start_index + 2 * 3 + 2] = (uint16_t)(start_vertex_index + 6);
        index_data[start_index + 3 * 3 + 0] = (uint16_t)(start_vertex_index + 4);
        index_data[start_index + 3 * 3 + 1] = (uint16_t)(start_vertex_index + 6);
        index_data[start_index + 3 * 3 + 2] = (uint16_t)(start_vertex_index + 7);

        index_data[start_index + 4 * 3 + 0] = (uint16_t)(start_vertex_index + 0);
        index_data[start_index + 4 * 3 + 1] = (uint16_t)(start_vertex_index + 1);
        index_data[start_index + 4 * 3 + 2] = (uint16_t)(start_vertex_index + 5);
        index_data[start_index + 5 * 3 + 0] = (uint16_t)(start_vertex_index + 0);
        index_data[start_index + 5 * 3 + 1] = (uint16_t)(start_vertex_index + 5);
        index_data[start_index + 5 * 3 + 2] = (uint16_t)(start_vertex_index + 4);

        index_data[start_index + 6 * 3 + 0] = (uint16_t)(start_vertex_index + 3);
        index_data[start_index + 6 * 3 + 1] = (uint16_t)(start_vertex_index + 2);
        index_data[start_index + 6 * 3 + 2] = (uint16_t)(start_vertex_index + 6);
        index_data[start_index + 7 * 3 + 0] = (uint16_t)(start_vertex_index + 3);
        index_data[start_index + 7 * 3 + 1] = (uint16_t)(start_vertex_index + 6);
        index_data[start_index + 7 * 3 + 2] = (uint16_t)(start_vertex_index + 7);

        index_data[start_index + 8 * 3 + 0] = (uint16_t)(start_vertex_index + 4);
        index_data[start_index + 8 * 3 + 1] = (uint16_t)(start_vertex_index + 5);
        index_data[start_index + 8 * 3 + 2] = (uint16_t)(start_vertex_index + 1);
        index_data[start_index + 9 * 3 + 0] = (uint16_t)(start_vertex_index + 1);
        index_data[start_index + 9 * 3 + 1] = (uint16_t)(start_vertex_index + 0);
        index_data[start_index + 9 * 3 + 2] = (uint16_t)(start_vertex_index + 4);

        index_data[start_index + 10 * 3 + 0] = (uint16_t)(start_vertex_index + 2);
        index_data[start_index + 10 * 3 + 1] = (uint16_t)(start_vertex_index + 3);
        index_data[start_index + 10 * 3 + 2] = (uint16_t)(start_vertex_index + 7);
        index_data[start_index + 11 * 3 + 0] = (uint16_t)(start_vertex_index + 7);
        index_data[start_index + 11 * 3 + 1] = (uint16_t)(start_vertex_index + 6);
        index_data[start_index + 11 * 3 + 2] = (uint16_t)(start_vertex_index + 2);
    }

} // namespace Piccolo
