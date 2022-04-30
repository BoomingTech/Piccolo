#version 310 es

#extension GL_GOOGLE_include_directive : enable

#include "constants.h"
#include "gbuffer.h"

struct DirectionalLight
{
    highp vec3 direction;
    lowp float _padding_direction;
    highp vec3 color;
    lowp float _padding_color;
};

struct PointLight
{
    highp vec3  position;
    highp float radius;
    highp vec3  intensity;
    lowp float  _padding_intensity;
};

layout(set = 0, binding = 0) readonly buffer _mesh_per_frame
{
    highp mat4       proj_view_matrix;
    highp vec3       camera_position;
    lowp float       _padding_camera_position;
    highp vec3       ambient_light;
    lowp float       _padding_ambient_light;
    highp uint       point_light_num;
    uint             _padding_point_light_num_1;
    uint             _padding_point_light_num_2;
    uint             _padding_point_light_num_3;
    PointLight       scene_point_lights[m_max_point_light_count];
    DirectionalLight scene_directional_light;
    highp mat4       directional_light_proj_view;
};

layout(set = 0, binding = 3) uniform sampler2D brdfLUT_sampler;
layout(set = 0, binding = 4) uniform samplerCube irradiance_sampler;
layout(set = 0, binding = 5) uniform samplerCube specular_sampler;
layout(set = 0, binding = 6) uniform highp sampler2DArray point_lights_shadow;
layout(set = 0, binding = 7) uniform highp sampler2D directional_light_shadow;

layout(input_attachment_index = 0, set = 1, binding = 0) uniform highp subpassInput in_gbuffer_a;
layout(input_attachment_index = 1, set = 1, binding = 1) uniform highp subpassInput in_gbuffer_b;
layout(input_attachment_index = 2, set = 1, binding = 2) uniform highp subpassInput in_gbuffer_c;
layout(input_attachment_index = 3, set = 1, binding = 3) uniform highp subpassInput in_scene_depth;

layout(set = 2, binding = 1) uniform samplerCube skybox_sampler;

layout(location = 0) in highp vec2 in_texcoord;
layout(location = 0) out highp vec4 out_color;

#include "mesh_lighting.h"

void main()
{
    PGBufferData gbuffer;
    highp vec4   gbuffer_a = subpassLoad(in_gbuffer_a).rgba;
    highp vec4   gbuffer_b = subpassLoad(in_gbuffer_b).rgba;
    highp vec4   gbuffer_c = subpassLoad(in_gbuffer_c).rgba;
    DecodeGBufferData(gbuffer, gbuffer_a, gbuffer_b, gbuffer_c);

    highp vec3  N                   = gbuffer.worldNormal;
    highp vec3  basecolor           = gbuffer.baseColor;
    highp float metallic            = gbuffer.metallic;
    highp float dielectric_specular = 0.08 * gbuffer.specular;
    highp float roughness           = gbuffer.roughness;

    highp vec3 in_world_position;
    {
        highp float scene_depth              = subpassLoad(in_scene_depth).r;
        highp vec4  ndc                      = vec4(uv_to_ndcxy(in_texcoord), scene_depth, 1.0);
        highp mat4  inverse_proj_view_matrix = inverse(proj_view_matrix);
        highp vec4  in_world_position_with_w = inverse_proj_view_matrix * ndc;
        in_world_position                    = in_world_position_with_w.xyz / in_world_position_with_w.www;
    }

    highp vec3 result_color = vec3(0.0, 0.0, 0.0);

    if (SHADINGMODELID_UNLIT == gbuffer.shadingModelID)
    {
        // skybox
        highp vec3 in_UVW            = normalize(in_world_position - camera_position);
        highp vec3 origin_sample_UVW = vec3(in_UVW.x, in_UVW.z, in_UVW.y);

        result_color = textureLod(skybox_sampler, origin_sample_UVW, 0.0).rgb;
    }
    else if (SHADINGMODELID_DEFAULT_LIT == gbuffer.shadingModelID)
    {
#include "mesh_lighting.inl"
    }

    out_color = vec4(result_color, 1.0f);
}
