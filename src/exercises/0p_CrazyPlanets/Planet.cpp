#include "Planet.hpp"
#include <random>
#include <time.h>

#ifdef INF443_0P_CRAZYPLANETS
// std::uniform_real_distribution<float> distrib(0.0,1.0);
std::random_device rd2;
std::default_random_engine generator2(rd2());

Planet::Planet(float height, float radius, int octave, float persistency, float freq_gain, int precision) {
    planet_cpu.texture_uv.resize(precision * precision);
    for(int ku=0; ku<precision; ++ku) {
        for(int kv=0; kv<precision; ++kv) {
            const float u = static_cast<float>(ku)/static_cast<float>(precision-1);
            const float v = static_cast<float>(kv)/static_cast<float>(precision);
  
            const float theta = static_cast<float>( 3.14159f*u );
            const float phi   = static_cast<float>( 2*3.14159f*v );
  
            const float tmpX = std::sin(theta) * std::cos(phi);
            const float tmpY = std::sin(theta) * std::sin(phi);
            const float tmpZ = std::cos(theta);
  
            const float x = (radius + height * vcl::perlin(tmpX, tmpY, tmpZ, octave, persistency, freq_gain)) * std::sin(theta) * std::cos(phi);
            const float y = (radius + height * vcl::perlin(tmpX, tmpY, tmpZ, octave, persistency, freq_gain)) * std::sin(theta) * std::sin(phi);
            const float z = (radius + height * vcl::perlin(tmpX, tmpY, tmpZ, octave, persistency, freq_gain)) * std::cos(theta);
  
            const vcl::vec3 p = {x,y,z};
            // const vcl::vec3 n = normalize(p);
  
            planet_cpu.position.push_back(p);
            // planet_cpu.normal.push_back(n);

            planet_cpu.texture_uv[kv+precision*ku] = {5*u, 5*v};

        }
    }

    planet_cpu.connectivity = vcl::connectivity_grid(precision, precision, false, true);
}

vcl::mesh_drawable Planet::planet_gpu() {
    vcl::mesh_drawable p_gpu = planet_cpu;
    return p_gpu;
}


void generate_planet_positions(std::vector<vcl::vec3> &plan_pos){
  //seed rng
  srand(time(0));
  std::uniform_int_distribution<int> uni(40, 200);
  int n_tree = uni(generator2);
  std::cout << n_tree << " " << std::endl;
  for (int i=0; i<n_tree; i++){
    bool ok = false;
    float u, v, w;
    while(!ok) {
      u=((static_cast <float>(rand())/static_cast <float>(RAND_MAX)));
      v=((static_cast <float>(rand())/static_cast <float>(RAND_MAX)));
      w=((static_cast <float>(rand())/static_cast <float>(RAND_MAX)));
      u=100*(u-0.5f);
      v=100*(v-0.5f);
      w=100*(w-0.5f);
      
      ok = true;
      /*for (vec3 tp : scene_exercise::tree_position) {
        if (pow((tp[0] - u), 2) + pow((tp[1] - v), 2) < 100) {
          ok = false;
        }
      }*/
    }
    plan_pos.push_back((vcl::vec3){u, v, w});
  }
}

void generate_swarm(std::vector<vcl::mesh_drawable> &plan_swarm, Planet (*planet_generator)()) {
    std::vector<vcl::vec3> plan_pos;
    generate_planet_positions(plan_pos);
    for (vcl::vec3 pp : plan_pos) {
        plan_swarm.push_back(planet_generator().planet_gpu());
        plan_swarm.back().uniform_parameter.translation = pp;
        plan_swarm.back().uniform_parameter.shading.specular = 0.0f; // non-specular terrain material
    }
}


#endif