#version 330 core

out vec4 FragColor;

in vec3 texCoords;

uniform samplerCube skybox;

void main()
{
    FragColor = texture(skybox, texCoords);     // the cubemap is SRGB8, so this is already linear light
}
