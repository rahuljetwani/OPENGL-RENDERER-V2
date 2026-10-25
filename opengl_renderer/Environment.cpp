#include "Environment.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>
#include <iostream>

// unit cube, positions only (36 vertices)
static const float CUBE[] =
{
	-1,-1,-1,  1, 1,-1,  1,-1,-1,   1, 1,-1, -1,-1,-1, -1, 1,-1,
	-1,-1, 1,  1,-1, 1,  1, 1, 1,   1, 1, 1, -1, 1, 1, -1,-1, 1,
	-1, 1, 1, -1, 1,-1, -1,-1,-1,  -1,-1,-1, -1,-1, 1, -1, 1, 1,
	 1, 1, 1,  1,-1,-1,  1, 1,-1,   1,-1,-1,  1, 1, 1,  1,-1, 1,
	-1,-1,-1,  1,-1,-1,  1,-1, 1,   1,-1, 1, -1,-1, 1, -1,-1,-1,
	-1, 1,-1,  1, 1, 1,  1, 1,-1,   1, 1, 1, -1, 1,-1, -1, 1, 1
};

// the six cameras used to render into the faces of a cubemap
static const glm::mat4 CAPTURE_PROJ = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
static const glm::mat4 CAPTURE_VIEWS[6] =
{
	glm::lookAt(glm::vec3(0), glm::vec3( 1, 0, 0), glm::vec3(0,-1, 0)),
	glm::lookAt(glm::vec3(0), glm::vec3(-1, 0, 0), glm::vec3(0,-1, 0)),
	glm::lookAt(glm::vec3(0), glm::vec3( 0, 1, 0), glm::vec3(0, 0, 1)),
	glm::lookAt(glm::vec3(0), glm::vec3( 0,-1, 0), glm::vec3(0, 0,-1)),
	glm::lookAt(glm::vec3(0), glm::vec3( 0, 0, 1), glm::vec3(0,-1, 0)),
	glm::lookAt(glm::vec3(0), glm::vec3( 0, 0,-1), glm::vec3(0,-1, 0))
};

// empty float cubemap that we can render into
static GLuint makeCubemap(int size, bool mipmaps)
{
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
	for (int i = 0; i < 6; i++)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA16F, size, size, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return tex;
}

void Environment::drawCube()
{
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

// draws the cube from the inside once per face into `target` (shader needs uniforms "projection" and "view")
void Environment::renderToCubemap(Shader& shader, GLuint target, int size, int mip)
{
	shader.Activate();
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(CAPTURE_PROJ));
	glViewport(0, 0, size, size);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (int i = 0; i < 6; i++)
	{
		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(CAPTURE_VIEWS[i]));
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, target, mip);
		glClear(GL_COLOR_BUFFER_BIT);
		drawCube();
	}
}

// the six skybox pictures -> one cubemap. SRGB8 means the GPU gives us linear colours when we sample it.
void Environment::loadFaces()
{
	const char* faces[6] = { "skybox/negx.jpg", "skybox/posx.jpg", "skybox/posy.jpg", "skybox/negy.jpg", "skybox/posx.jpg", "skybox/negz.jpg" };

	glGenTextures(1, &envCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	stbi_set_flip_vertically_on_load(false);		// Texture.cpp turns this on for models, cubemaps must not be flipped
	for (int i = 0; i < 6; i++)
	{
		int w, h, channels;
		unsigned char* data = stbi_load(faces[i], &w, &h, &channels, 3);
		if (!data)
		{
			std::cout << "Failed to load " << faces[i] << std::endl;
			continue;
		}
		envSize = (float)w;
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_SRGB8, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);			// the convolutions below read blurrier mips
}

Environment::Environment()
{
	// remember the state we change, so the main loop is not affected
	GLint oldFBO, oldViewport[4];
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFBO);
	glGetIntegerv(GL_VIEWPORT, oldViewport);
	glDisable(GL_CULL_FACE);		// we look at the cube from the inside
	glDisable(GL_DEPTH_TEST);

	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE), CUBE, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glBindVertexArray(0);
	glGenFramebuffers(1, &captureFBO);

	loadFaces();

	// 1) diffuse: how bright is the sky when seen from a surface facing each direction
	irradianceMap = makeCubemap(32, false);
	{
		Shader s("ibl_cube.vert", "ibl_irradiance.frag");
		s.Activate();
		glUniform1i(glGetUniformLocation(s.ID, "environmentMap"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
		renderToCubemap(s, irradianceMap, 32, 0);
		s.Delete();
	}

	// 2) specular: the sky blurred by different amounts, mip 0 = sharp ... last mip = rough
	const int prefilterSize = 128;
	prefilterMap = makeCubemap(prefilterSize, true);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	{
		Shader s("ibl_cube.vert", "ibl_prefilter.frag");
		s.Activate();
		glUniform1i(glGetUniformLocation(s.ID, "environmentMap"), 0);
		glUniform1f(glGetUniformLocation(s.ID, "resolution"), envSize);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
		for (int mip = 0; mip < prefilterMips; mip++)
		{
			glUniform1f(glGetUniformLocation(s.ID, "roughness"), (float)mip / (float)(prefilterMips - 1));
			renderToCubemap(s, prefilterMap, prefilterSize >> mip, mip);
		}
		s.Delete();
	}

	// 3) BRDF lookup table (drawn with a fullscreen triangle, no vertex buffer needed)
	glGenTextures(1, &brdfLUT);
	glBindTexture(GL_TEXTURE_2D, brdfLUT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 256, 256, 0, GL_RG, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	{
		Shader s("ibl_brdf.vert", "ibl_brdf.frag");
		GLuint emptyVAO;
		glGenVertexArrays(1, &emptyVAO);
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUT, 0);
		glViewport(0, 0, 256, 256);
		s.Activate();
		glClear(GL_COLOR_BUFFER_BIT);
		glBindVertexArray(emptyVAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);
		glDeleteVertexArrays(1, &emptyVAO);
		s.Delete();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
	glViewport(oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3]);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
}

void Environment::BindIBL(Shader& pbr, GLuint firstUnit)
{
	pbr.Activate();
	glActiveTexture(GL_TEXTURE0 + firstUnit);		glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
	glActiveTexture(GL_TEXTURE0 + firstUnit + 1);	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
	glActiveTexture(GL_TEXTURE0 + firstUnit + 2);	glBindTexture(GL_TEXTURE_2D, brdfLUT);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(pbr.ID, "irradianceMap"), firstUnit);
	glUniform1i(glGetUniformLocation(pbr.ID, "prefilterMap"), firstUnit + 1);
	glUniform1i(glGetUniformLocation(pbr.ID, "brdfLUT"), firstUnit + 2);
	glUniform1f(glGetUniformLocation(pbr.ID, "prefilterMaxLod"), (float)(prefilterMips - 1));
}

void Environment::DrawSky(Shader& sky, const glm::mat4& view, const glm::mat4& projection)
{
	sky.Activate();
	glm::mat4 viewNoMove = glm::mat4(glm::mat3(view));		// the sky must not move when the camera moves
	glUniformMatrix4fv(glGetUniformLocation(sky.ID, "view"), 1, GL_FALSE, glm::value_ptr(viewNoMove));
	glUniformMatrix4fv(glGetUniformLocation(sky.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniform1i(glGetUniformLocation(sky.ID, "skybox"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	glDisable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);
	drawCube();
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
}
