#ifdef _WIN32
#  include <windows.h>
#  include <stdlib.h>
#  include <errhandlingapi.h>
#endif
#include "render/ShaderViewer.h"
#include <vector>
#include <string>
#include <cmath>

#include "render/Mesh.h"
#include "render/MeshViewer.h"

// Render your GLSL fBm into a 2D array of height values.
// \param size	resolution at which to sample the fBm.
Array2D<float> calculate_fbm(std::pair<size_t, size_t> const& size) {
	ShaderViewer shvi("fBm", size.first, size.second);
	std::vector<std::string>  vtxShaders = { SHADER_PATH "/display.vert" };
	std::vector<std::string> fragShaders = { SHADER_PATH "/noise.frag",
											 SHADER_PATH "/display_fbm_terrain.frag" };
	shvi.setShaderFiles(vtxShaders, fragShaders);

	Array2D<vec4> noise_value_rgbd({0, 0});
	shvi.run([&]() {
		shvi.resize(size.first, size.second); // Force the texture size--GLFW might have given us the wrong window size.
		noise_value_rgbd = shvi.extractFrame();
		return false;
	});

	return noise_value_rgbd.map<float>([](size_t x, size_t y, vec4 const& val_rgbd) {
		return val_rgbd[0];
	});
}

constexpr float WATER_LEVEL = -0.03125;

// Procedurally generate a triangle mesh of the terrain from the 2D array of height values.
// This triangle mesh is a uniform grid in the (x, y) plane that is displaced in the z
// direction by the height values.
std::shared_ptr<Mesh> build_terrain_mesh(Array2D<float> const& height_map) {
	std::pair<size_t, size_t> const grid_size = height_map.size();

	std::vector<vec3> vertices(grid_size.first * grid_size.second);
	std::vector<Mesh::Face> faces;

	// Map a 2D grid index (x, y) into a 1D index into the output vertex array.
	auto const xy_to_v_index = [&](int x, int y) {
		return x + y*grid_size.first;
	};

	// First, generate the displaced vertices of the grid.
	// (iterate over y first then over x to use CPU cache better)
	for(int gy = 0; gy < grid_size.second; gy++) {
		for(int gx = 0; gx < grid_size.first; gx++) {
			int const idx = xy_to_v_index(gx, gy);

			 float z = height_map(gx,gy);
			 if(z < WATER_LEVEL){
			     z = WATER_LEVEL; //however if the point falls below WATER_LEVEL, it should be clamped back to WATER_LEVEL.
			 }

            vec2 world = vec2(float(gx)/grid_size.first - 0.5 , float(gy)/grid_size.second - 0.5); //generate whole world coord.
			vertices.at(idx) = vec3(world.x, world.y, z);
		}
	}
	// Second, connect the grid vertices with triangles to triangulate the terrain.
	for(int gy = 0; gy < grid_size.second-1; gy++) {
		for(int gx = 0; gx < grid_size.first-1; gx++) {

            int bottom_left = xy_to_v_index(gx,gy);
            int top_left = xy_to_v_index(gx,gy+1);
            int bottom_right = xy_to_v_index(gx+1,gy);
            int top_right = xy_to_v_index(gx+1,gy+1);

            faces.emplace_back(bottom_left, bottom_right, top_left);
            faces.emplace_back(bottom_right, top_right, top_left);

        }
	}

	return std::make_shared<Mesh>(vertices, faces);
}


int main(int arg_count, char *arg_values[])
{
#ifdef _WIN32
    // This make crashes very visible - without them, starting the
    // application from cmd.exe or powershell can surprisingly hide
    // any signs of a an application crash!
    SetErrorMode(0);
#endif
	std::pair<size_t, size_t> grid_size(96, 96);

	// If we try to build meshes when no window is created (GLFW is not loaded)
	// calls to OpenGL will crash randomly
	Array2D<float> fbm_values = calculate_fbm(grid_size);
	fbm_values *= 0.5;

	MeshViewer mvi;
	auto mesh = build_terrain_mesh(fbm_values);

	mesh->print_info();
	mvi.setMesh(mesh);
	return mvi.run();
}
