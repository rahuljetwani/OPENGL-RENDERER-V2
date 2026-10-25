#version 330 core

// Physically based shading (Cook-Torrance GGX, metal/roughness workflow) + image based lighting.
// Output is LINEAR HDR radiance. Tone mapping and the sRGB encode happen later in framebuffer.frag.

out vec4 FragColor;

in vec3 crntPos;
in vec3 normal;
in vec3 color;
in vec2 texCoord;

// material
uniform sampler2D diffuse0;
uniform vec4  baseColor;
uniform int   hasDiffuseTex;
uniform float metallic;
uniform float roughness;

// one directional light ("sun"). lightColor.rgb is radiance = colour * intensity, so it can go well above 1.0 (HDR)
uniform vec4 lightColor;
uniform vec3 lightDirection;        // direction the light rays TRAVEL (pointing away from the light)
uniform vec3 camPos;

// image based lighting (see Environment.cpp)
uniform samplerCube irradianceMap;  // diffuse ambient
uniform samplerCube prefilterMap;   // specular ambient, one mip per roughness level
uniform sampler2D   brdfLUT;        // split-sum scale/bias
uniform float prefilterMaxLod;

// shadows
uniform sampler2DShadow shadowMap;  // hardware depth compare (see ShadowMap.cpp)
uniform mat4  lightSpaceMatrix;
uniform float shadowTexelWorld;     // size of one shadow texel in world units
uniform float shadowBias;           // ~0.0005 in [0,1] depth units

const float PI = 3.14159265359;

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

// ---- Cook-Torrance building blocks ----
float distributionGGX(float NdotH, float a)
{
    float a2 = a * a;
    float d  = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d);
}

float geometrySchlickGGX(float NdotX, float k)
{
    return NdotX / (NdotX * (1.0 - k) + k);
}

float geometrySmith(float NdotV, float NdotL, float r)
{
    float k = (r + 1.0) * (r + 1.0) / 8.0;      // direct-lighting remap
    return geometrySchlickGGX(NdotV, k) * geometrySchlickGGX(NdotL, k);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float r)
{
    return F0 + (max(vec3(1.0 - r), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    // --- surface ---
    vec4 albedo4 = baseColor * vec4(color, 1.0);
    if (hasDiffuseTex != 0)
        albedo4 *= texture(diffuse0, texCoord);
    vec3  albedo = albedo4.rgb;
    float rough  = clamp(roughness, 0.045, 1.0);        // a perfect mirror makes the sun a 1-pixel firefly
    float metal  = clamp(metallic, 0.0, 1.0);

    vec3 N = normalize(normal);
    vec3 V = normalize(camPos - crntPos);
    vec3 L = normalize(-lightDirection);
    vec3 H = normalize(V + L);
    vec3 R = reflect(-V, N);

    float NdotV = max(dot(N, V), 1e-4);
    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float HdotV = max(dot(H, V), 0.0);

    vec3 F0 = mix(vec3(0.04), albedo, metal);           // 4% reflectance for non-metals, tinted reflectance for metals
    float a = rough * rough;

    // --- direct light: the sun ---
    float D = distributionGGX(NdotH, a);
    float G = geometrySmith(NdotV, NdotL, rough);
    vec3  F = fresnelSchlick(HdotV, F0);

    vec3 specular = (D * G * F) / (4.0 * NdotV * NdotL + 1e-4);
    vec3 kD       = (vec3(1.0) - F) * (1.0 - metal);    // metals have no diffuse term
    float shadow  = (NdotL > 0.0) ? shadowAmount(crntPos, N, L) : 1.0;

    vec3 Lo = (kD * albedo / PI + specular) * lightColor.rgb * NdotL * (1.0 - shadow);

    // --- ambient: image based lighting ---
    vec3 Fa  = fresnelSchlickRoughness(NdotV, F0, rough);
    vec3 kDa = (vec3(1.0) - Fa) * (1.0 - metal);

    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuseIBL = irradiance * albedo;

    vec3 prefiltered = textureLod(prefilterMap, R, rough * prefilterMaxLod).rgb;
    vec2 brdf        = texture(brdfLUT, vec2(NdotV, rough)).rg;
    vec3 specularIBL = prefiltered * (Fa * brdf.x + brdf.y);

    vec3 ambient = kDa * diffuseIBL + specularIBL;

    FragColor = vec4(Lo + ambient, albedo4.a);
}
