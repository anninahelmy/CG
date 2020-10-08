//=============================================================================
//
//   Exercise code for the lecture
//   "Introduction to Computer Graphics"
//   by Prof. Dr. Mario Botsch, Bielefeld University
//
//   Copyright (C) Computer Graphics Group, Bielefeld University.
//
//=============================================================================

//== INCLUDES =================================================================
#include "Scene.h"

#include "Plane.h"
#include "Sphere.h"
#include "Cylinder.h"
#include "Mesh.h"

#include <limits>
#include <map>
#include <functional>
#include <stdexcept>

#ifdef _OPENMP
#  include <omp.h>
#endif

//-----------------------------------------------------------------------------


Image Scene::render()
{
    // allocate new image.
    Image img(camera.width, camera.height);

    // Function rendering a full column of the image
    auto raytraceColumn = [&img, this](int x) {
        for (int y=0; y<int(camera.height); ++y)
        {
            Ray ray = camera.primary_ray(x,y);

            // compute color by tracing this ray
            vec3 color = trace(ray, 0);

            // avoid over-saturation
            color = min(color, vec3(1, 1, 1));

            // store pixel color
            img(x,y) = color;
        }
    };

    // If possible, raytrace image columns in parallel.
    //
    // Mac users beware:
    // XCode's compiler (based on clang) does not support OpenMP
    // - you will have to install a vanialla clang (or gcc) if you want to use
    // OpenMP parallelization, e.g. via MacPorts.

#ifdef _OPENMP
    std::cout << "Rendering with " << omp_get_num_threads() << " threads." << std::endl;
#  pragma omp parallel for
#else
    std::cout << "Rendering singlethreaded." << std::endl;
#endif
    for (int x=0; x<int(camera.width); ++x)
        raytraceColumn(x);

    // Note: compiler will elide copy.
    return img;
}

//-----------------------------------------------------------------------------

vec3 Scene::trace(const Ray& _ray, int _depth)
{
    // stop if recursion depth (=number of reflection) is too large
    if (_depth > max_depth) return vec3(0,0,0);

    // Find first intersection with an object. If an intersection is found,
    // it is stored in object, point, normal, and t.
    Object_ptr  object;
    vec3        point;
    vec3        normal;
    double      t;
    if (!intersect(_ray, object, point, normal, t))
    {
        return background;
    }

    // compute local Phong lighting (ambient+diffuse+specular)
    vec3 color = lighting(point, normal, -_ray.direction, object->material);

    /** \todo
     * Compute reflections by recursive ray tracing:
     * - generate reflected ray, compute its color contribution, and mix it with
     * the color computed by local Phong lighting (use `object->material.mirror` as weight)
     * - check whether your recursive algorithm reflects the ray `max_depth` times
     */
    // - check whether `object` is reflective by checking its `material.mirror` and checking recursion depth
    if(_depth<max_depth&&object->material.mirror>0.0){
        //generate reflected ray
    }

    return color;
}

//-----------------------------------------------------------------------------

bool Scene::intersect(const Ray& _ray, Object_ptr& _object, vec3& _point, vec3& _normal, double& _t)
{
    double  t, tmin(Object::NO_INTERSECTION);
    vec3    p, n;

    for (const auto &o: objects) // for each object
    {
        if (o->intersect(_ray, p, n, t)) // does ray intersect object?
        {
            if (t < tmin) // is intersection point the currently closest one?
            {
                tmin = t;
                _object = o.get();
                _point  = p;
                _normal = n;
                _t      = t;
            }
        }
    }

    return (tmin != Object::NO_INTERSECTION);
}

vec3 Scene::lighting(const vec3& _point, const vec3& _normal, const vec3& _view, const Material& _material)
{   /** \todo
     * Compute the Phong lighting:
     * - start with global ambient contribution
    **/
    vec3 light_color = _material.ambient * ambience; //ambient light color: (material ambient refelction coefficient)*(ambient light intensity for scene)

    //for each (add for loop!) light source (vec lights) add diffuse and specular contribution

    for(const Light& _light:lights){
        vec3 l_direction = normalize(_light.position - _point);

        //check whether object is in shadow
        //shadow ray from intersection point to light source;
        // discard diffuse and specular contribution if light source is blocked by another object
        Ray shadow = (_point, l_direction);
        //discard secondary intersection points that are too close to the light source
        Object_ptr  object;
        vec3        point;
        vec3        normal;
        double      t;
        double l_distance = distance(_point, _light.position);
        if (t < l_distance && intersect(shadow, object, point, normal, t)){
            continue;}
        /*diffusion: I_l*m_d(n\dot l)where
         * I_l is the intensity of light source,
         * m_d is materials diffuse reflection coefficient.
         * light direction l and normalized vector n
         **/
        if (dot(l_direction, _normal) > 0.0) { //add if statement, if n*l<0 then the light source is behind the plane (siehe Vorlesung)
            light_color = light_color + _material.diffuse * _light.color * dot(l_direction, _normal);
            /**need to add specular light: I_l*m_s(r\dotv) with
             * I_l is the intensity of light source,
             * m_s the materials specular reflection coefficient
             * r reflected ray = 2n(n\dot l) - l
             * v refracted ray
             */
            vec3 reflected_ray = normalize(2 * _normal * (dot(_normal, l_direction)) - l_direction);
            vec3 refracted_ray = normalize(mirror(l_direction, _normal));
            if(dot(reflected_ray, refracted_ray) > 0.0) {//siehe Vorlesung, wenn kleiner als 0 wäre, dann würde man von der anderen Seite auf Ebene schauen{
                light_color = light_color + _light.color * _material.specular *
                                            pow(dot(reflected_ray, refracted_ray), _material.shininess);
            }
        }
    }
    return light_color;
}

//-----------------------------------------------------------------------------

void Scene::read(const std::string &_filename)
{
    std::ifstream ifs(_filename);
    if (!ifs)
        throw std::runtime_error("Cannot open file " + _filename);

    const std::map<std::string, std::function<void(void)>> entityParser = {
            {"depth",      [&]() { ifs >> max_depth; }},
            {"camera",     [&]() { ifs >> camera; }},
            {"background", [&]() { ifs >> background; }},
            {"ambience",   [&]() { ifs >> ambience; }},
            {"light",      [&]() { lights .emplace_back(ifs); }},
            {"plane",      [&]() { objects.emplace_back(new    Plane(ifs)); }},
            {"sphere",     [&]() { objects.emplace_back(new   Sphere(ifs)); }},
            {"cylinder",   [&]() { objects.emplace_back(new Cylinder(ifs)); }},
            {"mesh",       [&]() { objects.emplace_back(new     Mesh(ifs, _filename)); }}
    };

    // parse file
    std::string token;
    while (ifs && (ifs >> token) && (!ifs.eof())) {
        if (token[0] == '#') {
            ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        if (entityParser.count(token) == 0)
            throw std::runtime_error("Invalid token encountered: " + token);
        entityParser.at(token)();
    }
}


//=============================================================================