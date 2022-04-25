#version 310 es

#extension GL_GOOGLE_include_directive : enable

#include "constants.h"
#include "gbuffer.h"

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

// output screen color to location 0
layout(location = 0) out highp vec4 out_gbuffer_a;
layout(location = 1) out highp vec4 out_gbuffer_b;
layout(location = 2) out highp vec4 out_gbuffer_c;
// layout(location = 3) out highp vec4 out_scene_color;

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

void main()
{
    PGBufferData gbuffer;
    gbuffer.worldNormal    = calculateNormal();
    gbuffer.baseColor      = getBasecolor();
    gbuffer.metallic       = texture(metallic_roughness_texture_sampler, in_texcoord).z * metallicFactor;
    gbuffer.specular       = 0.5;
    gbuffer.roughness      = texture(metallic_roughness_texture_sampler, in_texcoord).y * roughnessFactor;
    gbuffer.shadingModelID = SHADINGMODELID_DEFAULT_LIT;

    highp vec3 Le = texture(emissive_color_texture_sampler, in_texcoord).xyz * emissiveFactor;

    EncodeGBufferData(gbuffer, out_gbuffer_a, out_gbuffer_b, out_gbuffer_c);

    // out_scene_color.rgba = vec4(Le, 1.0);
}