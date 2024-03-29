#pragma once

#include "../../exercises/base_exercise/base_exercise.hpp"

#ifdef INF443_01_MODELISATION

// Stores some parameters that can be set from the GUI
struct gui_scene_structure
{
    bool wireframe;
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
    GLuint texture_id;



    gui_scene_structure gui_scene;

    std::vector<vcl::vec3> tree_position;

    std::vector<vcl::mesh_drawable> trees;

    void update_tree_position(std::vector<struct colline> collines);
    vcl::mesh create_terrain();

};

struct colline {
  vcl::vec2 center;
  float height;
  float sigma;
};

vcl::mesh create_tree();
vcl::mesh create_tree_foliage(float radius, float height, float z_offset, float startHeight);
vcl::mesh create_cone(float radius, float height, float z_offset);
vcl::mesh create_cylinder(float radius, float height);

#endif
