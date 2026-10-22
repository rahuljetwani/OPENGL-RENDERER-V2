#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTex;

out vec3 crntPos;       // world-space position
out vec3 normal;        // world-space normal
out vec3 color;
out vec2 texCoord;

uniform mat4 camMatrix;         // projection * view
uniform mat4 model;             // local -> world (already includes the node hierarchy + model root transform)
uniform mat3 normalMatrix;      // transpose(inverse(mat3(model)))

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    crntPos  = worldPos.xyz;
    normal   = normalMatrix * aNormal;      // normals must be transformed too, or lighting is wrong on rotated/scaled models
    color    = aColor;
    texCoord = aTex;                        // no UV hacks any more: Assimp + Texture.cpp agree on the convention
    gl_Position = camMatrix * worldPos;
}
