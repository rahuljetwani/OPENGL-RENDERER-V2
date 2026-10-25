#version 330 core
// Big (sRGB, 8-bit) cubemap -> float cubemap. Sampling an sRGB texture returns LINEAR values.
out vec4 FragColor;
in vec3 localPos;

uniform samplerCube sourceMap;

void main()
{
    FragColor = vec4(texture(sourceMap, normalize(localPos)).rgb, 1.0);
}
