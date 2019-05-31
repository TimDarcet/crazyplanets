#include "Planet.hpp"

Planet::Planet(float height, float radius, int octave, float persistency, float freq_gain, int precision) {
    planet_cpu.texture_uv.resize(precision * precision);
    for( size_t ku=0; ku<precision; ++ku ) {
        for( size_t kv=0; kv<precision; ++kv ) {
            const float u = static_cast<float>(ku)/static_cast<float>(precision-1);
            const float v = static_cast<float>(kv)/static_cast<float>(precision);
  
            const float theta = static_cast<float>( 3.14159f*u );
            const float phi   = static_cast<float>( 2*3.14159f*v );
  
            const float tmpX = std::sin(theta) * std::cos(phi);
            const float tmpY = std::sin(theta) * std::sin(phi);
            const float tmpZ = std::cos(theta);
  
            const float x = (radius + height * vcl::perlin(tmpX, tmpY, tmpZ, octave, persistency, freq_gain)) * std::sin(theta) * std::cos(phi);
            const float y = (radius + height * vcl::perlin(tmpX, tmpY, tmpZ, octave, persistency, freq_gain)) * std::sin(theta) * std::sin(phi);
            const float z = (radius + height * vcl::perlin(tmpX, tmpY, tmpZ, octave, persistency, freq_gain)) * std::cos(theta);
  
            const vcl::vec3 p = {x,y,z};
            // const vcl::vec3 n = normalize(p);
  
            planet_cpu.position.push_back(p);
            // planet_cpu.normal.push_back(n);

            planet_cpu.texture_uv[kv+precision*ku] = {5*u, 5*v};

        }
    }

    planet_cpu.connectivity = vcl::connectivity_grid(precision, precision, false, true);
}

vcl::mesh_drawable Planet::planet_gpu() {
    vcl::mesh_drawable p_gpu = planet_cpu;
    return p_gpu;
}