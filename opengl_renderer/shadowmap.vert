#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;      // world -> light clip space
uniform mat4 model;                 // local -> world, same matrix the colour pass uses

void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}
