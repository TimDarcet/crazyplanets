#pragma once


#include "../../vcl/vcl.hpp"
#include "../../helper_scene.hpp"
#include "../../exercise_current.hpp"

class Planet {
  public:
    vcl::mesh_drawable planet_gpu;
    Planet(int roughness, float radius, int precision);
};