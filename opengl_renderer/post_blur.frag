#version 330 core
// One direction of a separable 9-tap gaussian (5 texture reads thanks to bilinear filtering).
out vec4 FragColor;
in vec2 texCoords;

uniform sampler2D image;
uniform vec2 direction;      // (1/width, 0) for horizontal, (0, 1/height) for vertical

void main()
{
    const float offset[3] = float[](0.0, 1.3846153846, 3.2307692308);
    const float weight[3] = float[](0.2270270270, 0.3162162162, 0.0702702703);

    vec3 result = texture(image, texCoords).rgb * weight[0];
    for (int i = 1; i < 3; i++)
    {
        result += texture(image, texCoords + direction * offset[i]).rgb * weight[i];
        result += texture(image, texCoords - direction * offset[i]).rgb * weight[i];
    }
    FragColor = vec4(result, 1.0);
}
