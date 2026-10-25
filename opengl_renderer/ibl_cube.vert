#version 330 core
// Renders a unit cube from the inside, once per cubemap face (Environment.cpp sets view/projection).
layout (location = 0) in vec3 aPos;

out vec3 localPos;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    localPos = aPos;
    gl_Position = projection * view * vec4(aPos, 1.0);
}
