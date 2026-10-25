#ifndef MESH_CLASS_H
#define MESH_CLASS_H

#include<string>
#include<vector>

#include "VAO.h"
#include "EBO.h"
#include "Camera.h"
#include "Texture.h"

// Everything the fragment shader needs to know about a surface.
// Textures live in mesh::textures; this holds the numeric part (colour factor, shininess).
struct Material
{
	glm::vec4 baseColor = glm::vec4(1.0f);	// albedo: multiplied with the diffuse texture (or used alone if there is none)
	float metallic  = 0.0f;					// PBR metal/roughness workflow: 0 = dielectric (paint, plastic), 1 = bare metal
	float roughness = 0.5f;					// 0 = mirror, 1 = fully diffuse-looking
	float shininess = 32.0f;				// (legacy Blinn-Phong values, no longer used by default.frag)
	float specularStrength = 0.25f;
};

class mesh
{
public:
	std::vector <Vertex> vertices;
	std::vector <GLuint> indices;
	std::vector <Texture> textures;
	Material material;

	// Local-space bounding box (used for camera framing / shadow fitting)
	glm::vec3 boundsMin = glm::vec3(0.0f);
	glm::vec3 boundsMax = glm::vec3(0.0f);

	VAO vao;

	mesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices, std::vector <Texture>& textures, const Material& material = Material());

	// Full colour draw. modelMatrix = local -> world. Updates the draw-call / triangle counters (Profiler.h).
	void Draw(Shader& shader, Camera& camera, const glm::mat4& modelMatrix);

	size_t TriangleCount() const { return indices.size() / 3; }

	// Geometry only (shadow pass): sets "model" and draws. No textures, no camera.
	void DrawDepth(Shader& shader, const glm::mat4& modelMatrix);
};
#endif
