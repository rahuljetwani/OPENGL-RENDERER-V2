#ifndef MESH_CLASS_H
#define MESH_CLASS_H

#include<string>
#include<vector>

#include "VAO.h"
#include "EBO.h"
#include "Camera.h"
#include "Texture.h"

// what the PBR shader needs to know about a surface
struct Material
{
	glm::vec4 baseColor = glm::vec4(1.0f);	// albedo, multiplied with the texture if there is one
	float metallic  = 0.0f;					// 0 = plastic / paint, 1 = bare metal
	float roughness = 0.5f;					// 0 = mirror, 1 = very rough
};

class mesh
{
public:
	std::vector <Vertex> vertices;
	std::vector <GLuint> indices;
	std::vector <Texture> textures;			// only the diffuse texture is used
	Material material;

	// bounding box in local space, used for frustum culling
	glm::vec3 boundsMin = glm::vec3(0.0f);
	glm::vec3 boundsMax = glm::vec3(0.0f);

	VAO vao;

	mesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices, std::vector <Texture>& textures, const Material& material = Material());

	size_t TriangleCount() const { return indices.size() / 3; }

	// modelMatrix goes from local space to world space
	void Draw(Shader& shader, Camera& camera, const glm::mat4& modelMatrix);

	// shadow pass: geometry only
	void DrawDepth(Shader& shader, const glm::mat4& modelMatrix);
};
#endif
