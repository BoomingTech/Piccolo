#version 310 es

layout(early_fragment_tests) in;

layout(location = 0) out highp float out_depth;

void main()
{
    out_depth = gl_FragCoord.z;
}