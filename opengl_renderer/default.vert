#version 330 core

// Positions/Coordinates
layout (location = 0) in vec3 aPos;
// Normals 
layout (location = 1) in vec3 aNormal;
// Colors
layout (location = 2) in vec3 aColor;
// Texture Coordinates
layout (location = 3) in vec2 aTex;


out vec3 crntPos;                       //Output : current position for Fragment Shader
out vec3 normal;                        //Output : normal for Fragment Shader
out vec3 color;                         //output : color for fragment shader
out vec2 texCoord;                      //output : texture coordinates for frament 
out vec4 fragPosLight;

uniform mat4 camMatrix;                 // Imports the camera matrix from the main function
uniform mat4 model;                     // Imports the model matrix from the main function
uniform mat4 translation;
uniform mat4 rotation;
uniform mat4 scale;
uniform mat4 lightProjection;



void main()
{
    crntPos = vec3(model * translation * -rotation * scale * vec4(aPos, 1.0f));     // calculates current position
    normal = aNormal;                                                               // Assigns the normal from the Vertex Data to "Normal"    
    color = aColor;                                                                 // Assigns the colors from the Vertex Data to "color"
    texCoord = mat2(0.0, -1.0, 1.0, 0.0) * aTex;                                    // Assigns the texture coordinates from the Vertex Data to "texCoord"
    fragPosLight = lightProjection * vec4(crntPos, 1.0f);
    gl_Position = camMatrix * vec4(crntPos, 1.0);                              // Outputs the positions/coordinates of all vertices
    

}