
#include "test_cloud.hpp"
#include "cloud.hpp"

#ifdef TEST_CLOUD

// Add vcl namespace within the current one - Allows to use function from vcl library without explicitely preceeding their name with vcl::
using namespace vcl;


/** This function is called before the beginning of the animation loop
    It is used to initialize all part-specific data */
void scene_exercise::setup_data(std::map<std::string,GLuint>& , scene_structure& , gui_structure& )
{
    float shape[3];
    shape[0] = 0.1f;
    shape[1] = 0.4f;
    shape[2] = 0.2f;
    nuage = Cloud(10, shape);
    nuage_gpu = nuage.surfaces_gpu();
    for (int i = 0; i < nuage.nb_surfaces; i++) {
        nuage_gpu[i].uniform_parameter.shading = {1,0,0}; // set pure ambiant component (no diffuse, no specular) - allow to only see the color of the texture
    }

    // Load a texture (with transparent background)
    texture_id = texture_gpu( image_load_png("data/cloud.jpg") );

}



/** This function is called at each frame of the animation loop.
    It is used to compute time-varying argument and perform data data drawing */
void scene_exercise::frame_draw(std::map<std::string,GLuint>& shaders, scene_structure& scene, gui_structure& )
{
    set_gui();


    // Enable use of alpha component as color blending for transparent elements
    //  new color = previous color + (1-alpha) current color
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Disable depth buffer writing
    //  - Transparent elements cannot use depth buffer
    //  - They are supposed to be display from furest to nearest elements
    glDepthMask(false);


    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // avoids sampling artifacts
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // avoids sampling artifacts

    // Display a billboard always facing the camera direction
    // ********************************************************** //
    for (int i = 0; i < nuage.nb_surfaces; i++) {
        nuage_gpu[i].uniform_parameter.rotation = scene.camera.orientation;
        nuage_gpu[i].uniform_parameter.translation = {0.25f,0,-0.5f};
        nuage_gpu[i].draw(shaders["mesh"], scene.camera);
        if(gui_scene.wireframe)
            nuage_gpu[i].draw(shaders["wireframe"], scene.camera);
    }

    glBindTexture(GL_TEXTURE_2D, scene.texture_white);
    glDepthMask(true);

}




void scene_exercise::set_gui()
{
    ImGui::Checkbox("Wireframe", &gui_scene.wireframe);
};



#endif

