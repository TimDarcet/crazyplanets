
#include "0p_CrazyPlanets.hpp"
#include "Planet.hpp"
#include "skybox.hpp"
#include "bird.hpp"

//#include <stdio.h>

#ifdef INF443_0P_CRAZYPLANETS
#define PI 3.14159265
#include <random>
#include <time.h>
#include <string>

#define BIRD_PRECISION 20

// Add vcl namespace within the current one - Allows to use function from vcl library without explicitely preceeding their name with vcl::
using namespace vcl;

// Uniform distribution in [0,1]
std::uniform_real_distribution<float> distrib(0.0,1.0);
std::random_device rd;
std::default_random_engine generator(rd());

float evaluate_terrain_z(float u, float v, std::vector<struct colline> collines);
vec3 evaluate_terrain(float u, float v, std::vector<struct colline> collines);

// Colors
vec3 green = {0.6f,0.85f,0.5f};
vec3 brown = {0.25f,0.1f,0.1f};

// Textures
char *asteroid_texture = "data/asteroid.png";
char *skybox_texture = "data/space_skybox_big.png";
char *mPlanet_texture = "data/grass.png";
/** This function is called before the beginning of the animation loop
    It is used to initialize all part-specific data */
void scene_exercise::setup_data(std::map<std::string,GLuint>& , scene_structure& scene, gui_structure& )
{
    timer.scale = 0.5f;
    // Create visual terrain surface
    update_terrain();
    asteroid_texture_id = texture_gpu(image_load_png(asteroid_texture));
    mPlanet_texture_id = texture_gpu(image_load_png(mPlanet_texture));
  
    update_tree_position();
    for (vcl::vec3 tp : tree_position) {
      trees.push_back(/*(vcl::mesh_drawable)*/ create_tree());
      trees.back().uniform_parameter.translation += tp - vec3({0, 0, 0.1});
      trees.back().uniform_parameter.shading.specular = 0.0f; // non-specular terrain material
      trees.back().uniform_parameter.scaling = 10;
      trees.back().uniform_parameter.rotation = rotation_between_vector_mat3(vec3{0,0,1}, tp);
    }

    // float shape[3];
    // shape[0] = 0.1f;
    // shape[1] = 0.4f;
    // shape[2] = 0.2f;
    // Cloud nuage(10, shape);
    // nuage_gpu = nuage.surfaces_gpu();
    // for (int i = 0; i < nuage.nb_surfaces; i++) {
    //     nuage_gpu[i].uniform_parameter.shading = {1,0,0}; // set pure ambiant component (no diffuse, no specular) - allow to only see the color of the texture
    // }

    // // Load a texture (with transparent background)
    // texture_id = texture_gpu( image_load_png("data/cloud.png") );


    // Setup initial camera mode and position
    scene.camera.camera_type = camera_control_spherical_coordinates;
    scene.camera.scale = 10.0f;
    scene.camera.apply_rotation(0,0,0,1.2f);

    skybox = create_skybox();
    skybox.uniform_parameter.shading = {1,0,0};
    skybox.uniform_parameter.rotation = rotation_from_axis_angle_mat3({1,0,0},-3.014f/2.0f);
    texture_skybox = texture_gpu(image_load_png(skybox_texture));

    generate_swarm(planets, &asteroid_generator);

    bird=create_bird(BIRD_PRECISION);
    update_trajectory();
    timer.t = trajectory.time[1];
    timer.t_min = trajectory.time[1];
    timer.t_max = trajectory.time[trajectory.time.size()-2];
}


/** This function is called at each frame of the animation loop.
    It is used to compute time-varying argument and perform data data drawing */
void scene_exercise::frame_draw(std::map<std::string,GLuint>& shaders, scene_structure& scene, gui_structure& )
{
    set_gui();
    timer.update();

    const float t = timer.t;

    glEnable( GL_POLYGON_OFFSET_FILL ); // avoids z-fighting when displaying wireframe


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Display terrain
    glPolygonOffset(1.0, 1.0);
    if (gui_scene.main_planet) {
      glBindTexture(GL_TEXTURE_2D, mPlanet_texture_id);
      terrain.draw(shaders["mesh"], scene.camera);
    }
    if (gui_scene.swarm){
      glBindTexture(GL_TEXTURE_2D, asteroid_texture_id);
      for (vcl::mesh_drawable p : planets) {
        p.draw(shaders["mesh"], scene.camera);
        std::cout << p.uniform_parameter.translation << std::endl;
      }
    }
    glBindTexture(GL_TEXTURE_2D, scene.texture_white);

    for (mesh_drawable t : trees) {
      t.draw(shaders["mesh"], scene.camera);
    }

    display_skybox(shaders, scene);

    display_bird(shaders, scene);
    
    if( gui_scene.wireframe ){ // wireframe if asked from the GUI
        glPolygonOffset( 1.0, 1.0 );
        terrain.draw(shaders["wireframe"], scene.camera);
        for (mesh_drawable t : trees) {
          t.draw(shaders["wireframe"], scene.camera);
        }
    }
    
}


void scene_exercise::update_tree_position(){
  std::uniform_int_distribution<int> uni(2, 10);
  int n_tree = uni(generator);
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
    scene_exercise::tree_position.push_back({gui_scene.radius * std::sin(PI*u) * std::cos(2*PI*v),
                                            gui_scene.radius * std::sin(PI*u) * std::sin(2*PI*v),
                                            gui_scene.radius * std::cos(PI*u)});
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

    // scene_exercise::update_tree_position(collines);

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

Planet asteroid_generator() {
  int precision = 100;
  float height = 0.8f;
  float radius = 1.0f;
  int octave = 7;
  float persistency = 0.4f;
  float freq_gain = 2;
  Planet p(height, radius, octave, persistency, freq_gain, precision);
  return p;
}

void scene_exercise::update_terrain() {
  Planet p(gui_scene.height, gui_scene.radius, gui_scene.octave, gui_scene.persistency, gui_scene.freq_gain, gui_scene.precision);
  terrain = p.planet_gpu();
  terrain.uniform_parameter.color = green;
  terrain.uniform_parameter.shading.specular = 0.0f; // non-specular terrain material
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


void scene_exercise::display_bird(std::map<std::string,GLuint>& shaders, scene_structure& scene)
{
    const float t = timer.t;
    const vec3 p = cardinal_spline_interpolation(trajectory, t);
    const vec3 d = normalize(cardinal_spline_derivative_interpolation(trajectory, t));


    mat3 R = rotation_between_vector_mat3({1,0,0},d);

    // up vector
    const vec3 up = {0,0,1};
    const vec3 up_proj = up-dot(up,d)*d;
    const vec3 new_up = R*vec3{0,0,1};

    const mat3 twist = rotation_between_vector_mat3(new_up,up_proj);
    R = twist*R;

    const float theta = std::cos(7*3.14f*timer.t);

    bird.rotation("head") = rotation_from_axis_angle_mat3({0,1,0}, std::cos(2.0f*3.14f*timer.t)/5.0f);
    move_sheet_wings(bird, t, BIRD_PRECISION, [](float t, float x){return (float) 0.2f*(float)sin(2*PI*t)*(float)sin(0.8f*PI*x) + (float)x * (float)sin(2*PI*t+PI/2);});
    bird.translation("body") = p;
    bird.rotation("body") = R;
    bird.draw(shaders["mesh"], scene.camera);
}



void scene_exercise::update_trajectory()
{
    const size_t N = 12;
    const float r = 16.0f;
    for(size_t k=0; k<N; ++k)
    {
        const float u = k%(N-3)/(N-3.0f);
        const vec3 p = {r*std::cos(2*PI*u),r*std::sin(2*PI*u), 10+5.0f*std::cos(4*PI*u)};
        trajectory.position.push_back(p);
    }

    update_time_trajectory();

}

void scene_exercise::update_time_trajectory()
{
    const float max_time = 10.0f;

    const size_t N = trajectory.position.size();
    trajectory.time.resize(N);

    float length = 0.0f;
    for(size_t k=0; k<N-1; ++k)
    {
        const vec3& p0 = trajectory.position[k];
        const vec3& p1 = trajectory.position[k+1];
        const float L = norm(p1-p0);
        length += L;
    }

    trajectory.time[0] = 0.0f;
    for(size_t k=1; k<N-1; ++k)
    {
        const vec3& p0 = trajectory.position[k-1];
        const vec3& p1 = trajectory.position[k];
        const float L = norm(p1-p0);

        trajectory.time[k] = trajectory.time[k-1]+L/length * max_time;
    }
    trajectory.time[N-1]=max_time;

    timer.t_min = trajectory.time[1];
    timer.t_max = trajectory.time[trajectory.time.size()-2];


}

vec3 cardinal_spline_interpolation(float t, float t0, float t1, float t2, float t3, const vec3& p0, const vec3& p1, const vec3& p2, const vec3& p3)
{
    const float sigma = t2-t1;

    const vec3 d1 = (p2-p0)/(t2-t0) * sigma;
    const vec3 d2 = (p3-p1)/(t3-t1) * sigma;

    const float s = (t-t1)/sigma;
    const float s2 = s*s;
    const float s3 = s2*s;

    const vec3 p = (2*s3-3*s2+1)*p1 + (s3-2*s2+s)*d1 + (-2*s3+3*s2)*p2 + (s3-s2)*d2;

    return p;
}
vec3 cardinal_spline_derivative_interpolation(float t, float t0, float t1, float t2, float t3, const vec3& p0, const vec3& p1, const vec3& p2, const vec3& p3)
{
    const float sigma = t2-t1;

    const vec3 d1 = (p2-p0)/(t2-t0) * sigma;
    const vec3 d2 = (p3-p1)/(t3-t1) * sigma;

    const float s = (t-t1)/sigma;
    const float s2 = s*s;

    const vec3 p = (6*s2-6*s)*p1 + (3*s2-4*s+1)*d1 + (-6*s2+6*s)*p2 + (3*s2-2*s)*d2;

    return p;
}
vec3 cardinal_spline_interpolation(const trajectory_structure& trajectory, float t)
{
    const size_t idx = index_at_value(t, trajectory.time);

    const float t0 = trajectory.time[idx-1];
    const float t1 = trajectory.time[idx];
    const float t2 = trajectory.time[idx+1];
    const float t3 = trajectory.time[idx+2];

    const vec3& p0 = trajectory.position[idx-1];
    const vec3& p1 = trajectory.position[idx];
    const vec3& p2 = trajectory.position[idx+1];
    const vec3& p3 = trajectory.position[idx+2];

    //const vec3 p = linear_interpolation(t,t1,t2,p1,p2);
    const vec3 p = cardinal_spline_interpolation(t,t0,t1,t2,t3,p0,p1,p2,p3);
    return p;
}
vec3 cardinal_spline_derivative_interpolation(const trajectory_structure& trajectory, float t)
{
    const size_t idx = index_at_value(t, trajectory.time);

    const float t0 = trajectory.time[idx-1];
    const float t1 = trajectory.time[idx];
    const float t2 = trajectory.time[idx+1];
    const float t3 = trajectory.time[idx+2];

    const vec3& p0 = trajectory.position[idx-1];
    const vec3& p1 = trajectory.position[idx];
    const vec3& p2 = trajectory.position[idx+1];
    const vec3& p3 = trajectory.position[idx+2];

    const vec3 p = cardinal_spline_derivative_interpolation(t,t0,t1,t2,t3,p0,p1,p2,p3);
    return p;
}

size_t index_at_value(float t, const std::vector<float>& vt)
{
    const size_t N = vt.size();
    assert(vt.size()>=2);
    assert(t>=vt[0]);
    assert(t<vt[N-1]);

    size_t k=0;
    while( vt[k+1]<t )
        ++k;
    return k;
}

void scene_exercise::set_gui()
{
    ImGui::Checkbox("Wireframe", &gui_scene.wireframe);
    ImGui::Checkbox("Skybox", &gui_scene.skybox);
    ImGui::Checkbox("Multiple planets", &gui_scene.swarm);
    ImGui::Checkbox("Main planet", &gui_scene.main_planet);
    
    
    if(gui_scene.main_planet) {
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
}



#endif