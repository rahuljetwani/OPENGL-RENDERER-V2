#version 330 core
// Diffuse IBL: convolves the environment over the cosine-weighted hemisphere around each direction.
out vec4 FragColor;
in vec3 localPos;

uniform samplerCube environmentMap;
const float sourceLod = 5.0;      // read a blurry mip of the big skybox: fewer samples needed, no sparkle

const float PI = 3.14159265359;

void main()
{
    vec3 N = normalize(localPos);
    vec3 up    = abs(N.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up         = cross(N, right);

    vec3 irradiance = vec3(0.0);
    float sampleDelta = 0.05;
    float nrSamples = 0.0;
    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;
            irradiance += textureLod(environmentMap, sampleVec, sourceLod).rgb * cos(theta) * sin(theta);
            nrSamples += 1.0;
        }
    }
    FragColor = vec4(PI * irradiance / nrSamples, 1.0);
}
