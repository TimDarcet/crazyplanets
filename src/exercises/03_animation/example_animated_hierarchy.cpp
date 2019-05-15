
#include "example_animated_hierarchy.hpp"
#include<string.h>
#include<math.h>

#ifdef INF443_EXAMPLE_ANIMATED_HIERARCHY

#define PRECISION 50
#define PI 3.141592653589793238f


using namespace vcl;

vec3 reflect(vec3 v, int dir) 
{
    vec3 r = {v[0], v[1], v[2]};
    r[dir] = -r[dir];
    return r;
}

void simple_wings(vcl::mesh_drawable_hierarchy &hier, float r_body, vec3 s_body)
{
    const vec3 shape_arm_tl[] = {{0,0,-0.4},{0,0,0.4},{-1.2,0,0.3},{-1.2,0,-0.3 }};
    const vec3 pos_arm_tl = {-0.85,0,0};

    const vec3 shape_arm_bl[] = {{0,0,-0.3},{0,0,0.3},{-0.9,0,0.2},{-0.9,0,-0.2 }};

    mesh arm_top_left = mesh_primitive_quad(r_body*termP(shape_arm_tl[0], s_body),
                                            r_body*termP(shape_arm_tl[1], s_body),
                                            r_body*termP(shape_arm_tl[2], s_body),
                                            r_body*termP(shape_arm_tl[3], s_body));
    mesh arm_bottom_left = mesh_primitive_quad(r_body*termP(shape_arm_bl[0], s_body),
                                               r_body*termP(shape_arm_bl[1], s_body),
                                               r_body*termP(shape_arm_bl[2], s_body),
                                               r_body*termP(shape_arm_bl[3], s_body));

    mesh arm_top_right = mesh_primitive_quad(reflect(r_body*termP(shape_arm_tl[0], s_body), 0),
                                             reflect(r_body*termP(shape_arm_tl[1], s_body), 0),
                                             reflect(r_body*termP(shape_arm_tl[2], s_body), 0),
                                             reflect(r_body*termP(shape_arm_tl[3], s_body), 0));
    mesh arm_bottom_right = mesh_primitive_quad(reflect(r_body*termP(shape_arm_bl[0], s_body), 0),
                                                reflect(r_body*termP(shape_arm_bl[1], s_body), 0),
                                                reflect(r_body*termP(shape_arm_bl[2], s_body), 0),
                                                reflect(r_body*termP(shape_arm_bl[3], s_body), 0));

    hier.add_element(arm_top_left, "arm_top_left", "body",r_body*termP(s_body, pos_arm_tl));
    hier.add_element(arm_bottom_left, "arm_bottom_left", "arm_top_left",{(shape_arm_tl[3][0]) * r_body * s_body[0], 0, 0});

    hier.add_element(arm_top_right, "arm_top_right", "body", reflect(r_body*termP(s_body, pos_arm_tl), 0));
    hier.add_element(arm_bottom_right, "arm_bottom_right", "arm_top_right",{-shape_arm_tl[3][0] * r_body * s_body[0], 0, 0});

}

void move_simple_wings(vcl::mesh_drawable_hierarchy &hier, float t)
{
    hier.rotation("arm_top_left") = rotation_from_axis_angle_mat3({0,0,1}, std::sin(2*3.14f*(t-0.4f)) );
    hier.rotation("arm_bottom_left") = rotation_from_axis_angle_mat3({0,0,1}, std::sin(2*3.14f*(t-0.6f)) );

    hier.rotation("arm_top_right") = rotation_from_axis_angle_mat3({0,0,-1}, std::sin(2*3.14f*(t-0.4f)) );
    hier.rotation("arm_bottom_right") = rotation_from_axis_angle_mat3({0,0,-1}, std::sin(2*3.14f*(t-0.6f)) );

}

void move_sheet_wings(vcl::mesh_drawable_hierarchy &hier, float t, int precision, float (*mov)(float, float))
{
    std::string lwing = "lwing";
    std::string rwing = "rwing";
    float lastangle = 0;
    for (int i = 0; i < precision - 1; i++)
    {
        float slope = (mov(t, (float)(i+1)/precision) - mov(t, (float)i/precision)) * precision;
        float angle = atan(slope);
        hier.rotation(lwing + to_string(i)) = rotation_from_axis_angle_mat3({0,0,1}, angle - lastangle);
        hier.rotation(rwing + to_string(i)) = rotation_from_axis_angle_mat3({0,0,-1}, angle - lastangle);
        lastangle = angle;
    }
}

void sheet_wings(vcl::mesh_drawable_hierarchy &hier, vec3 attach, float (*front)(float), float (*back)(float), float length)
{
    mesh left_wing[PRECISION];
    mesh right_wing[PRECISION];
    float fact = length / PRECISION;
    for (int i=0; i<PRECISION; i++)
    {
        left_wing[i] = mesh_primitive_quad({-i*fact,0,front((float)i/PRECISION)},
                                           {-(i+1)*fact,0,front((float)(i+1)/PRECISION)},
                                           {-(i+1)*fact,0,back((float)(i+1)/PRECISION)},
                                           {-i*fact,0,back((float)i/PRECISION)});
        right_wing[i] = mesh_primitive_quad({i*fact,0,front((float)i/PRECISION)},
                                            {(i+1)*fact,0,front((float)(i+1)/PRECISION)},
                                            {(i+1)*fact,0,back((float)(i+1)/PRECISION)},
                                            {i*fact,0,back((float)i/PRECISION)});
    }

    
    hier.add_element(left_wing[0], "lwing0", "body", attach);
    hier.add_element(right_wing[0], "rwing0", "body", reflect(attach, 0));

    for (int i=1; i<PRECISION; i++)
    {
        std::string lwing = "lwing";
        hier.add_element(left_wing[i], lwing + to_string(i), lwing + to_string(i-1), {0, 0, 0});
        std::string rwing = "rwing";
        hier.add_element(right_wing[i], rwing + to_string(i), rwing + to_string(i-1), {0, 0, 0});
    }
}

void thick_wings(vcl::mesh_drawable_hierarchy &hier, vec3 attach, float (*front)(float), float (*back)(float), float length, float (*thickness)(float))
{
    mesh left_wing[PRECISION];
    mesh right_wing[PRECISION];
    float fact = length / PRECISION;
    for (int i=0; i<PRECISION; i++)
    {
        left_wing[i] = mesh_primitive_smooth_cyl(thickness(i*fact),
                                                 {-i*fact,0,front((float)i/PRECISION)},
                                                 {-i*fact,0,back((float)i/PRECISION)});
        right_wing[i] = mesh_primitive_smooth_cyl(thickness(i*fact),
                                                  {i*fact,0,front((float)i/PRECISION)},
                                                  {i*fact,0,back((float)i/PRECISION)});
    }

    
    hier.add_element(left_wing[0], "lwing0", "body", attach);
    hier.add_element(right_wing[0], "rwing0", "body", reflect(attach, 0));

    for (int i=1; i<PRECISION; i++)
    {
        std::string lwing = "lwing";
        hier.add_element(left_wing[i], lwing + to_string(i), lwing + to_string(i-1), {0, 0, 0});
        std::string rwing = "rwing";
        hier.add_element(right_wing[i], rwing + to_string(i), rwing + to_string(i-1), {0, 0, 0});
    }
}

void scene_exercise::setup_data(std::map<std::string,GLuint>& , scene_structure& , gui_structure& )
{
    const float r_body = 0.25f;
    const vec3 s_body = {1,1,2};
    const float r_head = 0.15f;
    const vec3 s_head = {1,2,1};
    const vec3 pos_head = {0,0.75f,0.8f};

    const float r_eye = 0.03f;
    const vec3 s_eye = {1,1,1};
    const vec3 pos_eye_l = {1/2.0f,1/3.0f,1/1.5f};
    const vec3 pos_eye_r = {-pos_eye_l.x,pos_eye_l.y,pos_eye_l.z};
    
    const vec3 pos_lwing = {-0.55,0,-0.2};



    mesh body = mesh_primitive_ellipsoid(r_body,{0,0,0}, s_body,40,40);
    mesh head = mesh_primitive_ellipsoid(r_head,{0,0,0}, s_head,40,40);

    mesh_drawable eye = mesh_primitive_ellipsoid(r_eye,{0,0,0},s_eye,20,20);
    eye.uniform_parameter.color = {0,0,0};




    hierarchy.add_element(body, "body", "root");
    hierarchy.add_element(head, "head", "body",r_body*termP(s_body, pos_head));
    hierarchy.add_element(eye, "eye_left", "head",r_head*termP(s_head, pos_eye_l));
    hierarchy.add_element(eye, "eye_right", "head",r_head*termP(s_head, pos_eye_r));
    thick_wings(hierarchy, r_body*termP(s_body, pos_lwing), [](float x){return 0.3f*sqrt(1-x);}, [](float x){return 0.03f*sqrt(1-x);}, 2*r_body, [](float x){return 0.12f*(float)pow(1-x, 3);});
    timer.scale = 0.5f;
}


void scene_exercise::frame_draw(std::map<std::string,GLuint>& shaders, scene_structure& scene, gui_structure& )
{
    timer.update();
    set_gui();

    const float t = timer.t;
    hierarchy.translation("body") = {0,0,0.2f*(1+std::sin(2*3.14f*t))};

    move_sheet_wings(hierarchy, t, PRECISION, [](float t, float x){return (float) 0.2*sin(2*PI*t)*sin(0.8f*PI*x) + x * sin(2*PI*t+PI/2);});

    hierarchy.draw(shaders["mesh"], scene.camera);
    if(gui_scene.wireframe)
        hierarchy.draw(shaders["wireframe"], scene.camera);

}

void scene_exercise::set_gui()
{
    ImGui::SliderFloat("Time", &timer.t, timer.t_min, timer.t_max);

    const float time_scale_min = 0.1f;
    const float time_scale_max = 3.0f;
    ImGui::SliderFloat("Time scale", &timer.scale, time_scale_min, time_scale_max);
    ImGui::Checkbox("Wireframe", &gui_scene.wireframe);

}

#endif
