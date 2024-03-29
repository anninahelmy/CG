#version 140
#pragma optionNV unroll all

//=============================================================================
//	Exercise code for "Introduction to Computer Graphics 2018"
//     by
//	Krzysztof Lis @ EPFL
//=============================================================================

uniform float t;
float hash_poly(float x) {
	return mod(((x*34.0)+1.0)*x, 289.0);
}

// -- Hash function --
// For 1d case, use vec2(c, 0)
int hash_func(vec2 grid_point) {
	return int(hash_poly(hash_poly(grid_point.x) + grid_point.y));
}

// -- Smooth interpolation polynomial --
// Use mix(a, b, blending_weight_poly(t))
float blending_weight_poly(float t) {
	return t*t*t*(t*(t*6.0 - 15.0)+10.0);
}

// -- Gradient table --
// use gradients[hash % NUM_GRADIENTS] to access the gradient corresponding
// to a hashed grid point location
#define NUM_GRADIENTS 12
const vec2 gradients[NUM_GRADIENTS] = vec2[NUM_GRADIENTS](
	vec2( 1,  1),
	vec2(-1,  1),
	vec2( 1, -1),
	vec2(-1, -1),
	vec2( 1,  0),
	vec2(-1,  0),
	vec2( 1,  0),
	vec2(-1,  0),
	vec2( 0,  1),
	vec2( 0, -1),
	vec2( 0,  1),
	vec2( 0, -1)
);

// Constants for FBM
const float freq_multiplier = 2.17;
const float ampl_multiplier = 0.5;
const int num_octaves = 4;

// ==============================================================
// 1D Perlin noise evaluation and plotting

float perlin_noise_1d(float x) {
	/*
	// Note Gradients in the table are 2d, so in the 1D case we use grad.x
	*/
	/**
	 * Evaluate the 1D Perlin noise function at "x" as described
	 * in the handout. You will determine the two grid points
	 * surrounding x, look up their gradients, evaluate the the
	 * linear functions these gradients describe, and interpolate these
	 * values using the smooth interolation polygnomial blending_weight_poly.
	 * Note: gradients in the gradient lookup table are 2D,
	 */

	//step 1
	float cell_corner_left = floor(x);
	float cell_corner_right = cell_corner_left+1.0;

	//step 2
	int hash_left = hash_func(vec2(cell_corner_left, 0));
	int hash_right = hash_func(vec2(cell_corner_right,0));
	//gi = gradients[h(ci) mod 12].
	//// use gradients[hash % NUM_GRADIENTS] to access the gradient corresponding
	//// to a hashed grid point location
	vec2 gradient_left = gradients[hash_left % NUM_GRADIENTS];
	vec2 gradient_right = gradients[hash_right % NUM_GRADIENTS];

	//step 3: calculate contributions: φi(p)=gi ·(p−ci)
	float contribution_left = gradient_left.x*(x-cell_corner_left);
	float contribution_right = gradient_right.x*(x-cell_corner_right);


	return mix(contribution_left, contribution_right, blending_weight_poly(x-cell_corner_left));
}

float perlin_fbm_1d(float x) {
	float amplitude = 1.;
	float frequency = 1.;
	//with the help of [[https://thebookofshaders.com/13/]]
	float y = 0;
	for (int i=0; i < num_octaves; i++){
		y += amplitude*perlin_noise_1d(frequency*x);
		amplitude*= ampl_multiplier; //increment
		frequency *= freq_multiplier; //increment
	}
	return y;
}

// ----- plotting -----

const vec3 plot_foreground = vec3(0.5, 0.8, 0.5);
const vec3 plot_background = vec3(0.2, 0.2, 0.2);

vec3 plot_value(float func_value, float coord_within_plot) {
	return (func_value < ((coord_within_plot - 0.5)*2.0)) ? plot_foreground : plot_background;
}

vec3 plots(vec2 point) {
	// Press D (or right arrow) to scroll

	if(point.y < 0 || point.y > 1.0) {
		return vec3(255, 0, 0);
	}

	float y_inv = 1-point.y;
	float y_rel = y_inv / 0.2;
	int which_plot = int(floor(y_rel));
	float coord_within_plot = fract(y_rel);

	vec3 result;
	if(which_plot < 4) {
		result = plot_value(
 			perlin_noise_1d(point.x * pow(freq_multiplier, which_plot)),
			coord_within_plot
		);
	} else {
		result = plot_value(
			perlin_fbm_1d(point.x) * 1.5,
			coord_within_plot
		);
	}

	return result;
}

// ==============================================================
// 2D Perlin noise evaluation

float perlin_noise(vec2 point) {
	/**
	* Implement 2D perlin noise as described in the handout.
	* You may find a glsl `for` loop useful here, but it's not necessary.
	**/

	//step 1
	vec2 corner_00 = floor(point);
	vec2 corner_10 = corner_00+vec2(1,0);
	vec2 corner_01 = corner_00+vec2(0,1);
	vec2 corner_11 = corner_00+vec2(1,1);

	//step 2
	//gi = gradients[h(ci) mod 12].
	// use gradients[hash % NUM_GRADIENTS] to access the gradient corresponding
	// to a hashed grid point location
	vec2 gradient_00 = gradients[hash_func(corner_00) % NUM_GRADIENTS];
	vec2 gradient_10 = gradients[hash_func(corner_10) % NUM_GRADIENTS];
	vec2 gradient_01 = gradients[hash_func(corner_01) % NUM_GRADIENTS];
	vec2 gradient_11 = gradients[hash_func(corner_11) % NUM_GRADIENTS];

	//step 3: calculate contributions: φi(p)=gi ·(p−ci)
	float contribution_00 = dot(point-corner_00,gradient_00);
	float contribution_10 = dot(point-corner_10,gradient_10);
	float contribution_01 = dot(point-corner_01,gradient_01);
	float contribution_11 = dot(point-corner_11,gradient_11);
	//need realtive point see assignment(x and y in the slide’s formulas mean p’s relative position inside the cell)

	vec2 point_relative = fract(point); //fract(x) returns the fractional part of x. This is calculated as x - floor(x).

	float st = mix(contribution_00, contribution_10, blending_weight_poly(point_relative.x));
	float uv = mix(contribution_01, contribution_11, blending_weight_poly(point_relative.x));

	return mix(st, uv,blending_weight_poly(point_relative.y));
}

// ==============================================================
// 2D Fractional Brownian Motion

float perlin_fbm(vec2 point) {
	float amplitude = 1.;
	float frequency = 1.;
	//with the help of [[https://thebookofshaders.com/13/]]
	//exactly the same as in 1D…
	float y = 0;
	for (int i=0; i < num_octaves; i++){
		y += amplitude*perlin_noise(frequency*point);
		amplitude*= ampl_multiplier; //increment
		frequency *= freq_multiplier; //increment
	}
	return y;
}

// ==============================================================
// 2D turbulence

float turbulence(vec2 point) {
	float amplitude = 1.;
	float frequency = 1.;
	//with the help of [[https://thebookofshaders.com/13/]]
	//exactly the same as in 1D…
	float y = 0;
	for (int i=0; i < num_octaves; i++){
		y += amplitude*abs(perlin_noise(frequency*point)); //just add absolute value for turbulence
		amplitude*= ampl_multiplier; //increment
		frequency *= freq_multiplier; //increment
	}
	return y;
}

// ==============================================================
// Procedural "map" texture

const float terrain_water_level = -0.075;
const vec3 terrain_color_water = vec3(0.29, 0.51, 0.62);
const vec3 terrain_color_grass = vec3(0.43, 0.53, 0.23);
const vec3 terrain_color_mountain = vec3(0.8, 0.7, 0.7);

vec3 tex_map(vec2 point) {
	/**
	 * Implement your map texture evaluation routine as described in the
	 * handout. You will need to use your perlin_fbm routine and the
	 * terrain color constants described above.
	 */
	float s = perlin_fbm(point);
	if(s < terrain_water_level){
		return terrain_color_water;
	}
	else{
		return mix(terrain_color_grass, terrain_color_mountain, s-terrain_water_level); //mix(x, y, α) interpolate between x and y, with weight alpha
	}

}

// ==============================================================
// Procedural "wood" texture

const vec3 brown_dark 	= vec3(0.48, 0.29, 0.00);
const vec3 brown_light 	= vec3(0.90, 0.82, 0.62);

vec3 tex_wood(vec2 point) {

	//α = 1/2* (1 + sin(100 (∥p∥ + 0.15 turbulence(p)))).
	float sine_in = 100*(length(point)+0.15*turbulence(point));
	float alpha = 0.5*(1.0+sin(sine_in));
	return mix(brown_dark, brown_light, alpha);
}


// ==============================================================
// Procedural "marble" texture

const vec3 white 			= vec3(0.95, 0.95, 0.95);

vec3 tex_marble(vec2 point) {
/* α =1/2(1 + fbm(p + 4q)), where q =(fbm(p), fbm(p + (1.7, 4.6)).
*/
	vec2 q = vec2(perlin_fbm(point), perlin_fbm(point+(1.7, 4.6)));
	float alpha = 0.5*(1.0+perlin_fbm(point+4*q));
	return mix(white, brown_dark, alpha);
}

