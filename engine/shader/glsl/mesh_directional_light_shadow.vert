#version 310 es

#extension GL_GOOGLE_include_directive : enable

#include "constants.h"
#include "structures.h"

layout(set = 0, binding = 0) readonly buffer _unused_name_global_set_per_frame_binding_buffer
{
    mat4 light_proj_view;
};

layout(set = 0, binding = 1) readonly buffer _unused_name_per_drawcall
{
    VulkanMeshInstance mesh_instances[m_mesh_per_drawcall_max_instance_count];
};

layout(set = 0, binding = 2) readonly buffer _unused_name_per_drawcall_vertex_blending
{
    mat4 joint_matrices[m_mesh_vertex_blending_max_joint_count * m_mesh_per_drawcall_max_instance_count];
};

layout(set = 1, binding = 0) readonly buffer _unused_name_per_mesh_joint_binding
{
    VulkanMeshVertexJointBinding indices_and_weights[];
};

layout(location = 0) in highp vec3 in_position;

void main()
{
    highp mat4 model_matrix = mesh_instances[gl_InstanceIndex].model_matrix;
    highp float enable_vertex_blending = mesh_instances[gl_InstanceIndex].enable_vertex_blending;

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

    highp vec3 position_world_space = (model_matrix * vec4(model_position, 1.0)).xyz;

    gl_Position = light_proj_view * vec4(position_world_space, 1.0f);
}
