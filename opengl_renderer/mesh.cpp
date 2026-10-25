#include "mesh.h"
#include "Profiler.h"
#include <limits>

mesh::mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, std::vector<Texture>& textures, const Material& material)
{
	mesh::vertices = vertices;
	mesh::indices = indices;
	mesh::textures = textures;
	mesh::material = material;

	// Local bounding box
	if (!vertices.empty())
	{
		boundsMin = glm::vec3(std::numeric_limits<float>::max());
		boundsMax = glm::vec3(std::numeric_limits<float>::lowest());
		for (const Vertex& v : vertices)
		{
			boundsMin = glm::min(boundsMin, v.position);
			boundsMax = glm::max(boundsMax, v.position);
		}
	}

	vao.Bind();

	VBO VBO(vertices);
	EBO EBO(indices);

	vao.LinkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)(0));
	vao.LinkAttrib(VBO, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
	vao.LinkAttrib(VBO, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
	vao.LinkAttrib(VBO, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float)));

	vao.Unbind();
	VBO.Unbind();
	EBO.Unbind();
}

void mesh::Draw(Shader& shader, Camera& camera, const glm::mat4& modelMatrix)
{
	shader.Activate();
	vao.Bind();

	// --- textures: bound to units 0,1,2... in order, named diffuse0, specular0, ... ---
	unsigned int numDiffuse = 0;
	unsigned int numSpecular = 0;
	bool hasDiffuse = false;
	bool hasSpecular = false;

	for (unsigned int i = 0; i < textures.size(); i++)
	{
		std::string num;
		const std::string& type = textures[i].type;

		if (type == "diffuse")
		{
			num = std::to_string(numDiffuse++);
			hasDiffuse = true;
		}
		else if (type == "specular")
		{
			num = std::to_string(numSpecular++);
			hasSpecular = true;
		}

		glUniform1i(glGetUniformLocation(shader.ID, (type + num).c_str()), i);
		textures[i].Bind(i);
	}

	// --- material ---
	glUniform4fv(glGetUniformLocation(shader.ID, "baseColor"), 1, glm::value_ptr(material.baseColor));
	glUniform1f(glGetUniformLocation(shader.ID, "metallic"), material.metallic);
	glUniform1f(glGetUniformLocation(shader.ID, "roughness"), material.roughness);
	glUniform1i(glGetUniformLocation(shader.ID, "hasDiffuseTex"), hasDiffuse);
	glUniform1i(glGetUniformLocation(shader.ID, "hasSpecularTex"), hasSpecular);

	// --- camera + transforms ---
	glUniform3f(glGetUniformLocation(shader.ID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);
	camera.Matrix(shader, "camMatrix");

	glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));	// correct under non-uniform scale too
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniformMatrix3fv(glGetUniformLocation(shader.ID, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));

	glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);

	// stats
	if (material.baseColor.a < 0.999f) gStats.transparentDraws++; else gStats.opaqueDraws++;
	gStats.trisVisible += TriangleCount();
}

void mesh::DrawDepth(Shader& shader, const glm::mat4& modelMatrix)
{
	shader.Activate();
	vao.Bind();
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
}
