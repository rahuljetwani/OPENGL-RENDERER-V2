#version 330 core

// HDR image -> exposure -> ACES tone mapping -> sRGB -> window

out vec4 FragColor;
in vec2 texCoords;

uniform sampler2D screenTexture;
uniform float exposure;

vec3 aces(vec3 x)
{
    const float a = 2.51, b = 0.03, c = 2.43, d = 0.59, e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main()
{
    vec3 hdr = texture(screenTexture, texCoords).rgb * exposure;
    vec3 ldr = aces(hdr);
    FragColor = vec4(pow(ldr, vec3(1.0 / 2.2)), 1.0);
}
