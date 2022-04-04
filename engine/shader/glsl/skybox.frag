#version 450

layout(set = 0, binding = 1) uniform samplerCube specular_sampler;

layout(location = 0) in vec3 in_UVW;

layout(location = 0) out vec4 outColor;

void main()
{
    vec3 origin_sample_UVW = vec3(in_UVW.x, in_UVW.z, in_UVW.y);
    vec3 color = textureLod(specular_sampler, origin_sample_UVW, 0).rgb;

    outColor = vec4(color, 1.0);
}