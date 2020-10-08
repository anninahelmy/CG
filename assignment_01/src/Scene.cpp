///=============================================================================
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

// add offset shadow and reflected ray emission points from surface intersection
constexpr double shadow_ray_offset = 1e-5;
constexpr double reflection_ray_offset = 1e-5;

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
    std::cout << "Rendering with up to " << omp_get_max_threads() << " threads." << std::endl;
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

    // - check whether `object` is reflective by checking its `material.mirror` and checking recursion depth
    /**Theory:
     * At each intersection point, reflect and/or refract incoming viewing ray at surface normal,
     * and trace child rays recursively.The final color is interpolated between local illumination,
     * reflection, and refraction based on material properties.
     * interpolation: color = (1 − α) · color + α · reflected_color, with α the material mirror of object
     * reflected_color is computed by recursively tracing a ray reflected at the intersection point
     */

    if(_depth<max_depth&&object->material.mirror>0.0){
        vec3 reflected = reflect(_ray.direction, normal); //reflect incoming viewing ray at surface normal. is describing the direction of reflected ray!
        Ray reflectedRAY(point + reflected*reflection_ray_offset, reflected); //create reflected ray. add offset.
        double material_mirror = object -> material.mirror; //add how reflective the material is, with material.mirror property
        color = (1-material_mirror)*color + material_mirror*trace(reflectedRAY, _depth+1); //recursion!!
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

vec3 Scene::lighting(const vec3& _point, const vec3& _normal, const vec3& _view, const Material& _material) {
    /**
     * ambient:
     * ambient light color: (material ambient reflection coefficient)*(ambient light intensity for scene)
     */
    vec3 ambientLight = (_material.ambient * ambience);
    vec3 diffuseLight = vec3(0, 0, 0);
    vec3 specularLight = vec3(0, 0, 0);

    /**
     * iterate over all light sources
     */
    for (const Light &light : lights) {
        vec3 l_direction = normalize(light.position - _point);
        /**
         * check: is point in shadow?
         */
        Ray shadow(_point + l_direction * shadow_ray_offset, l_direction);
        Object_ptr object;
        vec3 point;
        vec3 normal;
        double t;

        double l_distance = distance(_point, light.position);
        if (!intersect(shadow, object, point, normal, t) && (t < l_distance)) {
            //check if light comes from right direction.

            if (dot(l_direction, _normal) > 0.0) {
                /**
                * diffusion: I_l*m_d(n\dot l)where
                * I_l is the intensity of light source,
                * m_d is materials diffuse reflection coefficient.
                * light direction l and normalized vector n
                **/
                vec3 lightcolor = light.color;
                diffuseLight += lightcolor * dot(l_direction, _normal) * _material.diffuse;
                /**
                 * specular reflection: I_l*m_s(r\dotv)^s with
                 * I_l is the intensity of light source,
                 * m_s the materials specular reflection coefficient
                 * r reflected ray
                 * v refracted ray
                 * s material shininess
                 */
                vec3 reflected_ray = normalize(mirror(l_direction, _normal));
                if ( dot(_view, reflected_ray) > 0.0) {
                    //siehe Vorlesung, wenn kleiner als 0 wäre, dann würde man von der anderen Seite auf Ebene schauen
                    specularLight = specularLight + (_material.specular * light.color) * pow(dot(reflected_ray, _view), _material.shininess);
                }
            }

        }
    }
    vec3 color = ambientLight + diffuseLight + specularLight;
    return color;

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
