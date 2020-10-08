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

#include "Cylinder.h"
#include "SolveQuadratic.h"

#include <array>
#include <cmath>

//== IMPLEMENTATION =========================================================

//intersect line equation with cylinder equation, find intersection with infinite cylinder with center in the origin
bool
Cylinder::
intersect(const Ray&  _ray,
          vec3&       _intersection_point,
          vec3&       _intersection_normal,
          double&     _intersection_t) const
{
    const vec3 &dir = _ray.direction;
    const vec3  oc = _ray.origin - center; //translate origin
//the rays intersect the cylinder that are at equal distance equal to the radius from cylinder axis.
    //range check: idea: let two points x1, x2 be on axis.
    //Let 0<= ("solution of intersection"-x1)(x2-x1)<=(x2-x1)^2, then p is a solution of bounded cylinder (bounded by x1,x2)

    const double dir_axis = dot(axis, dir), oc_axis = dot(axis, oc);

    //coefficients for intersection equation (infinite cylinder along x-axis:
    //double a = dot(dir,dir);
    //double b = 2*(dot(dir,oc));
    //double c = dot(oc,oc)-radius*radius;
    double a = dot(dir,dir)-dir_axis*dir_axis;
    double b = 2*(dot(dir,oc) - dir_axis*oc_axis);
    double c = dot(oc,oc)- oc_axis*oc_axis-radius*radius;
    //infinite extension of cylinder sol
    std::array<double, 2> t;
    size_t nsol = solveQuadratic(a,b,c,t);

    _intersection_t = NO_INTERSECTION;

    // Find the closest valid solution (in front of the viewer)
    for (size_t i = 0; i < nsol; ++i) {
        if(t[i] <=0) continue;
        double q = dot(_ray(t[i])-center, axis);
        if (2*std::abs(q)<height) //if absolute value is smaller than the height -- there must be an intersection
            _intersection_t = std::min(_intersection_t, t[i]);
    }

    if (_intersection_t == NO_INTERSECTION) return false;

    //intersection
    _intersection_point = _ray(_intersection_t);
    _intersection_normal = (_intersection_point-center)/radius;
    _intersection_normal -= (dot(_intersection_normal,axis))*axis;


    return true;
}


