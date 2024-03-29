#include "mesh_primitive.hpp"

#include "vcl/core/math/math.hpp"

namespace vcl
{

std::vector<index3> connectivity_grid(size_t Nu, size_t Nv, bool periodic_u, bool periodic_v)
{
    std::vector<std::array<unsigned int,3> > triangle_index;
    for(size_t ku=0; ku<Nu-1; ++ku) {
        for( size_t kv=0; kv<Nv-1; ++kv) {

            const unsigned int u00 = static_cast<unsigned int>( kv + Nv*ku );
            const unsigned int u10 = static_cast<unsigned int>( kv+1 + Nv*ku );
            const unsigned int u01 = static_cast<unsigned int>( kv + Nv*(ku+1) );
            const unsigned int u11 = static_cast<unsigned int>( kv+1 + Nv*(ku+1) );

            triangle_index.push_back({u00,u10,u11});
            triangle_index.push_back({u11,u01,u00});
        }
    }

    if( periodic_u==true ) {
        for( size_t kv=0; kv<Nv-1; ++kv ) {

            const unsigned int u00 = static_cast<unsigned int>( kv + Nv*(Nu-1) );
            const unsigned int u10 = static_cast<unsigned int>( kv+1 + Nv*(Nu-1) );
            const unsigned int u01 = static_cast<unsigned int>( kv + Nv*0 );
            const unsigned int u11 = static_cast<unsigned int>( kv+1 + Nv*0 );

            triangle_index.push_back({u00,u10,u11});
            triangle_index.push_back({u11,u01,u00});
        }
    }

    if( periodic_v==true ) {
        for( size_t ku=0; ku<Nu-1; ++ku ) {

            const unsigned int u00 = static_cast<unsigned int>( Nv-1 + Nv*ku );
            const unsigned int u10 = static_cast<unsigned int>( 0 + Nv*ku );
            const unsigned int u01 = static_cast<unsigned int>( Nv-1 + Nv*(ku+1) );
            const unsigned int u11 = static_cast<unsigned int>( 0 + Nv*(ku+1) );

            triangle_index.push_back({u00,u10,u11});
            triangle_index.push_back({u11,u01,u00});
        }
    }

    if( periodic_u==true && periodic_v==true )
    {
        const unsigned int u00 = static_cast<unsigned int>( Nv-1 + Nv*(Nu-1) );
        const unsigned int u10 = static_cast<unsigned int>( 0 + Nv*(Nu-1) );
        const unsigned int u01 = static_cast<unsigned int>( Nv-1 + Nv*0 );
        const unsigned int u11 = static_cast<unsigned int>( 0 + Nv*0 );

        triangle_index.push_back({u00,u10,u11});
        triangle_index.push_back({u11,u01,u00});
    }

    return triangle_index;

}


mesh mesh_primitive_sphere(float radius, const vec3& p0, size_t Nu, size_t Nv)
{
    mesh shape;
    for( size_t ku=0; ku<Nu; ++ku ) {
        for( size_t kv=0; kv<Nv; ++kv ) {

            const float u = static_cast<float>(ku)/static_cast<float>(Nu-1);
            const float v = static_cast<float>(kv)/static_cast<float>(Nv);

            const float theta = static_cast<float>( 3.14159f*u );
            const float phi   = static_cast<float>( 2*3.14159f*v );

            const float x = radius * std::sin(theta) * std::cos(phi);
            const float y = radius * std::sin(theta) * std::sin(phi);
            const float z = radius * std::cos(theta);

            const vec3 p = {x,y,z};
            const vec3 n = normalize(p);

            shape.position.push_back( p+p0 );
            shape.normal.push_back( n );
        }
    }

    shape.connectivity = connectivity_grid(Nu, Nv, false, true);

    return shape;
}

mesh mesh_primitive_ellipsoid(float radius, const vec3& p0, const vec3& scale, size_t Nu, size_t Nv)
{
    mesh shape;
    for( size_t ku=0; ku<Nu; ++ku ) {
        for( size_t kv=0; kv<Nv; ++kv ) {

            const float u = static_cast<float>(ku)/static_cast<float>(Nu-1);
            const float v = static_cast<float>(kv)/static_cast<float>(Nv);

            const float theta = static_cast<float>( 3.14159f*u );
            const float phi   = static_cast<float>( 2*3.14159f*v );

            const float x = radius * std::sin(theta) * std::cos(phi);
            const float y = radius * std::sin(theta) * std::sin(phi);
            const float z = radius * std::cos(theta);

            vec3 p = {x,y,z};
            vec3 tmp1 = sqrt(1/(pow(x, 2) + pow(y, 2)))* (vec3){-y, x, 0};
            vec3 tmp2 = cross(p, tmp1);
            tmp1 = termP(tmp1, scale);
            tmp2 = termP(tmp2, scale);
            p = termP(p, scale);
            const vec3 n = normalize(2 * termP((vec3){pow(scale.x, -2),pow(scale.y, -2),pow(scale.z, -2)},p));

            shape.position.push_back( p+p0 );
            shape.normal.push_back( n );
        }
    }

    shape.connectivity = connectivity_grid(Nu, Nv, false, true);

    return shape;
}

mesh mesh_primitive_smooth_cyl(float radius, const vec3& p0, const vec3& p1, size_t Nu, size_t Nv)
{
    mesh cyl = mesh_primitive_cylinder(radius, p1, p0, Nu, Nv);
    cyl.push_back(mesh_primitive_sphere(radius, p0, Nu, Nv));
    cyl.push_back(mesh_primitive_sphere(radius, p1, Nu, Nv));

    return cyl;
}


mesh mesh_primitive_cylinder(float radius,  const vec3& p1, const vec3& p2, size_t Nu, size_t Nv)
{
    const vec3 p12 = p2-p1;
    const float L = norm(p12);
    const vec3 dir = p12/L;
    const mat3 R = rotation_between_vector_mat3({0,0,1},dir);

    mesh shape;
    for( size_t ku=0; ku<Nu; ++ku ) {
        for( size_t kv=0; kv<Nv; ++kv ) {

            const float u = static_cast<float>(ku)/static_cast<float>(Nu);
            const float v = static_cast<float>(kv)/static_cast<float>(Nv-1);

            const float theta = static_cast<float>( 2* 3.14159f * u );

            const float x = radius * std::sin(theta);
            const float y = radius * std::cos(theta);
            const float z = L * v;

            const vec3 p0 = {x,y,z};
            const vec3 p = R*p0+p1;
            const vec3 n = normalize(R*vec3{x,y,0.0f});

            shape.position.push_back(p);
            shape.normal.push_back(n);
        }
    }

    shape.connectivity = connectivity_grid(Nu, Nv, true, false);

    return shape;
}

mesh mesh_primitive_cone(float radius,  const vec3& p1, const vec3& p2, size_t Nu, size_t Nv)
{
    const vec3 p12 = p2-p1;
    const float L = norm(p12);
    const vec3 dir = p12/L;
    const mat3 R = rotation_between_vector_mat3({0,0,1},dir);

    mesh shape;
    for( size_t ku=0; ku<Nu; ++ku ) {
        for( size_t kv=0; kv<Nv; ++kv ) {

            const float u = static_cast<float>(ku)/static_cast<float>(Nu);
            const float v = static_cast<float>(kv)/static_cast<float>(Nv-1);

            const float theta = static_cast<float>( 2* 3.14159f * u );
            const float alpha = 1.0f-v;

            const float x = alpha*radius * std::sin(theta);
            const float y = alpha*radius * std::cos(theta);
            const float z = L * v;

            const vec3 p0 = {x,y,z};
            const vec3 p = R*p0+p1;
            const vec3 normal_d = R*vec3{x,y,0.0f};
            const float normal_n = norm(normal_d);
            vec3 n = normal_d;
            if( normal_n>1e-6f )
                n /= normal_n;
            else
                n = {0,0,1};


            shape.position.push_back( p );
            shape.normal.push_back( n );
        }
    }

    shape.connectivity = connectivity_grid(Nu, Nv, true, false);

    return shape;
}


mesh mesh_primitive_frame(float sphere_radius, float cylinder_radius, float cone_radius, float cone_length)
{
    mesh sphere = mesh_primitive_sphere(sphere_radius); sphere.fill_color_uniform({1,1,1});

    mesh cylinder_x = mesh_primitive_cylinder(cylinder_radius,{0,0,0},{1.0f-cone_length,0,0}); cylinder_x.fill_color_uniform({1,0,0});
    mesh cylinder_y = mesh_primitive_cylinder(cylinder_radius,{0,0,0},{0,1.0f-cone_length,0}); cylinder_y.fill_color_uniform({0,1,0});
    mesh cylinder_z = mesh_primitive_cylinder(cylinder_radius,{0,0,0},{0,0,1.0f-cone_length}); cylinder_z.fill_color_uniform({0,0,1});

    mesh cone_x = mesh_primitive_cone(cone_radius,{1.0f-cone_length,0,0},{1.0f,0,0});  cone_x.fill_color_uniform({1,0,0});
    mesh cone_y = mesh_primitive_cone(cone_radius,{0,1.0f-cone_length,0},{0,1.0f,0});  cone_y.fill_color_uniform({0,1,0});
    mesh cone_z = mesh_primitive_cone(cone_radius,{0,0,1.0f-cone_length},{0,0,1.0f});  cone_z.fill_color_uniform({0,0,1});

    mesh m;
    m.push_back(sphere);
    m.push_back(cylinder_x);
    m.push_back(cylinder_y);
    m.push_back(cylinder_z);
    m.push_back(cone_x);
    m.push_back(cone_y);
    m.push_back(cone_z);

    return m;
}



mesh mesh_primitive_quad(const vec3& p00, const vec3& p10, const vec3& p11, const vec3& p01)
{
    // Compute normal of the quadrangle (orthogonal to the two basis vectors p00->p01 and p00->p10)
    const vec3 n = normalize(cross(normalize(p10-p00), normalize(p01-p00)));

    // Fill per-vertex buffers
    mesh quad;
    quad.position     = {p00, p10, p11, p01};         // 3D-coordinates
    quad.normal       = {n,n,n,n};                    // same normal for all vertices
    quad.texture_uv   = {{0,0}, {1,0}, {1,1}, {0,1}}; // 2D (u,v) - texture coordinates
    quad.connectivity = {{0,1,2}, {0,2,3}};           // Quadrangle made up of two triangles

    return quad;
}

mesh mesh_primitive_disc(float radius, const vec3& p0, const vec3& n, size_t N)
{
    mesh disc;

    mat3 R = rotation_between_vector_mat3({0,0,1},n);

    for(size_t k=0; k<N; ++k)
    {
        const float u = static_cast<float>(k)/(static_cast<float>(N)-1.0f);
        const float theta = 2*static_cast<float>(3.14159f)*u;
        const vec3 p = R*vec3(radius*std::cos(theta),radius*std::sin(theta),0.0f);

        disc.position.push_back(p0+p);
        disc.normal.push_back(n);

        disc.connectivity.push_back( {static_cast<unsigned int>( k ),
                                      static_cast<unsigned int>((k+1)%N),
                                      static_cast<unsigned int>( N )});
    }

    // add central point
    disc.position.push_back(p0);
    disc.normal.push_back(n);

    return disc;

}

mesh mesh_primitive_grid(size_t Nu, size_t Nv, float size_u, float size_v, const vec3& p0, const vec3& n)
{
    const mat3 R = rotation_between_vector_mat3({0,0,1},n);

    mesh shape;
    for( size_t ku=0; ku<Nu; ++ku ) {
        for( size_t kv=0; kv<Nv; ++kv ) {

            const float u = static_cast<float>(ku)/static_cast<float>(Nu-1);
            const float v = static_cast<float>(kv)/static_cast<float>(Nv-1);

            const float x = size_u*u;
            const float y = size_v*v;
            const float z = 0.0f;

            const vec3 position = R*vec3{x,y,z}+p0;
            const vec3 normal = R*vec3{0,0,1};

            shape.position.push_back( position );
            shape.normal.push_back( normal );
            shape.texture_uv.push_back({u,v});
        }
    }

    shape.connectivity = connectivity_grid(Nu, Nv, false, false);

    return shape;

}



mesh mesh_primitive_parallelepiped(const vec3& p0, const vec3& u1, const vec3& u2, const vec3& u3)
{

    const vec3 p000 = p0;
    const vec3 p100 = p0+u1;
    const vec3 p010 = p0+u2;
    const vec3 p001 = p0+u3;
    const vec3 p110 = p0+u1+u2;
    const vec3 p101 = p0+u1+u3;
    const vec3 p011 = p0+u2+u3;
    const vec3 p111 = p0+u1+u2+u3;

    mesh shape;
    shape.position = {p000, p100, p110, p010,
                      p100, p101, p111, p110,
                      p110, p111, p011, p010,
                      p000, p001, p011, p010,
                      p000, p001, p101, p100,
                      p101, p001, p011, p111};




    const vec3 n1 = normalize(u1);
    const vec3 n2 = normalize(u2);
    const vec3 n3 = normalize(u3);

    shape.normal = {-n3, -n3, -n3, -n3,
                    n1, n1, n1, n1,
                    n2, n2, n2, n2,
                    -n1, -n1, -n1, -n1,
                    -n2, -n2, -n2, -n2,
                    n3, n3, n3, n3};

    shape.connectivity = {{0,1,2}, {0,2,3},
                          {4,5,6}, {4,6,7},
                          {8,9,10}, {8,10,11},
                          {12,13,14}, {12,14,15},
                          {16,17,18}, {16,18,19},
                          {20,21,22}, {20,22,23}};

    return shape;
}


}
