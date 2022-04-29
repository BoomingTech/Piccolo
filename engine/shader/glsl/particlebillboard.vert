#version 310 es

layout(set = 0, binding = 0) readonly buffer _unused_name_perframe
{
    mat4 proj_view_matrix;
    vec3 eye_position;
    vec3 up_direction;
};

layout(set = 0, binding = 1) readonly buffer _unused_name_perdrawcall { vec4 positions[4096]; };

void main()
{
    const vec2 vertex_buffer[4] = vec2[4](vec2(-1.0, 1.0), vec2(1.0, 1.0), vec2(-1.0, -1.0), vec2(1.0, -1.0));
    vec2       model_position   = vertex_buffer[gl_VertexIndex];

    // Real-Time Rendering Fourth Edition
    // 13.6 Billboarding
    // 13.6.2 World-Oriented Billboard

    vec3 anchor_location = positions[gl_InstanceIndex].xyz;

    // viewport-oriented
    vec3 front_direction = eye_position - anchor_location;

    // keep FrontDirection fixed and deduce UpDirection
    vec3 right_diection = normalize(cross(up_direction, front_direction));
    vec3 up_direction   = normalize(cross(front_direction, right_diection));

    // model to World
    vec3 world_position = right_diection * model_position.x + up_direction * model_position.y + anchor_location;

    // world to NDC
    gl_Position = proj_view_matrix * vec4(world_position, 1.0);
}