#include "Planet.hpp"

Planet::Planet(int roughness, float radius, int precision) {
    int octave = 5;
    float persistency = 3, freq_gain = 2;
    vcl::mesh planet_cpu; // temporary planet storage (CPU only)
    for( size_t ku=0; ku<precision; ++ku ) {
        for( size_t kv=0; kv<precision; ++kv ) {

            const float u = static_cast<float>(ku)/static_cast<float>(precision-1);
            const float v = static_cast<float>(kv)/static_cast<float>(precision);

            const float theta = static_cast<float>( 3.14159f*u );
            const float phi   = static_cast<float>( 2*3.14159f*v );

            const float x = (radius + roughness * vcl::perlin(theta, phi, octave, persistency, freq_gain)) * std::sin(theta) * std::cos(phi);
            const float y = (radius + roughness * vcl::perlin(theta, phi, octave, persistency, freq_gain)) * std::sin(theta) * std::sin(phi);
            const float z = (radius + roughness * vcl::perlin(theta, phi, octave, persistency, freq_gain)) * std::cos(theta);

            const vcl::vec3 p = {x,y,z};
            const vcl::vec3 n = normalize(p);

            planet_cpu.position.push_back(p);
            planet_cpu.normal.push_back(n);
        }
    }

    planet_cpu.connectivity = vcl::connectivity_grid(precision, precision, false, true);
    planet_gpu = planet_cpu;
}