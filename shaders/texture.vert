#version 330

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec2 vertex_texcoord;

uniform mat4 projection_mat, view_mat, model_mat;
uniform mat3 normal_mat;
out vec3 position_world, normal_world;
out vec2 texture_coordinates;


void main () {
	position_world = vec3 (model_mat * vec4 (vertex_position, 1.0));
	normal_world = normalize(normal_mat * vertex_normal);
	texture_coordinates = vertex_texcoord;
	gl_Position = projection_mat * view_mat * vec4 (position_world, 1.0);
}
