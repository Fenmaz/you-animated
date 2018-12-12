#version 330 

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec2 vertex_texcoord;
layout (location = 3) in ivec4 boneIDs;
layout (location = 4) in vec4 weights;

uniform mat4 projection_mat, view_mat, model_mat;
uniform mat3 normal_mat;

out vec3 position_world, normal_world;
out vec2 texture_coordinates;

const int MAX_BONES = 100;
uniform mat4 bones[MAX_BONES];

void main()
{ 
    mat4 boneTransform = bones[boneIDs[0]] * weights[0];
    boneTransform += bones[boneIDs[1]] * weights[1];
    boneTransform += bones[boneIDs[2]] * weights[2];
    boneTransform += bones[boneIDs[3]] * weights[3];

    position_world = vec3 (model_mat * boneTransform * vec4 (vertex_position, 1.0));
    normal_world = normalize(normal_mat * vec3(boneTransform * vec4(vertex_normal, 0.0)));
    texture_coordinates = vertex_texcoord;
    
    gl_Position = projection_mat * view_mat * vec4 (position_world, 1.0);
}
