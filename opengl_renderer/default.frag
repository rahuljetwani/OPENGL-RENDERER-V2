#version 330 core


out vec4 FragColor;             // Outputs colors in RGBA


in vec3 crntPos;                // Imports the current position from the Vertex Shader
in vec3 normal;                 // Imports the normal from the Vertex Shader
in vec3 color;                  // Inputs the color from the Vertex Shader
in vec2 texCoord;               // Inputs the texture coordinates from the Vertex Shader
in vec4 fragPosLight;


uniform sampler2D diffuse0;         // Gets the Texture Unit from the main function
uniform sampler2D specular0;         // Gets the Texture Unit from the main function
uniform sampler2D shadowMap; 
uniform vec4 lightColor;        // Gets the color of the light from the main function
uniform vec3 lightPos;          // Gets the position of the light from the main function
uniform vec3 camPos;            // Gets the position of the camera from the main function


vec4 directionLight()
{
    // ambient lighting
    float ambient = 0.05f;  
    
    // diffuse lighting
    vec3 norm = normalize(normal);
    vec3 lightDirection = normalize(vec3(1.0f, 1.0f, 0.0f));
    float diffuse  = max(dot(norm, lightDirection), 0.0f);
    
    // specular lighting
    float specularLight = 0.50f;
    vec3 viewDirection  = normalize(camPos - crntPos);
    vec3 reflectionDirection = reflect(-lightDirection, norm);
    float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 20);
    float specular = specAmount * specularLight;

    float shadow = 0.0f;
    vec3 lightCoords = fragPosLight.xyz / fragPosLight.w;
    if(lightCoords.z <= 1.0f)
    {
		// Get from [-1, 1] range to [0, 1] range just like the shadow map
		lightCoords = (lightCoords + 1.0f) / 2.0f;
		float currentDepth = lightCoords.z;
		// Prevents shadow acne
		float bias = max(0.025f * (1.0f - dot(normal, lightDirection)), 0.0005f);

		// Smoothens out the shadows
		int sampleRadius = 2;
		vec2 pixelSize = 1.0 / textureSize(shadowMap, 0);
		for(int y = -sampleRadius; y <= sampleRadius; y++)
		{
		    for(int x = -sampleRadius; x <= sampleRadius; x++)
		    {
		        float closestDepth = texture(shadowMap, lightCoords.xy + vec2(x, y) * pixelSize).r;
				if (currentDepth > closestDepth + bias)
					shadow += 1.0f;     
		    }    
		}
		// Get average shadow
		shadow /= pow((sampleRadius * 2 + 1), 2);

	}

    return (texture(diffuse0, texCoord) * (diffuse * (1.0f - shadow) + ambient) + texture(specular0, texCoord).r * specular  * (1.0f - shadow)) * lightColor;
}

vec4 pointLight()
{
    vec3 lightVec = lightPos - crntPos;
    float dist = length(lightVec);
    float a = 0.08;
    float b = 0.04;
    float inten = 1.0f / (a * dist * dist + b * dist + 1.0f);

    // ambient lighting
    float ambient = 0.025f;  
    
    // diffuse lighting
    vec3 norm = normalize(normal);
    vec3 lightDirection = normalize(lightVec);
    float diffuse  = max(dot(norm, lightDirection), 0.0f);
    
    
    // specular lighting
    float specular = 0.0f;
    if(diffuse != 0.0f)
    {
        float specularLight = 0.50f;
        vec3 viewDirection  = normalize(camPos - crntPos);
        vec3 reflectionDirection = reflect(-lightDirection, norm);

        vec3 halfWayVec = normalize(viewDirection + lightDirection);

        float specAmount = pow(max(dot(normal, halfWayVec), 0.0f), 20);
        specular = specAmount * specularLight;
    };
    return (texture(diffuse0, texCoord) * lightColor * (diffuse + inten + ambient) + texture(specular0, texCoord).r * specular * inten) * lightColor;
}

vec4 spotLight()
{
    float innerCone = 0.95f;
    float outerCone = 0.90f;
    
    // ambient lighting
    float ambient = 0.20f;  
    
    // diffuse lighting
    vec3 norm = normalize(normal);
    vec3 lightDirection = normalize(lightPos - crntPos);
    float diffuse  = max(dot(norm, lightDirection), 0.0f);
    
    // specular lighting
    float specularLight = 0.50f;
    vec3 viewDirection  = normalize(camPos - crntPos);
    vec3 reflectionDirection = reflect(-lightDirection, norm);

    float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 20);
    float specular = specAmount * specularLight;

    float angle = dot(vec3(0.0f, -1.0f, 0.0f), -lightDirection);
    float inten = clamp((angle - outerCone) / (innerCone - outerCone), 0.0f, 1.f);

    return (texture(diffuse0, texCoord) * lightColor * (diffuse + inten + ambient) + texture(specular0, texCoord).r * specular * inten) * lightColor;
}

void main()
{
    
    FragColor = directionLight();
}