#version 460 core

#extension GL_GOOGLE_include_directive :enable

#include "constants.h"

struct PointLight
{
	vec3 position;
	float radius;
	vec3 intensity;
	float _padding_intensity;
};

layout(set = 0, binding = 0) readonly buffer _unused_name_perframe
{
    mat4 proj_view_matrix;
    vec3 camera_position;
    float _padding_camera_position;
    vec3 ambient_light;
    float _padding_ambient_light;
    uint point_light_num;
    uint _padding_point_light_num_1;
	uint _padding_point_light_num_2;
	uint _padding_point_light_num_3;
    PointLight scene_point_lights[m_max_point_light_count];
};

layout(set = 0, binding = 1) readonly buffer _unused_name_axis
{
    mat4 model_matrix;
    uint selected_axis;
};

layout(location = 0) in vec3 in_position; // for some types as dvec3 takes 2 locations
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_tangent;
layout(location = 3) in vec2 in_texcoord;

layout(location = 0) out vec3 out_color; // output in location 0 for fragment shader

void main()
{
    vec3 world_position = (model_matrix * vec4(in_position, 1.0)).xyz;
    vec4 clip_position = proj_view_matrix * vec4(world_position, 1.0f);

    // depth set to 0.0001 (closest)
    clip_position.z = clip_position.z * 0.0001;
    gl_Position = clip_position;

    if(in_texcoord.x < 0.01f)
    {
        if(selected_axis == 0)
        {
            out_color = vec3(1.0, 1.0, 0.0);
        }
        else
        {
            out_color = vec3(1.0, 0.0, 0.0);
        }
    }
    else if(in_texcoord.x < 1.01f)
    {
        if(selected_axis == 1)
        {
            out_color = vec3(1.0, 1.0, 0.0);
        }
        else
        {
            out_color = vec3(0.0, 1.0, 0.0);
        }
    }
    else if(in_texcoord.x < 2.01f)
    {
        if(selected_axis == 2)
        {
            out_color = vec3(1.0, 1.0, 0.0);
        }
        else
        {
            out_color = vec3(0.0, 0.0, 1.0);
        }
    }
    else
    {
        out_color = vec3(1.0, 1.0, 1.0);
    }
}