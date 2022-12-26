#version 450 core

#extension GL_EXT_spirv_intrinsics: enable

layout(constant_id = 5) const uint targetWidth = 32;
spirv_execution_mode_id(4460/*=DenormFlushToZero*/, targetWidth);

layout(constant_id = 6) const uint builtIn = 1;
spirv_decorate_id(11/*=BuiltIn*/, builtIn) out float pointSize;

void main()
{
    pointSize = 4.0;
}
