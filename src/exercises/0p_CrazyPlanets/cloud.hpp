#pragma once


#include "../../vcl/vcl.hpp"
#include "../../helper_scene.hpp"
#include "../../exercise_current.hpp"

class Cloud
{
public:
    Cloud(int _nb_surfaces, float _shape[3]);
    Cloud() = default;
    ~Cloud();
    int nb_surfaces;
    float shape[3];
    std::vector<vcl::mesh> surfaces;
    std::vector<vcl::mesh_drawable> surfaces_gpu();
    vcl::vec3 positions[];

};

