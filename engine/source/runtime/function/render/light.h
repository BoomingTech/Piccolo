#pragma once

#include "runtime/core/math/vector3.h"

#include "runtime/function/render/render_type.h"

#include <vector>

namespace Piccolo
{
    struct PointLight
    {
        Vector3 m_position;
        // radiant flux in W
        Vector3 m_flux;

        // calculate an appropriate radius for light culling
        // a windowing function in the shader will perform a smooth transition to zero
        // this is not physically based and usually artist controlled
        float calculateRadius() const
        {
            // radius = where attenuation would lead to an intensity of 1W/m^2
            const float INTENSITY_CUTOFF    = 1.0f;
            const float ATTENTUATION_CUTOFF = 0.05f;
            Vector3     intensity           = m_flux / (4.0f * Math_PI);
            float       maxIntensity        = Vector3::getMaxElement(intensity);
            float       attenuation = Math::max(INTENSITY_CUTOFF, ATTENTUATION_CUTOFF * maxIntensity) / maxIntensity;
            return 1.0f / sqrtf(attenuation);
        }
    };

    struct AmbientLight
    {
        Vector3 m_irradiance;
    };

    struct PDirectionalLight
    {
        Vector3 m_direction;
        Vector3 m_color;
    };

    struct LightList
    {
        // vertex buffers seem to be aligned to 16 bytes
        struct PointLightVertex
        {
            Vector3 m_position;
            float   m_padding;
            // radiant intensity in W/sr
            // can be calculated from radiant flux
            Vector3 m_intensity;
            float   m_radius;
        };
    };

    class PointLightList : public LightList
    {
    public:
        void init() {}
        void shutdown() {}

        // upload changes to GPU
        void update() {}

        std::vector<PointLight> m_lights;

        std::shared_ptr<BufferData> m_buffer;
    };

} // namespace Piccolo