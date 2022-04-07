#version 310 es

#extension GL_GOOGLE_include_directive : enable

#include "constants.h"
#include "structures.h"

layout(set = 0, binding = 0) readonly buffer _unused_name_perframe
{
    mat4 proj_view_matrix;
    uint rt_width;
    uint rt_height;
};

layout(set = 0, binding = 1) readonly buffer _unused_name_perdrawcall
{
    mat4 model_matrices[m_mesh_per_drawcall_max_instance_count];
    uint node_ids[m_mesh_per_drawcall_max_instance_count];
    float enable_vertex_blendings[m_mesh_per_drawcall_max_instance_count];
};

layout(set = 0, binding = 2) readonly buffer _unused_name_per_drawcall_vertex_blending
{
    mat4 joint_matrices[m_mesh_vertex_blending_max_joint_count * m_mesh_per_drawcall_max_instance_count];
};

layout(set = 1, binding = 0) readonly buffer _unused_name_per_mesh_joint_binding
{
    VulkanMeshVertexJointBinding indices_and_weights[];
};

layout(location = 0) in vec3 in_position;

layout(location = 0) flat out highp uint out_nodeid;

void main()
{
    highp mat4 model_matrix = model_matrices[gl_InstanceIndex];
    highp float enable_vertex_blending = enable_vertex_blendings[gl_InstanceIndex];

    highp vec3 model_position;
    if (enable_vertex_blending > 0.0)
    {
        highp ivec4 in_indices = indices_and_weights[gl_VertexIndex].indices;
        highp vec4 in_weights = indices_and_weights[gl_VertexIndex].weights;

        highp mat4 vertex_blending_matrix = mat4x4(
            vec4(0.0, 0.0, 0.0, 0.0),
            vec4(0.0, 0.0, 0.0, 0.0),
            vec4(0.0, 0.0, 0.0, 0.0),
            vec4(0.0, 0.0, 0.0, 0.0));

        if (in_weights.x > 0.0 && in_indices.x > 0)
        {
            vertex_blending_matrix += joint_matrices[m_mesh_vertex_blending_max_joint_count * gl_InstanceIndex + in_indices.x] * in_weights.x;
        }

        if (in_weights.y > 0.0 && in_indices.y > 0)
        {
            vertex_blending_matrix += joint_matrices[m_mesh_vertex_blending_max_joint_count * gl_InstanceIndex + in_indices.y] * in_weights.y;
        }

        if (in_weights.z > 0.0 && in_indices.z > 0)
        {
            vertex_blending_matrix += joint_matrices[m_mesh_vertex_blending_max_joint_count * gl_InstanceIndex + in_indices.z] * in_weights.z;
        }

        if (in_weights.w > 0.0 && in_indices.w > 0)
        {
            vertex_blending_matrix += joint_matrices[m_mesh_vertex_blending_max_joint_count * gl_InstanceIndex + in_indices.w] * in_weights.w;
        }

        model_position = (vertex_blending_matrix * vec4(in_position, 1.0)).xyz;
    }
    else
    {
        model_position = in_position;
    }

    gl_Position = proj_view_matrix * model_matrix * vec4(in_position, 1.0);

    out_nodeid = node_ids[gl_InstanceIndex];
}