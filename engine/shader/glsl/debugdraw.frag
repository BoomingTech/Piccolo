#version 450

#extension GL_GOOGLE_include_directive :enable
layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 2) uniform sampler2D texSampler;


void main(){
    outColor = fragColor;
    if(fragTexCoord.x >= 0.0f && fragTexCoord.y >= 0.0f)
    {
        vec4 tex = texture(texSampler, fragTexCoord);
        float xi = tex.r;
        outColor = vec4(fragColor.r*xi,fragColor.g*xi,fragColor.b*xi,fragColor.a*xi);
    }
}