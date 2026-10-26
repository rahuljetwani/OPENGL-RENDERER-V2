#ifndef HDR_PIPELINE_H
#define HDR_PIPELINE_H

#include <glad/glad.h>
#include "shaderClass.h"

// Draws the scene into a 16-bit float framebuffer (light can go above 1.0),
// then Resolve() applies exposure and tone mapping and shows it on the window.
class HDRPipeline
{
public:
	HDRPipeline(int width, int height);

	void BeginScene();					// start drawing into the HDR image
	void EndScene();					// back to the window
	void Resolve(float exposureEV);		// tone map onto the window

private:
	int width, height;
	GLuint fbo = 0, colorTex = 0, depthRbo = 0;
	GLuint quadVAO = 0, quadVBO = 0;
	Shader shader;
};

#endif
