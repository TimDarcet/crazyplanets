#pragma once

#include "../../exercises/base_exercise/base_exercise.hpp"
#include "Planet.hpp"
#include "cloud.hpp"

#ifdef INF443_0P_CRAZYPLANETS

// Stores some parameters that can be set from the GUI
// Stores some parameters that can be set from the GUI
struct gui_scene_structure
{
    bool wireframe = false;
    bool skybox = true;
    bool swarm = false;
    bool main_planet = true;
    
    int precision = 300;

    float height = 1.0f;
    float radius = 10.0f;
    int octave = 4;
    float persistency = 0.4f;
    float freq_gain = 2;
};

struct trajectory_structure
{
    std::vector<vcl::vec3> position;
    std::vector<float> time;
};

struct scene_exercise : base_scene_exercise
{

    /** A part must define two functions that are called from the main function:
     * setup_data: called once to setup data before starting the animation loop
     * frame_draw: called at every displayed frame within the animation loop
     *
     * These two functions receive the following parameters
     * - shaders: A set of shaders.
     * - scene: Contains general common object to define the 3D scene. Contains in particular the camera.
     * - data: The part-specific data structure defined previously
     * - gui: The GUI structure allowing to create/display buttons to interact with the scene.
    */

    void setup_data(std::map<std::string,GLuint>& shaders, scene_structure& scene, gui_structure& gui);
    void frame_draw(std::map<std::string,GLuint>& shaders, scene_structure& scene, gui_structure& gui);

    void set_gui();

    // visual representation of a surface
    vcl::mesh_drawable terrain;
    vcl::mesh terrain_cpu;
    GLuint asteroid_texture_id;
    GLuint mPlanet_texture_id;
    GLuint texture_skybox;



    gui_scene_structure gui_scene;

    std::vector<vcl::vec3> tree_position;
    std::vector<vcl::vec3> planets_positions;

    std::vector<vcl::mesh_drawable> trees;
    std::vector<vcl::mesh_drawable> planets;

    vcl::mesh_drawable skybox;
    vcl::mesh_drawable_hierarchy bird;
    void display_bird(std::map<std::string,GLuint>& shaders, scene_structure& scene);
    trajectory_structure trajectory;
    void update_trajectory();
    void update_time_trajectory();
    
    void update_tree_position();
    vcl::mesh create_terrain();
    void update_terrain();
    void display_skybox(std::map<std::string,GLuint>& shaders, scene_structure& scene);
    vcl::timer_interval timer;

    std::vector<vcl::mesh_drawable> nuage_gpu;
    GLuint texture_id;
    Cloud nuage;
};

struct colline {
  vcl::vec2 center;
  float height;
  float sigma;
};

vcl::vec3 cardinal_spline_interpolation(float t, float t0, float t1, float t2, float t3, const vcl::vec3& p0, const vcl::vec3& p1, const vcl::vec3& p2, const vcl::vec3& p3);
vcl::vec3 cardinal_spline_derivative_interpolation(float t, float t0, float t1, float t2, float t3, const vcl::vec3& p0, const vcl::vec3& p1, const vcl::vec3& p2, const vcl::vec3& p3);
vcl::vec3 cardinal_spline_interpolation(const trajectory_structure& trajectory, float t);
vcl::vec3 cardinal_spline_derivative_interpolation(const trajectory_structure& trajectory, float t);
vcl::mesh create_tree();
vcl::mesh create_tree_foliage(float radius, float height, float z_offset, float startHeight);
vcl::mesh create_cone(float radius, float height, float z_offset);
vcl::mesh create_cylinder(float radius, float height);
Planet asteroid_generator();
size_t index_at_value(float t, const std::vector<float>& vt);

#endif
