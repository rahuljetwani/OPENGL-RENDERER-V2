#version 330 core
// Equirectangular .hdr panorama -> cubemap face
out vec4 FragColor;
in vec3 localPos;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591549, 0.3183099);   // 1/(2*pi), 1/pi

void main()
{
    vec3 v = normalize(localPos);
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y)) * invAtan + 0.5;
    FragColor = vec4(texture(equirectangularMap, uv).rgb, 1.0);
}
