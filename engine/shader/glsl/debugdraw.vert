#version 450

#extension GL_GOOGLE_include_directive :enable
#include "constants.h"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 texcoord;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 proj_view_matrix;
} ubo;

layout(set = 0, binding = 1) uniform UniformDynamicBufferObject {
    mat4 model;
    vec4 color;
} dynamic_ubo;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    if(texcoord.x<0)
    {
        gl_Position = ubo.proj_view_matrix * dynamic_ubo.model * vec4(inPosition,1.0);
    }
    else
    {
        gl_Position = vec4(inPosition,1.0);
    }
    
    gl_PointSize = 2;

    if(dynamic_ubo.color.a>0.000001)
    {
        fragColor = dynamic_ubo.color;
    }
    else 
    {
        fragColor = inColor;
    }
    fragTexCoord = texcoord;
}