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

#include "Mesh.h"
#include <fstream>
#include <string>
#include <stdexcept>
#include <limits>
#include <cmath>


//== IMPLEMENTATION ===========================================================


Mesh::Mesh(std::istream &is, const std::string &scenePath)
{
    std::string meshFile, mode;
    is >> meshFile;

    const char pathSep =
#ifdef _WIN32
            '\\';
#else
            '/';
#endif

    // load mesh from file
    read(scenePath.substr(0, scenePath.find_last_of(pathSep) + 1) + meshFile);

    is >> mode;
    if      (mode ==  "FLAT") draw_mode_ = FLAT;
    else if (mode == "PHONG") draw_mode_ = PHONG;
    else throw std::runtime_error("Invalid draw mode " + mode);

    is >> material;
}


//-----------------------------------------------------------------------------


bool Mesh::read(const std::string &_filename)
{
    // read a mesh in OFF format


    // open file
    std::ifstream ifs(_filename);
    if (!ifs)
    {
        std::cerr << "Can't open " << _filename << "\n";
        return false;
    }


    // read OFF header
    std::string s;
    unsigned int nV, nF, dummy, i;
    ifs >> s;
    if (s != "OFF")
    {
        std::cerr << "No OFF file\n";
        return false;
    }
    ifs >> nV >> nF >> dummy;
    std::cout << "\n  read " << _filename << ": " << nV << " vertices, " << nF << " triangles";


    // read vertices
    Vertex v;
    vertices_.clear();
    vertices_.reserve(nV);
    for (i=0; i<nV; ++i)
    {
        ifs >> v.position;
        vertices_.push_back(v);
    }


    // read triangles
    Triangle t;
    triangles_.clear();
    triangles_.reserve(nF);
    for (i=0; i<nF; ++i)
    {
        ifs >> dummy >> t.i0 >> t.i1 >> t.i2;
        triangles_.push_back(t);
    }


    // close file
    ifs.close();


    // compute face and vertex normals
    compute_normals();

    // compute bounding box
    compute_bounding_box();


    return true;
}


//-----------------------------------------------------------------------------

// Determine the weights by which to scale triangle (p0, p1, p2)'s normal when
// accumulating the vertex normals for vertices 0, 1, and 2.
// (Recall, vertex normals are a weighted average of their incident triangles'
// normals, and in our raytracer we'll use the incident angles as weights.)
// \param[in] p0, p1, p2    triangle vertex positions
// \param[out] w0, w1, w2    weights to be used for vertices 0, 1, and 2
void angleWeights(const vec3 &p0, const vec3 &p1, const vec3 &p2,
                  double &w0, double &w1, double &w2) {
    // compute angle weights
    const vec3 e01 = normalize(p1-p0);
    const vec3 e12 = normalize(p2-p1);
    const vec3 e20 = normalize(p0-p2);
    w0 = acos( std::max(-1.0, std::min(1.0, dot(e01, -e20) )));
    w1 = acos( std::max(-1.0, std::min(1.0, dot(e12, -e01) )));
    w2 = acos( std::max(-1.0, std::min(1.0, dot(e20, -e12) )));
}


//-----------------------------------------------------------------------------

void Mesh::compute_normals()
{
    // compute triangle normals
    for (Triangle& t: triangles_)
    {
        const vec3& p0 = vertices_[t.i0].position;
        const vec3& p1 = vertices_[t.i1].position;
        const vec3& p2 = vertices_[t.i2].position;
        t.normal = normalize(cross(p1-p0, p2-p0));
    }

    // initialize vertex normals to zero
    for (Vertex& v: vertices_)
    {
        v.normal = vec3(0,0,0);
    }

    /** \todo: review code
     * In some scenes (e.g the office scene) some objects should be flat
     * shaded (e.g. the desk) while other objects should be Phong shaded to appear
     * realistic (e.g. chairs). You have to implement the following:
     * - Compute vertex normals by averaging the normals of their incident triangles.
     * - Store the vertex normals in the Vertex::normal member variable.
     * - Weigh the normals by their triangles' angles.
     */

    // we need to find all incident triangles for every vertex ->very inefficient brute force algorithm
    // in order of efficiency we iterate over all triangle
    for (Triangle& t: triangles_){
        //find angles/weight for every vertex in triangle
        double w0, w1, w2;
        angleWeights(vertices_[t.i0].position, vertices_[t.i1].position, vertices_[t.i2].position, w0, w1, w2);
        // add for every vertex of the triangle the weighted normal of the triangle to the vertex normal
        vertices_[t.i0].normal += t.normal*w0;
        vertices_[t.i1].normal += t.normal*w1;
        vertices_[t.i2].normal += t.normal*w2;
    }

    // iterate over all vertex normals and normalize them
    for(Vertex& v: vertices_){
        v.normal = normalize(v.normal);
    }
}


//-----------------------------------------------------------------------------


void Mesh::compute_bounding_box()
{
    bb_min_ = vec3(std::numeric_limits<double>::max());
    bb_max_ = vec3(std::numeric_limits<double>::lowest());

    for (Vertex v: vertices_)
    {
        bb_min_ = min(bb_min_, v.position);
        bb_max_ = max(bb_max_, v.position);
    }
}


//-----------------------------------------------------------------------------


bool Mesh::intersect_bounding_box(const Ray& _ray) const
{

    /** \todo
    * Intersect the ray `_ray` with the axis-aligned bounding box of the mesh.
    * Note that the minimum and maximum point of the bounding box are stored
    * in the member variables `bb_min_` and `bb_max_`. Return whether the ray
    * intersects the bounding box.
    * This function is used in `Mesh::intersect()` to avoid the intersection test
    * with all triangles of every mesh in the scene. The bounding boxes are computed
    * in `Mesh::compute_bounding_box()`.
    */

    // since we are working with axis aligned bounding boxes we know the direction of our bounding boxes.
    // our box has 6 planes
    float t;
    vec3 n1,n2,n3, intersection;
    n1 = vec3(1,0,0);
    n2 = vec3(0,1,0);
    n3 = vec3(0,0,1);
    // discard obvious rays that don't intersect
    if(dot(_ray.direction, n1)==0 || dot(_ray.direction, n2)==0 || dot(_ray.direction, n3)==0){
        return false;
    }
    // check if ray intersect plane: t = (dot(n1, bb_min_-_ray.origin))/(dot(n1, _ray.direction));
    // check which planes of the box need to be checked for intersection
    // if there is an intersection and t is between min and max then the ray intersect with the bounding box.
    /*TODO richtig grusig coded, es git sicher en eleganteri lösig...
     * @Ronja: wahrschinlich chenntme e for-schleife mache...
     * aber s geit ja eich aues bis uf die komische schwarze flecke
     * */
    if (_ray.origin[0]<bb_min_[0]){
        t = (dot(n1, bb_min_-_ray.origin))/(dot(n1, _ray.direction));
        intersection = _ray(t);
        if(intersection[1]>bb_min_[1] && intersection[1]<bb_max_[1] &&
           intersection[2]>bb_min_[2] && intersection[2]<bb_max_[2] && t>0) return true;
    } else {
        t = (dot(n1, bb_max_-_ray.origin))/(dot(n1, _ray.direction));
        intersection = _ray(t);
        if(intersection[1]>bb_min_[1] && intersection[1]<bb_max_[1] &&
           intersection[2]>bb_min_[2] && intersection[2]<bb_max_[2] && t>0) return true;
    }

    if (_ray.origin[1]<bb_min_[1]){
        t = (dot(n2, bb_min_-_ray.origin))/(dot(n2, _ray.direction));
        intersection = _ray(t);
        if(intersection[0]>bb_min_[0] && intersection[0]<bb_max_[0] &&
           intersection[2]>bb_min_[2] && intersection[2]<bb_max_[2] && t>0) return true;
    } else {
        t = (dot(n2, bb_max_-_ray.origin))/(dot(n2, _ray.direction));
        intersection = _ray(t);
        if(intersection[0]>bb_min_[0] && intersection[0]<bb_max_[0] &&
           intersection[2]>bb_min_[2] && intersection[2]<bb_max_[2] && t>0) return true;
    }

    if (_ray.origin[2]<bb_min_[2]){
        t = (dot(n3, bb_min_-_ray.origin))/(dot(n3, _ray.direction));
        intersection = _ray(t);
        if(intersection[1]>bb_min_[1] && intersection[1]<bb_max_[1] &&
           intersection[0]>bb_min_[0] && intersection[0]<bb_max_[0] && t>0) return true;
    } else {
        t = (dot(n3, bb_max_-_ray.origin))/(dot(n3, _ray.direction));
        intersection = _ray(t);
        if(intersection[1]>bb_min_[1] && intersection[1]<bb_max_[1] &&
           intersection[0]>bb_min_[0] && intersection[0]<bb_max_[0] && t>0) return true;
    }
    return false;

}


//-----------------------------------------------------------------------------


bool Mesh::intersect(const Ray& _ray,
                     vec3&      _intersection_point,
                     vec3&      _intersection_normal,
                     double&    _intersection_t ) const
{
    // check bounding box intersection
    if (!intersect_bounding_box(_ray))
    {
        return false;
    }

    vec3   p, n;
    double t;

    _intersection_t = NO_INTERSECTION;

    // for each triangle
    for (const Triangle& triangle : triangles_)
    {
        // does ray intersect triangle?
        if (intersect_triangle(triangle, _ray, p, n, t))
        {
            // is intersection closer than previous intersections?
            if (t < _intersection_t)
            {
                // store data of this intersection
                _intersection_t      = t;
                _intersection_point  = p;
                _intersection_normal = n;
            }
        }
    }

    return (_intersection_t != NO_INTERSECTION);
}


//-----------------------------------------------------------------------------
static double determinant(vec3 _x, vec3 _y, vec3 _z){
    return _x[0] * _y[1] * _z[2] + _x[1] * _y[2] * _z[0] + _x[2] * _y[0] * _z[1]
           - (_x[2] * _y[1] * _z[0] + _x[0] * _y[2] * _z[1] + _x[1] * _y[0] * _z[2]) ;
}

//todo: review code
bool
Mesh::
intersect_triangle(const Triangle&  _triangle,
                   const Ray&       _ray,
                   vec3&            _intersection_point,
                   vec3&            _intersection_normal,
                   double&          _intersection_t) const
{
    const vec3& p0 = vertices_[_triangle.i0].position;
    const vec3& p1 = vertices_[_triangle.i1].position;
    const vec3& p2 = vertices_[_triangle.i2].position;


    _intersection_t = NO_INTERSECTION;

    // if ray x normal != 0 -> intersection of plane and ray exists
    if(dot(_triangle.normal, _ray.direction)==0){
        return false;
    }

    /**
    * solve x = aA +bB + cC and a+b+c=1 -> if there exists a solution intersection
     * ray.origin + t*ray.dir = a*p0 + b*p1 + (1-a-b)*p2
     * ray_origin = a*p0 + b*p1 + (1-a-b)*p2 - t*ray.dir
     * ray_origin = a*p0 + b*p1 + p2 - a*p2 - b*p2 - t*ray.dir
     * ray_origin = a*(p0-p2) + b*(p1-p2) + p2 - t*ray.dir
     * ray_origin -p2 = a*(p0-p2) + b*(p1-p2) - t*ray.dir
     * set c = 1-a-b
     * Cramers Rule and solve the equations.
     */
    double a,b,c,t;

    vec3 ray_dir = _ray.direction;
    vec3 ray_origin = _ray.origin;
    double det, det_A1, det_A2, det_A3;
    det = determinant(p0-p2,p1-p2,-ray_dir);
    det_A1 = determinant(ray_origin-p2, p1-p2, -ray_dir);
    det_A2 = determinant(p0-p2, ray_origin-p2, -ray_dir);
    det_A3 = determinant(p0-p2, p1-p2, ray_origin-p2);

    if (det == 0){
        return false;
    }
    a = det_A1/det;
    b = det_A2/det;
    t = det_A3/det;
    c = 1-a-b;
    // check if intersection is in front of viewer: t>0 return true
    if(t<0||a<0||b<0||c<0){
        return false;
    }
    _intersection_t = t;
    _intersection_point = _ray(_intersection_t);

    // compute normal depending on draw_mode_
    if(draw_mode_ == PHONG){
        double w0,w1,w2;
        angleWeights(p0,p1,p2,w0,w1,w2);
        _intersection_normal = normalize(a*vertices_[_triangle.i0].normal + b*vertices_[_triangle.i1].normal + c*vertices_[_triangle.i2].normal);
    }
    else if (draw_mode_ == FLAT){
        _intersection_normal = _triangle.normal;
    }
    return true;
}

//=============================================================================
