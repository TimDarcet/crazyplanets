#include "bird.hpp"

#include<string.h>

#ifdef INF443_0P_CRAZYPLANETS
vcl::vec3 reflect(vcl::vec3 v, int dir) 
{
    vcl::vec3 r = {v[0], v[1], v[2]};
    r[dir] = -r[dir];
    return r;
}


void simple_wings(vcl::mesh_drawable_hierarchy &hier, float r_body, vcl::vec3 s_body)
{
    const vcl::vec3 shape_arm_tl[] = {{0,0,-0.4},{0,0,0.4},{-1.2,0,0.3},{-1.2,0,-0.3 }};
    const vcl::vec3 pos_arm_tl = {-0.85,0,0};

    const vcl::vec3 shape_arm_bl[] = {{0,0,-0.3},{0,0,0.3},{-0.9,0,0.2},{-0.9,0,-0.2 }};

    vcl::mesh arm_top_left = vcl::mesh_primitive_quad(r_body*termP(shape_arm_tl[0], s_body),
                                            r_body*termP(shape_arm_tl[1], s_body),
                                            r_body*termP(shape_arm_tl[2], s_body),
                                            r_body*termP(shape_arm_tl[3], s_body));
    vcl::mesh arm_bottom_left = vcl::mesh_primitive_quad(r_body*termP(shape_arm_bl[0], s_body),
                                               r_body*termP(shape_arm_bl[1], s_body),
                                               r_body*termP(shape_arm_bl[2], s_body),
                                               r_body*termP(shape_arm_bl[3], s_body));

    vcl::mesh arm_top_right = vcl::mesh_primitive_quad(reflect(r_body*termP(shape_arm_tl[0], s_body), 0),
                                             reflect(r_body*termP(shape_arm_tl[1], s_body), 0),
                                             reflect(r_body*termP(shape_arm_tl[2], s_body), 0),
                                             reflect(r_body*termP(shape_arm_tl[3], s_body), 0));
    vcl::mesh arm_bottom_right = vcl::mesh_primitive_quad(reflect(r_body*termP(shape_arm_bl[0], s_body), 0),
                                                reflect(r_body*termP(shape_arm_bl[1], s_body), 0),
                                                reflect(r_body*termP(shape_arm_bl[2], s_body), 0),
                                                reflect(r_body*termP(shape_arm_bl[3], s_body), 0));

    hier.add_element(arm_top_left, "arm_top_left", "body",r_body*termP(s_body, pos_arm_tl));
    hier.add_element(arm_bottom_left, "arm_bottom_left", "arm_top_left",{(shape_arm_tl[3][0]) * r_body * s_body[0], 0, 0});

    hier.add_element(arm_top_right, "arm_top_right", "body", reflect(r_body*termP(s_body, pos_arm_tl), 0));
    hier.add_element(arm_bottom_right, "arm_bottom_right", "arm_top_right",{-shape_arm_tl[3][0] * r_body * s_body[0], 0, 0});

}

void sheet_wings(vcl::mesh_drawable_hierarchy &hier, int precision, vcl::vec3 attach, float (*front)(float), float (*back)(float), float length)
{
    vcl::mesh left_wing[precision];
    vcl::mesh right_wing[precision];
    float fact = length / precision;
    for (int i=0; i<precision; i++)
    {
        left_wing[i] = vcl::mesh_primitive_quad({-i*fact,0,front((float)i/precision)},
                                           {-(i+1)*fact,0,front((float)(i+1)/precision)},
                                           {-(i+1)*fact,0,back((float)(i+1)/precision)},
                                           {-i*fact,0,back((float)i/precision)});
        right_wing[i] = vcl::mesh_primitive_quad({i*fact,0,front((float)i/precision)},
                                            {(i+1)*fact,0,front((float)(i+1)/precision)},
                                            {(i+1)*fact,0,back((float)(i+1)/precision)},
                                            {i*fact,0,back((float)i/precision)});
    }

    
    hier.add_element(left_wing[0], "lwing0", "body", attach);
    hier.add_element(right_wing[0], "rwing0", "body", reflect(attach, 0));

    for (int i=1; i<precision; i++)
    {
        std::string lwing = "lwing";
        hier.add_element(left_wing[i], lwing + vcl::to_string(i), lwing + vcl::to_string(i-1), {0, 0, 0});
        std::string rwing = "rwing";
        hier.add_element(right_wing[i], rwing + vcl::to_string(i), rwing + vcl::to_string(i-1), {0, 0, 0});
    }
}

void thick_wings(vcl::mesh_drawable_hierarchy &hier, int precision, vcl::vec3 attach, float (*front)(float), float (*back)(float), float length, float (*thickness)(float))
{
    vcl::mesh left_wing[precision];
    vcl::mesh right_wing[precision];
    float fact = length / precision;
    for (int i=0; i<precision; i++)
    {
        left_wing[i] = vcl::mesh_primitive_smooth_cyl(thickness(i*fact),
                                                      vcl::vec3{front((float)i/precision), (float)-i*fact, 0.0f},
                                                      vcl::vec3{back((float)i/precision), (float)-i*fact, 0.0f});
                                                     //  {-i*fact,0,front((float)i/precision)},
                                                     //  {-i*fact,0,back((float)i/precision)});
        right_wing[i] = vcl::mesh_primitive_smooth_cyl(thickness(i*fact),
                                                       vcl::vec3{front((float)i/precision), (float)i*fact, 0.0f},
                                                       vcl::vec3{back((float)i/precision), (float)i*fact, 0.0f});
}

    
    hier.add_element(left_wing[0], "lwing0", "body", attach);
    hier.add_element(right_wing[0], "rwing0", "body", reflect(attach, 1));

    for (int i=1; i<precision; i++)
    {
        std::string lwing = "lwing";
        hier.add_element(left_wing[i], lwing + vcl::to_string(i), lwing + vcl::to_string(i-1), {0, 0, 0});
        std::string rwing = "rwing";
        hier.add_element(right_wing[i], rwing + vcl::to_string(i), rwing + vcl::to_string(i-1), {0, 0, 0});
    }
}

void move_simple_wings(vcl::mesh_drawable_hierarchy &hier, float t)
{
    hier.rotation("arm_top_left") = vcl::rotation_from_axis_angle_mat3({0,0,1}, std::sin(2*3.14f*(t-0.4f)) );
    hier.rotation("arm_bottom_left") = vcl::rotation_from_axis_angle_mat3({0,0,1}, std::sin(2*3.14f*(t-0.6f)) );

    hier.rotation("arm_top_right") = vcl::rotation_from_axis_angle_mat3({0,0,-1}, std::sin(2*3.14f*(t-0.4f)) );
    hier.rotation("arm_bottom_right") = vcl::rotation_from_axis_angle_mat3({0,0,-1}, std::sin(2*3.14f*(t-0.6f)) );

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
        hier.rotation(lwing + vcl::to_string(i)) = vcl::rotation_from_axis_angle_mat3({1,0,0}, angle - lastangle);
        hier.rotation(rwing + vcl::to_string(i)) = vcl::rotation_from_axis_angle_mat3({-1,0,0}, angle - lastangle);
        lastangle = angle;
    }
}

vcl::mesh_drawable_hierarchy create_bird(int precision) {
    vcl::mesh_drawable_hierarchy hierarchy;
    
    const float r_body = 0.25f;
    const vcl::vec3 s_body = {1.5,1,0.75};
    const float r_head = 0.15f;
    const vcl::vec3 s_head = {1,1,1};
    const vcl::vec3 pos_head = {0.75f,0,0.8f};

    const float r_eye = 0.03f;
    const vcl::vec3 s_eye = {1,1,1};
    const vcl::vec3 pos_eye_l = {1/2.0f,1/3.0f,1/1.5f};
    const vcl::vec3 pos_eye_r = {pos_eye_l.x,-pos_eye_l.y,pos_eye_l.z};
    const vcl::vec3 pos_beak = {1/2.0f,0.0f,0.0f};
    
    const vcl::vec3 pos_lwing = {-0.5,-0.5,0};



    vcl::mesh body = vcl::mesh_primitive_ellipsoid(r_body,{0,0,0}, s_body,40,40);
    vcl::mesh head = vcl::mesh_primitive_ellipsoid(r_head,{0,0,0}, s_head,40,40);

    vcl::mesh_drawable eye = vcl::mesh_primitive_ellipsoid(r_eye,{0,0,0},s_eye,20,20);
    eye.uniform_parameter.color = {0,0,0};

    vcl::mesh bird_beak = vcl::mesh_primitive_cone(0.04f, {0,0,0}, {0.2f, 0.0f, -0.2f});

    hierarchy.add_element(body, "body", "root");
    hierarchy.add_element(head, "head", "body",r_body*termP(s_body, pos_head));
    hierarchy.add_element(bird_beak,"beak","head", r_head*termP(s_head, pos_beak));
    hierarchy.add_element(eye, "eye_left", "head",r_head*termP(s_head, pos_eye_l));
    hierarchy.add_element(eye, "eye_right", "head",r_head*termP(s_head, pos_eye_r));
    thick_wings(hierarchy, precision, r_body*termP(s_body, pos_lwing), [](float x){return (float)0.3f*(float)sqrt(1-x);}, [](float x){return 0.03f*(float)sqrt(1-x);}, 2*r_body, [](float x){return 0.12f*(float)pow(1-x, 3);});

    hierarchy.mesh_visual("beak").uniform_parameter.color = {1,0.5f,0};

    hierarchy.scaling = 6;
    return hierarchy;
}



#endif