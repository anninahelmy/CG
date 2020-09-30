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

#include "Plane.h"
#include <limits>


//== CLASS DEFINITION =========================================================



Plane::Plane(const vec3& _center, const vec3& _normal)
: center(_center), normal(_normal)
{
}


//-----------------------------------------------------------------------------


bool
Plane::
intersect(const Ray& _ray,
          vec3&      _intersection_point,
          vec3&      _intersection_normal,
          double&    _intersection_t ) const
{

    const vec3 &dir = _ray.direction;
    const vec3 oc = _ray.origin;

    _intersection_t = NO_INTERSECTION;


    if (dot(&dir, normal) == 0) return false;
    else {
        _intersection_t = (center - dot(normal, oc))/(dot(normal, dir));
        if (_intersection_t<0){
            _intersection_t = NO_INTERSECTION;
        }
    }

    if(_intersection_t == NO_INTERSECTION) return false;

    _intersection_point = _ray(_intersection_t);
    _intersection_normal = normal;

    return true;
}


//=============================================================================
