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
	highp vec3 position;
	highp float radius;
	highp vec3 intensity;
	lowp float _padding_intensity;
};

layout(set = 0, binding = 0) readonly buffer _unused_name_perframe
{
	highp mat4 proj_view_matrix;
	highp vec3 camera_position;
	lowp float _padding_camera_position;
	highp vec3 ambient_light;
	lowp float _padding_ambient_light;
	highp uint point_light_num;
	uint _padding_point_light_num_1;
	uint _padding_point_light_num_2;
	uint _padding_point_light_num_3;
	PointLight scene_point_lights[m_max_point_light_count];
    DirectionalLight scene_directional_light;
	highp mat4 directional_light_proj_view;
};

layout(set = 0, binding = 3) uniform sampler2D brdfLUT_sampler;
layout(set = 0, binding = 4) uniform samplerCube irradiance_sampler;
layout(set = 0, binding = 5) uniform samplerCube specular_sampler;
layout(set = 0, binding = 6) uniform highp sampler2DArray point_lights_shadow;
layout(set = 0, binding = 7) uniform highp sampler2D directional_light_shadow;

layout(set = 2, binding = 0) uniform _unused_name_permaterial
{
	highp vec4 baseColorFactor;
	highp float metallicFactor;
	highp float roughnessFactor;
	highp float normalScale;
	highp float occlusionStrength;
	highp vec3 emissiveFactor;
	uint is_blend;
	uint is_double_sided;
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

//output screen color to location 0
layout(location = 0) out highp vec4 out_color;

#define PI 3.1416

// todo: param/const
#define MAX_REFLECTION_LOD 8.0

highp vec3 getBasecolor()
{
	highp vec3 basecolor = texture(base_color_texture_sampler, in_texcoord).xyz * baseColorFactor.xyz;
	return basecolor;
}

// Normal Distribution function --------------------------------------
highp float D_GGX(highp float dotNH, highp float roughness)
{
	highp float alpha = roughness * roughness;
	highp float alpha2 = alpha * alpha;
	highp float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
	return (alpha2) / (PI * denom * denom);
}

// Geometric Shadowing function --------------------------------------
highp float G_SchlicksmithGGX(highp float dotNL, highp float dotNV, highp float roughness)
{
	highp float r = (roughness + 1.0);
	highp float k = (r * r) / 8.0;
	highp float GL = dotNL / (dotNL * (1.0 - k) + k);
	highp float GV = dotNV / (dotNV * (1.0 - k) + k);
	return GL * GV;
}

// Fresnel function ----------------------------------------------------
highp vec3 F_Schlick(highp float cosTheta, highp vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
highp vec3 F_SchlickR(highp float cosTheta, highp vec3 F0, highp float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

// Specular and diffuse BRDF composition --------------------------------------------
highp vec3 BRDF(highp vec3 L, highp vec3 V, highp vec3 N, highp vec3 F0, highp float metallic, highp float roughness)
{
	// Precalculate vectors and dot products
	highp vec3 H = normalize(V + L);
	highp float dotNV = clamp(dot(N, V), 0.0, 1.0);
	highp float dotNL = clamp(dot(N, L), 0.0, 1.0);
	highp float dotLH = clamp(dot(L, H), 0.0, 1.0);
	highp float dotNH = clamp(dot(N, H), 0.0, 1.0);

	// Light color fixed
	// vec3 lightColor = vec3(1.0);

	highp vec3 color = vec3(0.0);

	highp float rroughness = max(0.05, roughness);
	// D = Normal distribution (Distribution of the microfacets)
	highp float D = D_GGX(dotNH, rroughness);
	// G = Geometric shadowing term (Microfacets shadowing)
	highp float G = G_SchlicksmithGGX(dotNL, dotNV, rroughness);
	// F = Fresnel factor (Reflectance depending on angle of incidence)
	highp vec3 F = F_Schlick(dotNV, F0);

	highp vec3 spec = D * F * G / (4.0 * dotNL * dotNV + 0.001);
	highp vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);

	color += (kD * getBasecolor() / PI + (1.0 - kD) * spec);
	//color += (kD * getBasecolor() / PI + spec) * dotNL;
	//color += (kD * getBasecolor() / PI + spec) * dotNL * lightColor;

	return color;
}

highp vec2 ndcxy_to_uv(highp vec2 ndcxy)
{
	return ndcxy * vec2(0.5, 0.5) + vec2(0.5, 0.5);
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
	highp vec3 N = calculateNormal();
	highp vec3 V = normalize(camera_position - in_world_position);
	highp vec3 R = reflect(-V, N);

	highp vec3 origin_samplecube_N = vec3(N.x, N.z, N.y);
	highp vec3 origin_samplecube_R = vec3(R.x, R.z, R.y);

	highp float metallic = texture(metallic_roughness_texture_sampler, in_texcoord).z * metallicFactor;
	highp float roughness = texture(metallic_roughness_texture_sampler, in_texcoord).y * roughnessFactor;

	highp vec3 F0 = mix(vec3(0.04, 0.04, 0.04), getBasecolor(), metallic);

	// direct light specular and diffuse BRDF contribution
	highp vec3 Lo = vec3(0.0, 0.0, 0.0);
	for (highp int light_index = 0; light_index < int(point_light_num) && light_index < m_max_point_light_count; ++light_index)
	{
		highp vec3 point_light_position = scene_point_lights[light_index].position;
		highp float point_light_radius = scene_point_lights[light_index].radius;

		highp vec3 L = normalize(point_light_position - in_world_position);
		highp float NoL = min(dot(N, L), 1.0);

		// point light
		highp float distance = length(point_light_position - in_world_position);
		highp float distance_attenuation = 1.0 / (distance * distance + 1.0);
		highp float radius_attenuation = 1.0 - ((distance * distance) / (point_light_radius * point_light_radius));

		highp float light_attenuation = radius_attenuation * distance_attenuation * NoL;
		if (light_attenuation > 0.0)
		{
			highp float shadow;
			{
				// world space to light view space
				// identity rotation
				// Z - Up
				// Y - Forward
				// X - Right
				highp vec3 position_view_space = in_world_position - point_light_position;

				highp vec3 position_spherical_function_domain = normalize(position_view_space);

				// use abs to avoid divergence
				// z > 0
				// (x_2d, y_2d, 0) + (0, 0, 1) = λ ((x_sph, y_sph, z_sph) + (0, 0, 1))
				// (x_2d, y_2d) = (x_sph, y_sph) / (z_sph + 1)
				// z < 0
				// (x_2d, y_2d, 0) + (0, 0, -1) = λ ((x_sph, y_sph, z_sph) + (0, 0, -1))
				// (x_2d, y_2d) = (x_sph, y_sph) / (-z_sph + 1)
				highp vec2 position_ndcxy = position_spherical_function_domain.xy / (abs(position_spherical_function_domain.z) + 1.0);

				// use sign to avoid divergence
				// -1.0 to 0
				// 1.0 to 1
				highp vec2 uv = ndcxy_to_uv(position_ndcxy);
				highp float layer_index = (0.5 + 0.5 * sign(position_spherical_function_domain.z)) + 2.0 * float(light_index);

				highp float depth = texture(point_lights_shadow, vec3(uv, layer_index)).r + 0.000075;
				highp float closest_length = (depth)*point_light_radius;

				highp float current_length = length(position_view_space);

				shadow = (closest_length >= current_length) ? 1.0f : -1.0f;
			}

			if (shadow > 0.0f)
			{
				highp vec3 En = scene_point_lights[light_index].intensity * light_attenuation;
				Lo += BRDF(L, V, N, F0, metallic, roughness) * En;
			}
		}
	};

#if 1
	// direct ambient contribution
	highp vec3 La = vec3(0.0f, 0.0f, 0.0f);
	// float ambient_occlusion = texture(occlusion_texture_sampler, in_texcoord).x * occlusionStrength;
	// La = getBasecolor() * ambient_light * ambient_occlusion;
	La = getBasecolor() * ambient_light;

	// indirect environment
	highp vec3 irradiance = texture(irradiance_sampler, origin_samplecube_N).rgb;
	highp vec3 diffuse = irradiance * getBasecolor();

	highp vec3 F = F_SchlickR(max(dot(N, V), 0.0), F0, roughness);
	highp vec2 brdfLUT = texture(brdfLUT_sampler, vec2(max(dot(N, V), 0.0), roughness)).rg;

	highp float lod = roughness * MAX_REFLECTION_LOD;
	highp vec3 reflection = textureLod(specular_sampler, origin_samplecube_R, lod).rgb;
	highp vec3 specular = reflection * (F * brdfLUT.x + brdfLUT.y);

	highp vec3 kD = 1.0 - F;
	kD *= 1.0 - metallic;
	highp vec3 Libl = (kD * diffuse + specular);

	// emmisive contribution
	highp vec3 Le = texture(emissive_color_texture_sampler, in_texcoord).xyz * emissiveFactor;
#endif
	
	// directional light
	{
		highp vec3 L = normalize(scene_directional_light.direction);
		highp float NoL = min(dot(N, L), 1.0);

		if (NoL > 0.0)
		{

			highp float shadow;
			{
				highp vec4 position_clip = directional_light_proj_view * vec4(in_world_position, 1.0);
				highp vec3 position_ndc = position_clip.xyz / position_clip.w;

				highp vec2 uv = ndcxy_to_uv(position_ndc.xy);

				highp float closest_depth = texture(directional_light_shadow, uv).r + 0.000075;
				highp float current_depth = position_ndc.z;

				shadow = (closest_depth >= current_depth) ? 1.0f : -1.0f;
			}

			if (shadow > 0.0f)
			{
				highp vec3 En = scene_directional_light.color * NoL;
				Lo += BRDF(L, V, N, F0, metallic, roughness) * En;
			}
		}
	}

	// result
	highp vec3 result_color = Lo+ La + Libl + Le;

	out_color = vec4(result_color, 1.0f);
}