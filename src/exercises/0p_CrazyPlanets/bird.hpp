#pragma once

#ifdef INF443_0P_CRAZYPLANETS
#include "../../vcl/vcl.hpp"
#include "../../helper_scene.hpp"
#include "../../exercise_current.hpp"
// #include "../../exercises/base_exercise/base_exercise.hpp"



void simple_wings(vcl::mesh_drawable_hierarchy &hier, float r_body, vcl::vec3 s_body);

void sheet_wings(vcl::mesh_drawable_hierarchy &hier, vcl::vec3 attach, float (*front)(float), float (*back)(float), float length);

void thick_wings(vcl::mesh_drawable_hierarchy &hier, vcl::vec3 attach, float (*front)(float), float (*back)(float), float length, float (*thickness)(float));

void move_simple_wings(vcl::mesh_drawable_hierarchy &hier, float t);

void move_sheet_wings(vcl::mesh_drawable_hierarchy &hier, float t, int precision, float (*mov)(float, float));


#endif