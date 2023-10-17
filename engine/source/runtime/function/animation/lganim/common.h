#pragma once

#include <math.h>

#define PIf 3.14159265358979323846f
#define LN2f 0.69314718056f

static inline float clampf(float x, float min, float max)
{
    return x > max ? max : x < min ? min : x;
}

static inline float minf(float x, float y)
{
    return x < y ? x : y;
}

static inline float maxf(float x, float y)
{
    return x > y ? x : y;
}

static inline float squaref(float x)
{
    return x*x;
}

static inline float lerpf(float x, float y, float a)
{
    return (1.0f - a) * x + a * y;
}

static inline float signf(float x)
{
    return x > 0.0f ? 1.0f : x < 0.0f ? -1.0f : 0.0f;
}

static inline float fast_negexpf(float x)
{
    return 1.0f / (1.0f + x + 0.48f*x*x + 0.235f*x*x*x);
}

static inline float fast_atanf(float x)
{
    float z = fabs(x);
    float w = z > 1.0f ? 1.0f / z : z;
    float y = (PIf / 4.0f)*w - w*(w - 1.0f)*(0.2447f + 0.0663f*w);
    return copysign(z > 1.0f ? PIf / 2.0f - y : y, x);
}

static inline int clamp(int x, int min, int max)
{
    return x < min ? min : x > max ? max : x;
}
