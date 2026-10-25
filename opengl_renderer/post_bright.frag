#version 330 core
// Bloom bright pass: keeps only HDR energy above `threshold` (soft knee, no hard edge).
out vec4 FragColor;
in vec2 texCoords;

uniform sampler2D hdrTexture;
uniform float threshold;

void main()
{
    vec3 c = texture(hdrTexture, texCoords).rgb;
    float brightness = max(c.r, max(c.g, c.b));
    float knee = 0.5 * threshold + 1e-4;
    float soft = clamp(brightness - threshold + knee, 0.0, 2.0 * knee);
    soft = soft * soft / (4.0 * knee);
    float contribution = max(soft, brightness - threshold) / max(brightness, 1e-4);
    FragColor = vec4(c * contribution, 1.0);
}
