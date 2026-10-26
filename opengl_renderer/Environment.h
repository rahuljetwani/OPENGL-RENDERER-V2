#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "shaderClass.h"

// Sky and the lighting that comes from it (image based lighting), built once at startup:
//   sky - environment cubemap
//       - irradiance map  (soft light for diffuse surfaces)
//       - prefilter map   (blurry reflections, one mip per roughness)
//   plus a BRDF lookup table for the PBR shader
class Environment
{
public:
	Environment(const char* hdrFile = "");

	// binds the three IBL textures to units firstUnit .. firstUnit + 2
	void BindIBL(Shader& pbrShader, GLuint firstUnit);

	void DrawSky(Shader& skyShader, const glm::mat4& view, const glm::mat4& projection);

private:
	GLuint envCubemap = 0, irradianceMap = 0, prefilterMap = 0, brdfLUT = 0;
	GLuint cubeVAO = 0, cubeVBO = 0, captureFBO = 0;
	float envSize = 0.0f;
	const int prefilterMips = 5;

	bool loadHDR(const char* file);
	void loadFaces();
	void renderToCubemap(Shader& shader, GLuint target, int size, int mip);
	void drawCube();
};

#endif
