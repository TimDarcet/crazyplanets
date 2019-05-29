#pragma once


#include "../../vcl/vcl.hpp"
#include "../../helper_scene.hpp"
#include "../../exercise_current.hpp"

class Planet {
  public:
    vcl::mesh_drawable planet_gpu;
    Planet(float height, float radius, int octave=5, float persistency=3, float freq_gain=2, int precision=100);
};