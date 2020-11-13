//=============================================================================
//
//   Exercise code for "Introduction to Computer Graphics"
//     by Julian Panetta, EPFL
//
//=============================================================================
#version 140

// Eye-space fragment position and normals from vertex shader.
in vec3 v2f_normal;
in vec3 v2f_ec_vertex;

uniform vec3 light_position; // Eye-space light position
uniform vec3 light_color;

// Material parameters
uniform vec3  diffuse_color;
uniform vec3 specular_color;
uniform float shininess;

uniform samplerCube shadow_map; // Distances in the shadow map can be accessed with texture(shadow_map, direction).r

out vec4 f_light_contribution;

void main()
{
    vec3 color = vec3(0.0f);
    // Orient the normal so it always points opposite the camera rays:
    vec3 N = -sign(dot(v2f_normal, v2f_ec_vertex)) *
             normalize(v2f_normal);

    /** \todo done
    * Compute this light's diffuse and specular contributions.
    * You should be able to copy your phong lighting code from assignment 6 mostly as-is,
    * though notice that the light and view vectors need to be computed from scratch
    * here; this time, they are not passed from the vertex shader.

    * The light should only contribute to this fragment if the fragment is not occluded
    * by another object in the scene. You need to check this by comparing the distance
    * from the fragment to the light against the distance recorded for this
    * light ray in the shadow map.
    * To prevent "shadow acne" and minimize aliasing issues, we need a rather large
    * tolerance on the distance comparison. It's recommended to use a *multiplicative*
    * instead of additive tolerance: compare the fragment's distance to 1.01x the
    * distance from the shadow map.
    ***/

    vec3 light = -light_position + v2f_ec_vertex; //fragment to light
    vec3 normal_light = normalize(-light);
    vec3 normal_view = normalize(-v2f_ec_vertex);

    /**why we use texture for distance: (learnopengl.com) What if we were to render the scene from the light's perspective and
    *store the resulting depth values in a texture? This way, we can sample the closest depth values as seen from
    *the light's perspective. After all, the depth values show the first fragment visible from the light's perspective.
    *We store all these depth values in a texture that we call a depth map or shadow map.
    */
    if(length(light) < 1.01f*texture(shadow_map,light).r){
        //when not in shadow, we compute specular und diffuse light.
        //diffuse
        if (dot(N, normal_light) > 0) {
            color += light_color * diffuse_color * dot(N, normal_light);
        }
        //specular
        float r = dot(normal_view, reflect(normal_light, N));
        if (dot(normal_view, normal_light) > 0 && r > 0) {
            color += light_color * specular_color * pow(dot(reflect(normal_light, N), normal_view), shininess);
        }
    }
    // append the required alpha value
    f_light_contribution = vec4(color, 1.0);
}
