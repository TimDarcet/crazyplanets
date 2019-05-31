
#include "0p_CrazyPlanets.hpp"
#include "Planet.hpp"
#include "skybox.hpp"
//#include <stdio.h>

#ifdef INF443_0P_CRAZYPLANETS
#define PI 3.14159265
#include <random>
#include <time.h>

// Add vcl namespace within the current one - Allows to use function from vcl library without explicitely preceeding their name with vcl::
using namespace vcl;

// Uniform distribution in [0,1]
std::uniform_real_distribution<float> distrib(0.0,1.0);
std::random_device rd;
std::default_random_engine generator(rd());

float evaluate_terrain_z(float u, float v, std::vector<struct colline> collines);
vec3 evaluate_terrain(float u, float v, std::vector<struct colline> collines);


vec3 green = {0.6f,0.85f,0.5f};
vec3 brown = {0.25f,0.1f,0.1f};



/** This function is called before the beginning of the animation loop
    It is used to initialize all part-specific data */
void scene_exercise::setup_data(std::map<std::string,GLuint>& , scene_structure& scene, gui_structure& )
{
    // Create visual terrain surface
    update_terrain();
    texture_id = texture_gpu(image_load_png("data/grass.png"));
  
    // for (vcl::vec3 tp : tree_position) {
    //   trees.push_back(/*(vcl::mesh_drawable)*/ create_tree());
    //   trees.back().uniform_parameter.translation += tp - vec3({0, 0, 0.1});
    //   trees.back().uniform_parameter.shading.specular = 0.0f; // non-specular terrain material
    //   trees.back();
    // }  texture_id = texture_gpu(image_load_png("data/grass.png"));



    // Setup initial camera mode and position
    scene.camera.camera_type = camera_control_spherical_coordinates;
    scene.camera.scale = 10.0f;
    scene.camera.apply_rotation(0,0,0,1.2f);

    skybox = create_skybox();
    skybox.uniform_parameter.shading = {1,0,0};
    skybox.uniform_parameter.rotation = rotation_from_axis_angle_mat3({1,0,0},-3.014f/2.0f);
    texture_skybox = texture_gpu(image_load_png("data/space_skybox_big.png"));
}

void scene_exercise::update_tree_position(std::vector<struct colline> collines){
  std::uniform_int_distribution<int> uni(40, 250);
  int n_tree = uni(generator);
  std::cout << n_tree << " " << std::endl;
  for (int i=0; i<n_tree; i++){
    bool ok = false;
    float u, v;
    while(!ok) {
      u=distrib(generator);
      v=distrib(generator);

      ok = true;
      /*for (vec3 tp : scene_exercise::tree_position) {
        if (pow((tp[0] - u), 2) + pow((tp[1] - v), 2) < 100) {
          ok = false;
        }
      }*/
    }
    scene_exercise::tree_position.push_back(evaluate_terrain(u, v, collines));
  }
}

/** This function is called at each frame of the animation loop.
    It is used to compute time-varying argument and perform data data drawing */
void scene_exercise::frame_draw(std::map<std::string,GLuint>& shaders, scene_structure& scene, gui_structure& )
{
    set_gui();

    glEnable( GL_POLYGON_OFFSET_FILL ); // avoids z-fighting when displaying wireframe


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Display terrain
    glPolygonOffset(1.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    terrain.draw(shaders["mesh"], scene.camera);
    glBindTexture(GL_TEXTURE_2D, scene.texture_white);

    // for (mesh_drawable t : trees) {
    //   t.draw(shaders["mesh"], scene.camera);
    // }

    display_skybox(shaders, scene);

    if( gui_scene.wireframe ){ // wireframe if asked from the GUI
        glPolygonOffset( 1.0, 1.0 );
        terrain.draw(shaders["wireframe"], scene.camera);
        for (mesh_drawable t : trees) {
          t.draw(shaders["wireframe"], scene.camera);
        }
    }
}



// Evaluate height of the terrain for any (u,v) \in [0,1]
float evaluate_terrain_z(float u, float v, std::vector<struct colline> collines)
{
    float d;
    float z=0;
    for (struct colline c : collines) {
      d = norm(vec2(u,v)-c.center)/c.sigma;
      z += c.height*std::exp(-d*d);
    }
    return z;
}

// Evaluate 3D position of the terrain for any (u,v) \in [0,1]
vec3 evaluate_terrain(float u, float v, std::vector<struct colline> collines)
{
    const float scaling = 3.0f;
    const int octave = 7;
    const float persistency = 0.4f;
    const float perlin_strength = 0.5f;

    const float x = 20*(u-0.5f);
    const float y = 20*(v-0.5f);
    const float z = evaluate_terrain_z(u, v, collines) + perlin_strength * perlin(scaling*u, scaling*v, octave, persistency);

    return {x,y,z};
}

// Generate terrain mesh
mesh scene_exercise::create_terrain()
{
    // Number of samples of the terrain is N x N
    const size_t N = 1000;
    const int texture_scale = 10;

    //seed rng
    srand(time(0));

    // Number of hills
    const int n_collines = 10;
    // Create random hills
    std::vector<struct colline> collines = {};
    int i;
    struct colline tmp={{0, 0}, 0, 0};
    for (i=0; i<n_collines; i++){
      tmp.center={static_cast <float> (rand()) / static_cast <float> (RAND_MAX),static_cast <float> (rand()) / static_cast <float> (RAND_MAX)};
      tmp.height=2*((static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) - 0.2f);
      tmp.sigma=0.5*((static_cast <float> (rand()) / static_cast <float> (RAND_MAX))+0.2f);
      collines.push_back(tmp);
    }

    mesh terrain; // temporary terrain storage (CPU only)
    terrain.position.resize(N*N);

    // Fill terrain geometry
    for(size_t ku=0; ku<N; ++ku)
    {
        for(size_t kv=0; kv<N; ++kv)
        {
            // Compute local parametric coordinates (u,v) \in [0,1]
            const float u = ku/(N-1.0f);
            const float v = kv/(N-1.0f);

            terrain.texture_uv.push_back({texture_scale * u, texture_scale * v});
            // Compute coordinates
            terrain.position[kv+N*ku] = evaluate_terrain(0.001 * distrib(generator) + u, 0.001 * distrib(generator) + v, collines);
        }
    }


    // Generate triangle organization    terrain.uniform_parameter.color = green;

    //  Parametric surface with uniform grid sampling: generate 2 triangles for each grid cell
    const unsigned int Ns = N;
    for(unsigned int ku=0; ku<Ns-1; ++ku)
    {
        for(unsigned int kv=0; kv<Ns-1; ++kv)
        {
            const unsigned int idx = kv + N*ku; // current vertex offset

            const index3 triangle_1 = {idx, idx+1+Ns, idx+1};
            const index3 triangle_2 = {idx, idx+Ns, idx+1+Ns};

            terrain.connectivity.push_back(triangle_1);
            terrain.connectivity.push_back(triangle_2);
        }
    }

    scene_exercise::update_tree_position(collines);

    return terrain;
}

// create a cylinder
mesh create_cylinder(float radius, float height) {
    const unsigned int precision = 20;
    mesh cyl;
    for (unsigned int i=0; i<precision; i++){
      cyl.position.push_back({(float)(radius*cos(2*PI*i/precision)), (float)(radius*sin(2*PI*i/precision)),0.0f});
      cyl.position.push_back({(float)(radius*cos(2*PI*i/precision)), (float)(radius*sin(2*PI*i/precision)),height});
      cyl.connectivity.push_back({2*i, 2*i+1, (2*i+2)%(2*precision)});
      cyl.connectivity.push_back({2*i+1, (2*i+2)%(2*precision), (2*i+3)%(2*precision)});
    }
    return cyl;
}

// create a cone
mesh create_cone(float radius, float height, float z_offset) {
    const unsigned int precision = 20;
    mesh c;
    c.position.push_back({0, 0, z_offset+height});
    c.position.push_back({0, 0, z_offset});
    for (unsigned int i=2; i<precision+2; i++){
      c.position.push_back({(float)(radius*cos(2*PI*i/precision)), (float)(radius*sin(2*PI*i/precision)), z_offset});
      if (i == precision+1){
        c.connectivity.push_back({i, 2, 0});
        c.connectivity.push_back({i, 2, 1});
      }
      else {
        c.connectivity.push_back({i, i+1, 0});
        c.connectivity.push_back({i, i+1, 1});
      }
    }
    return c;
}

mesh create_tree_foliage(float radius, float height, float z_offset, float startHeight)
{
    mesh m = create_cone(radius, height, startHeight);
    m.push_back(create_cone(radius, height, startHeight+z_offset) );
    m.push_back(create_cone(radius, height, startHeight+2*z_offset) );
    m.fill_color_uniform(green);
    return m;
}

mesh create_tree()
{
    float height = 0.4;//*distrib(generator);
    float r = 0.1;//*distrib(generator);
    mesh cyl = create_cylinder(r, height);
    cyl.fill_color_uniform(brown);
    float fr = (1.5 + 1 * distrib(generator))*r;
    mesh fol = create_tree_foliage(fr, height, fr / 1.7, height);
    cyl.push_back(fol);
    return cyl;
}

void scene_exercise::update_terrain() {
  Planet p(gui_scene.height, gui_scene.radius, gui_scene.octave, gui_scene.persistency, gui_scene.freq_gain, gui_scene.precision);
  terrain = p.planet_gpu();
  // terrain.uniform_parameter.color = green;
  // terrain.uniform_parameter.shading.specular = 0.0f; // non-specular terrain material
  // Load a texture image on GPU and stores its ID
}

void scene_exercise::display_skybox(std::map<std::string,GLuint>& shaders, scene_structure& scene)
{
    if(gui_scene.skybox)
    {
        glBindTexture(GL_TEXTURE_2D,texture_skybox);
        skybox.uniform_parameter.scaling = 150.0f;
        skybox.uniform_parameter.translation = scene.camera.camera_position() + vec3(0,0,-50.0f);
        skybox.draw(shaders["mesh"], scene.camera);
        glBindTexture(GL_TEXTURE_2D,scene.texture_white);
    }

}


void scene_exercise::set_gui()
{
    ImGui::Checkbox("Wireframe", &gui_scene.wireframe);
    ImGui::Checkbox("Skybox", &gui_scene.skybox);
    
    float radius_min = 0.1f;
    float radius_max = 10.0f;
    if( ImGui::SliderScalar("Radius", ImGuiDataType_Float, &gui_scene.radius, &radius_min, &radius_max) )
        update_terrain();

    int precision_min = 5;
    int precision_max = 500;
    if( ImGui::SliderScalar("precision", ImGuiDataType_S32, &gui_scene.precision, &precision_min, &precision_max) )
        update_terrain();
    
    ImGui::Separator();
    ImGui::Text("Perlin parameters");

    float height_min = 0.1f;
    float height_max = 5.0f;
    if( ImGui::SliderScalar("Height", ImGuiDataType_Float, &gui_scene.height, &height_min, &height_max) )
        update_terrain();

    int octave_min = 1;
    int octave_max = 10;
    if( ImGui::SliderScalar("Octave", ImGuiDataType_S32, &gui_scene.octave, &octave_min, &octave_max) )
        update_terrain();

    float persistency_min = 0.1f;
    float persistency_max = 2.0f;
    if( ImGui::SliderScalar("Persistency", ImGuiDataType_Float, &gui_scene.persistency, &persistency_min, &persistency_max) )
        update_terrain();

    float freq_gain_min = 0.1f;
    float freq_gain_max = 10.0f;
    if( ImGui::SliderScalar("Frequency gain", ImGuiDataType_Float, &gui_scene.freq_gain, &freq_gain_min, &freq_gain_max) )
        update_terrain();
}



#endif