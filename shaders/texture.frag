#version 330

in vec3 position_world, normal_world;
uniform vec3 eye_world;

vec3 lightPos = eye_world + vec3(-20, 20, 20);

uniform int hasTexture;
uniform vec4 materialColor;
uniform sampler2D textureSampler;
in vec2 texture_coordinates;

out vec4 fragment_colour;

void main () {
	vec4 color = materialColor * (1 - hasTexture) + hasTexture * texture(textureSampler, texture_coordinates );
    // Ambient
    vec3 ambient = 0.2 * color.rgb;
    // Diffuse
    vec3 lightDir = normalize(lightPos - position_world);
    vec3 normal = normalize(normal_world);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color.rgb;
    // Specular
    vec3 viewDir = normalize(eye_world - position_world);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;

    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.8) * spec;
    fragment_colour = vec4(ambient + diffuse + specular, color.a);
	
}