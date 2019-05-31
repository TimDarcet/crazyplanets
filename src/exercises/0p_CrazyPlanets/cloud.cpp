#include "cloud.hpp"
#include <random>
#include <time.h>

// Uniform distribution in [0,1]
std::uniform_real_distribution<float> distrib(0.0,1.0);
std::random_device rd;
std::default_random_engine generator(rd());

Cloud::Cloud(int _nb_surfaces, float _shape[3])
{
    nb_surfaces = _nb_surfaces;
    shape[0] = _shape[0];
    shape[1] = _shape[1];
    shape[2] = _shape[2];
    for (int i = 0; i < nb_surfaces; i++) {
        vcl::vec3 pos = {shape[0]*distrib(generator), shape[1]*distrib(generator), shape[2]*distrib(generator)};
        positions[i] = pos;
        vcl::mesh surface;
        surface.position = {pos+vcl::vec3({-0.2f,0,0}),pos+vcl::vec3({ 0.2f,0,0}),pos+vcl::vec3({ 0.2f, 0.4f,0}), pos+vcl::vec3({-0.2f, 0.4f,0})};
        int r = distrib(generator)*16;
        int x = r/4;
        int y = r%4;
        surface.texture_uv = {{static_cast<float>(x),(float)y+0.25f}, {(float)x+0.25f, (float)y+0.25f}, {(float)x+0.25f, static_cast<float>(y)}, {(float)x, (float)y}};
        surface.connectivity = {{0,1,2}, {0,2,3}};
        surfaces.push_back((surface));
    }
}
std::vector<vcl::mesh_drawable> Cloud::surfaces_gpu() {
    std::vector<vcl::mesh_drawable> s_gpu;
    for (int i = 0; i < nb_surfaces; i++) {
        vcl::mesh_drawable surface_gpu = surfaces[i];
        s_gpu.push_back(surface_gpu);
    }
    return s_gpu;
}
