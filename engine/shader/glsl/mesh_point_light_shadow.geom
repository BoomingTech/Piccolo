#version 310 es

#extension GL_GOOGLE_include_directive : enable

// TODO: geometry shader is inefficient for Mali GPU
#extension GL_EXT_geometry_shader : enable

// Imagination Technologies Limited. "Dual Paraboloid Environment Mapping." Power SDK Whitepaper 2017.
// https://github.com/powervr-graphics/Native_SDK/blob/R17.1-v4.3/Documentation/Whitepapers/Dual%20Paraboloid%20Environment%20Mapping.Whitepaper.pdf

#include "constants.h"

layout(set = 0, binding = 0) readonly buffer _unused_name_global_set_per_frame_binding_buffer
{
    uint point_light_count;
    uint _padding_point_light_count_0;
    uint _padding_point_light_count_1;
    uint _padding_point_light_count_2;
    highp vec4 point_lights_position_and_radius[m_max_point_light_count];
};

layout(triangles) in;
layout(triangle_strip, max_vertices = m_max_point_light_geom_vertices) out;

layout(location = 0) in highp vec3 in_positions_world_space[];

layout(location = 0) out highp float out_inv_length;
layout(location = 1) out highp vec3 out_inv_length_position_view_space;

void main()
{
    for (highp int point_light_index = 0; point_light_index < int(point_light_count) && point_light_index < m_max_point_light_count; ++point_light_index)
    {
        vec3 point_light_position = point_lights_position_and_radius[point_light_index].xyz;
        float point_light_radius = point_lights_position_and_radius[point_light_index].w;

        // TODO: find more effificient ways
        // we draw twice, since the gl_Layer of three vetices may not be the same
        for (highp int layer_index = 0; layer_index < 2; ++layer_index)
        {
            for (highp int vertex_index = 0; vertex_index < 3; ++vertex_index)
            {
                highp vec3 position_world_space = in_positions_world_space[vertex_index];

                // world space to light view space
                // identity rotation
                // Z - Up
                // Y - Forward
                // X - Right
                highp vec3 position_view_space = position_world_space - point_light_position;

                highp vec3 position_spherical_function_domain = normalize(position_view_space);

                // z > 0
                // (x_2d, y_2d, 0) + (0, 0, 1) = λ ((x_sph, y_sph, z_sph) + (0, 0, 1))
                // (x_2d, y_2d) = (x_sph, y_sph) / (z_sph + 1)
                // z < 0
                // (x_2d, y_2d, 0) + (0, 0, -1) = λ ((x_sph, y_sph, z_sph) + (0, 0, -1))
                // (x_2d, y_2d) = (x_sph, y_sph) / (-z_sph + 1)
                highp float layer_position_spherical_function_domain_z[2];
                layer_position_spherical_function_domain_z[0] = -position_spherical_function_domain.z;
                layer_position_spherical_function_domain_z[1] = position_spherical_function_domain.z;
                highp vec4 position_clip;
                position_clip.xy = position_spherical_function_domain.xy;
                position_clip.w = layer_position_spherical_function_domain_z[layer_index] + 1.0;
                position_clip.z = 0.5 * position_clip.w; //length(position_view_space) * position_clip.w / point_light_radius;
                gl_Position = position_clip;

                out_inv_length = 1.0f / length(position_view_space);
                out_inv_length_position_view_space = out_inv_length * position_view_space;

                gl_Layer = layer_index + 2 * point_light_index;
                EmitVertex();
            }
            EndPrimitive();
        }
    }
}
