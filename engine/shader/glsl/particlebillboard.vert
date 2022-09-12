#version 310 es

#extension GL_GOOGLE_include_directive : enable

#include "constants.h"

layout(set = 0, binding = 0) uniform _unused_name_perframe
{
    mat4 proj_view_matrix;
    vec3 right_diection;
    vec3 up_direction;
    vec3 forward_diection;
};

struct Particle
{
    vec3  pos;
    float life;
    vec3  vel;
    float size_x;
    vec3  acc;
    float size_y;
    vec4  color;
};

layout(set = 0, binding = 1) readonly buffer _unused_name_perdrawcall { Particle particles[]; };

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec2 out_uv;

void main()
{
    const vec2 vertex_buffer[4] = vec2[4](vec2(-0.5, 0.5), vec2(0.5, 0.5), vec2(-0.5, -0.5), vec2(0.5, -0.5));
    const vec2 uv_buffer[4]     = vec2[4](vec2(0, 1), vec2(1, 1), vec2(0, 0), vec2(1, 0));
    vec2       model_position   = vertex_buffer[gl_VertexIndex];

    // Real-Time Rendering Fourth Edition
    // 13.6 Billboarding
    // 13.6.2 World-Oriented Billboard
    Particle particle        = particles[gl_InstanceIndex];
    vec3     anchor_location = particle.pos;

    // viewport-oriented
    vec3  vel_dir      = particle.vel;
    float projectvel_x = dot(vel_dir, right_diection);
    float projectvel_y = dot(vel_dir, up_direction);
    float size_x       = particle.size_x;
    float size_y       = particle.size_y;

    vec3 world_position;
    if (abs(projectvel_x) < size_x || abs(projectvel_y) < size_y)
    {
        world_position =
            size_x * right_diection * model_position.x + size_y * up_direction * model_position.y + anchor_location;
    }
    else
    {
        vec3 project_dir = normalize(projectvel_x * right_diection + projectvel_y * up_direction);
        vec3 side_dir    = normalize(cross(forward_diection, project_dir));
        world_position =
            size_x * side_dir * model_position.x + size_y * project_dir * model_position.y + anchor_location;
    }

    // world to NDC
    gl_Position = proj_view_matrix * vec4(world_position, 1.0);

    out_color = particle.color;
    out_uv    = uv_buffer[gl_VertexIndex];
}