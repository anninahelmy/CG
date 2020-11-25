//=============================================================================
//
//   Exercise code for the lecture "Introduction to Computer Graphics"
//     by Prof. Mario Botsch, Bielefeld University
//
//   Copyright (C) by Computer Graphics Group, Bielefeld University
//
//=============================================================================

#version 140

uniform vec3 light_position; // Eye-space light position

in vec3  v2f_ec_vertex;
in vec3  v2f_normal;
in float v2f_height;

out vec4 f_color;

const vec3  sunlight = vec3(1.0, 0.941, 0.898);
// Small perturbation to prevent "z-fighting" on the water on some machines...
const float terrain_water_level    = -0.03125 + 1e-6;
const vec3  terrain_color_water    = vec3(0.29, 0.51, 0.62);
const vec3  terrain_color_mountain = vec3(0.8, 0.5, 0.4);
const vec3  terrain_color_grass    = vec3(0.33, 0.43, 0.18);

void main()
{
	const vec3 ambient = 0.2 * sunlight; // Ambient light intensity
	float height = v2f_height;

	vec3 material;
	float shininess;
	if(height>terrain_water_level){
		material = mix(terrain_color_grass, terrain_color_mountain, (height-terrain_water_level)*2);
		shininess = 0.5;
	}
	else{
		material = terrain_color_water;
		shininess =8.0;
	}

    /**
	 * \todo Paste your Phong fragment shading code from assignment 6/7 here,
	 * altering it to use the terrain color as the ambient, diffuse, and
	 * specular materials.
     */
	vec3 color = vec3(0.0f);
	// Orient the normal so it always points opposite the camera rays:
	vec3 N = -sign(dot(v2f_normal, v2f_ec_vertex)) *
	normalize(v2f_normal);

	vec3 light = -light_position + v2f_ec_vertex; //fragment to light
	vec3 normal_light = normalize(-light);
	vec3 normal_view = normalize(-v2f_ec_vertex);

	color += ambient * material;

		if (dot(N, normal_light) > 0) {
			color += material * sunlight * dot(N, normal_light);
		}
		//specular
		float r = dot(normal_view, reflect(normal_light, N));
		if (dot(normal_view, normal_light) > 0 && r > 0) {
			color += sunlight * material * pow(dot(reflect(normal_light, N), normal_view), shininess);
		}

	// append the required alpha value
	f_color = vec4(color, 1.0);
}
