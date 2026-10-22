#include"ShadowMap.h"
#include<iostream>
#include<cmath>

ShadowMap::ShadowMap(unsigned int resolution) : size(resolution)
{
	glGenTextures(1, &depthTex);
	glBindTexture(GL_TEXTURE_2D, depthTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, size, size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	// LINEAR + compare mode = the GPU does a bilinear-filtered depth comparison per tap ("free" 2x2 PCF)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	// Outside the map = fully lit (depth 1.0 is "nothing in front")
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float border[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
	glDrawBuffer(GL_NONE);		// depth only
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "[ShadowMap] framebuffer incomplete\n";

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void ShadowMap::UpdateDirectional(const glm::vec3& lightDir, const glm::vec3& sceneCentre, float sceneRadius)
{
	glm::vec3 dir = glm::normalize(lightDir);
	glm::vec3 up = (std::fabs(dir.y) > 0.99f) ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);

	// Rotation-only view: light at the origin looking along `dir`
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f), dir, up);

	// Scene centre in light space. Snap x/y to whole texels so the shadow doesn't crawl when the centre moves.
	glm::vec3 c = glm::vec3(view * glm::vec4(sceneCentre, 1.0f));
	worldTexel = (2.0f * sceneRadius) / (float)size;
	c.x = std::floor(c.x / worldTexel) * worldTexel;
	c.y = std::floor(c.y / worldTexel) * worldTexel;

	// In view space the light looks down -Z, so the scene sits at z = c.z (negative).
	// ortho() takes near/far as distances along -Z.
	glm::mat4 proj = glm::ortho(
		c.x - sceneRadius, c.x + sceneRadius,
		c.y - sceneRadius, c.y + sceneRadius,
		-c.z - sceneRadius, -c.z + sceneRadius);

	lightSpace = proj * view;
}

void ShadowMap::Begin()
{
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);
	glGetIntegerv(GL_VIEWPORT, prevViewport);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, size, size);			// <- the shadow map's own resolution, NOT the window's
	glClear(GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_CLAMP);				// casters in front of the near plane are clamped instead of clipped away
	glEnable(GL_POLYGON_OFFSET_FILL);		// slope-scaled depth bias against shadow acne
	glPolygonOffset(2.0f, 4.0f);
}

void ShadowMap::End()
{
	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_DEPTH_CLAMP);

	glBindFramebuffer(GL_FRAMEBUFFER, prevFBO);
	glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
}

void ShadowMap::Bind(GLuint unit) const
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, depthTex);
}

void ShadowMap::Delete()
{
	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &depthTex);
	fbo = depthTex = 0;
}
