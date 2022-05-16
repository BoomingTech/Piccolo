#version 310 es

#extension GL_GOOGLE_include_directive : enable

#include "constants.h"

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

layout(set = 0, binding = 0) readonly buffer _unused_name_perframe
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

layout(set = 2, binding = 0) uniform _unused_name_permaterial
{
    highp vec4  baseColorFactor;
    highp float metallicFactor;
    highp float roughnessFactor;
    highp float normalScale;
    highp float occlusionStrength;
    highp vec3  emissiveFactor;
    uint        is_blend;
    uint        is_double_sided;
};

layout(set = 2, binding = 1) uniform sampler2D base_color_texture_sampler;
layout(set = 2, binding = 2) uniform sampler2D metallic_roughness_texture_sampler;
layout(set = 2, binding = 3) uniform sampler2D normal_texture_sampler;
layout(set = 2, binding = 4) uniform sampler2D occlusion_texture_sampler;
layout(set = 2, binding = 5) uniform sampler2D emissive_color_texture_sampler;

// read in fragnormal (from vertex shader)
layout(location = 0) in highp vec3 in_world_position;
layout(location = 1) in highp vec3 in_normal;
layout(location = 2) in highp vec3 in_tangent;
layout(location = 3) in highp vec2 in_texcoord;

layout(location = 0) out highp vec4 out_scene_color;

highp vec3 getBasecolor()
{
    highp vec3 basecolor = texture(base_color_texture_sampler, in_texcoord).xyz * baseColorFactor.xyz;
    return basecolor;
}

highp vec3 calculateNormal()
{
    highp vec3 tangent_normal = texture(normal_texture_sampler, in_texcoord).xyz * 2.0 - 1.0;

    highp vec3 N = normalize(in_normal);
    highp vec3 T = normalize(in_tangent.xyz);
    highp vec3 B = normalize(cross(N, T));

    highp mat3 TBN = mat3(T, B, N);
    return normalize(TBN * tangent_normal);
}

#include "mesh_lighting.h"

void main()
{
    highp vec3  N                   = calculateNormal();
    highp vec3  basecolor           = getBasecolor();
    highp float metallic            = texture(metallic_roughness_texture_sampler, in_texcoord).z * metallicFactor;
    highp float dielectric_specular = 0.04;
    highp float roughness           = texture(metallic_roughness_texture_sampler, in_texcoord).y * roughnessFactor;

    highp vec3 result_color;

#include "mesh_lighting.inl"

    out_scene_color = vec4(result_color, 1.0);
}
