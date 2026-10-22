#ifndef SHADOWMAP_CLASS_H
#define SHADOWMAP_CLASS_H

#include<glad/glad.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include"shaderClass.h"

// Directional-light shadow map.
//
//   shadow.UpdateDirectional(lightDir, sceneCentre, sceneRadius);   // once per frame (or when light/scene moves)
//   shadow.Begin();
//       ...draw shadow casters with the depth shader (uniform "lightSpaceMatrix" = shadow.lightSpace)...
//   shadow.End();
//   ...main pass: shadow.Bind(unit) + upload shadow.lightSpace / shadow.WorldTexelSize()...
class ShadowMap
{
public:
	GLuint fbo = 0;
	GLuint depthTex = 0;
	unsigned int size = 0;

	// world -> light clip space. Upload this to both the depth shader and the main shader.
	glm::mat4 lightSpace = glm::mat4(1.0f);

	ShadowMap(unsigned int resolution = 2048);

	// Fit an orthographic light frustum around a sphere (centre, radius) for light rays travelling along lightDir.
	// Keep `radius` constant from frame to frame (e.g. computed once, with margin for animation): a changing radius
	// changes texel size and makes shadow edges shimmer. The centre may move freely (it is snapped to texels).
	void UpdateDirectional(const glm::vec3& lightDir, const glm::vec3& sceneCentre, float sceneRadius);

	void Begin();		// bind shadow FBO, set viewport, clear, enable depth clamp + slope-scaled offset
	void End();			// restore previous framebuffer and viewport

	void Bind(GLuint unit) const;

	// Size of one shadow-map texel in world units (used by the shader for normal-offset bias)
	float WorldTexelSize() const { return worldTexel; }

	void Delete();

private:
	float worldTexel = 0.01f;
	GLint prevFBO = 0;
	GLint prevViewport[4] = { 0, 0, 0, 0 };
};
#endif
