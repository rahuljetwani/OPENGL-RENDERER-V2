#version 330 core

out vec4 FragColor;

in vec3 crntPos;
in vec3 normal;
in vec3 color;
in vec2 texCoord;

// material
uniform sampler2D diffuse0;
uniform sampler2D specular0;
uniform vec4  baseColor;
uniform int   hasDiffuseTex;
uniform int   hasSpecularTex;
uniform float shininess;
uniform float specularStrength;

// light (one directional light)
uniform vec4 lightColor;
uniform vec3 lightDirection;        // direction the light rays TRAVEL (pointing away from the light)
uniform vec3 camPos;

// shadows
uniform sampler2DShadow shadowMap;  // hardware depth compare (see ShadowMap.cpp)
uniform mat4  lightSpaceMatrix;
uniform float shadowTexelWorld;     // size of one shadow texel in world units
uniform float shadowBias;           // ~0.0005 in [0,1] depth units

// Returns 0 = fully lit, 1 = fully shadowed
float shadowAmount(vec3 worldPos, vec3 N, vec3 L)
{
    float cosTheta = clamp(dot(N, L), 0.0, 1.0);
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    // Normal-offset: push the lookup point along the surface normal, more at grazing angles.
    // Removes acne without the "peter-panning" (floating shadows) a big depth bias causes.
    vec3 offsetPos = worldPos + N * (shadowTexelWorld * 1.5 * sinTheta);

    vec4 lp = lightSpaceMatrix * vec4(offsetPos, 1.0);
    vec3 p  = lp.xyz / lp.w * 0.5 + 0.5;

    // Outside the shadow map sideways = lit
    if (p.x < 0.0 || p.x > 1.0 || p.y < 0.0 || p.y > 1.0)
        return 0.0;

    // Receivers behind the far plane (e.g. a floor far from the light) must still be shadowed by casters,
    // so clamp the depth instead of skipping the lookup.
    p.z = min(p.z, 1.0);

    float bias = shadowBias * (1.0 - cosTheta) + shadowBias * 0.25;

    // 3x3 PCF; every tap is itself a bilinear comparison, so this is effectively a smooth 4x4 kernel
    vec2 texel = 1.0 / vec2(textureSize(shadowMap, 0));
    float lit = 0.0;
    for (int y = -1; y <= 1; y++)
        for (int x = -1; x <= 1; x++)
            lit += texture(shadowMap, vec3(p.xy + vec2(x, y) * texel, p.z - bias));

    return 1.0 - lit / 9.0;
}

void main()
{
    // --- surface colour ---
    vec4 albedo = baseColor * vec4(color, 1.0);
    if (hasDiffuseTex != 0)
        albedo *= texture(diffuse0, texCoord);

    float specMask = (hasSpecularTex != 0) ? texture(specular0, texCoord).r : specularStrength;

    vec3 N = normalize(normal);
    vec3 L = normalize(-lightDirection);
    vec3 V = normalize(camPos - crntPos);
    vec3 H = normalize(L + V);

    // --- lighting ---
    float NdotL   = max(dot(N, L), 0.0);
    float shadow  = shadowAmount(crntPos, N, L);

    vec3 ambient  = albedo.rgb * mix(0.10, 0.25, N.y * 0.5 + 0.5);      // cheap sky/ground ambient
    vec3 diffuse  = albedo.rgb * NdotL * (1.0 - shadow) * lightColor.rgb;
    float spec    = (NdotL > 0.0) ? pow(max(dot(N, H), 0.0), shininess) : 0.0;
    vec3 specular = lightColor.rgb * spec * specMask * (1.0 - shadow);

    FragColor = vec4(ambient + diffuse + specular, albedo.a);
}
