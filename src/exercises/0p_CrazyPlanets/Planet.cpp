#include "Planet.hpp"

Planet::Planet(float height, float radius, int precision, int octave, float persistency, float freq_gain) {
    vcl::mesh planet_cpu; // temporary planet storage (CPU only)
    for( size_t ku=0; ku<precision; ++ku ) {
        for( size_t kv=0; kv<precision; ++kv ) {

            const float u = static_cast<float>(ku)/static_cast<float>(precision-1);
            const float v = static_cast<float>(kv)/static_cast<float>(precision);

            const float theta = static_cast<float>( 3.14159f*u );
            const float phi   = static_cast<float>( 2*3.14159f*v );

            const float x = (radius + height * vcl::perlin(theta, phi, octave, persistency, freq_gain)) * std::sin(theta) * std::cos(phi);
            const float y = (radius + height * vcl::perlin(theta, phi, octave, persistency, freq_gain)) * std::sin(theta) * std::sin(phi);
            const float z = (radius + height * vcl::perlin(theta, phi, octave, persistency, freq_gain)) * std::cos(theta);

            const vcl::vec3 p = {x,y,z};
            const vcl::vec3 n = normalize(p);

            planet_cpu.position.push_back(p);
            planet_cpu.normal.push_back(n);
        }
    }

    planet_cpu.connectivity = vcl::connectivity_grid(precision, precision, false, true);
    planet_gpu = planet_cpu;
}