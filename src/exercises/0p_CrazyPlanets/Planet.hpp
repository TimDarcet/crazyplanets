#pragma once

#ifdef INF443_0P_CRAZYPLANETS
#include "../../vcl/vcl.hpp"
#include "../../helper_scene.hpp"
#include "../../exercise_current.hpp"

class Planet {
  public:
    vcl::mesh planet_cpu;
    vcl::mesh_drawable planet_gpu();
    Planet(float height, float radius, int octave=5, float persistency=3, float freq_gain=2, int precision=100);
};

void generate_planet_positions(std::vector<vcl::vec3> &plan_pos);

void generate_swarm(std::vector<vcl::mesh_drawable> &plan_swarm, Planet (*planet_generator)());


#endif