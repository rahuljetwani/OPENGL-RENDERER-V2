#ifndef HDR_PIPELINE_H
#define HDR_PIPELINE_H

#include <glad/glad.h>
#include "shaderClass.h"

// The scene is drawn into a 16-bit float framebuffer, so light values can go above 1.0.
// Resolve() then applies exposure + tone mapping + sRGB and draws the result to the window.
class HDRPipeline
{
public:
	HDRPipeline(int width, int height);

	void BeginScene();					// start drawing into the HDR framebuffer
	void EndScene();					// back to the window
	void Resolve(float exposureEV);		// tone map the HDR image onto the window

private:
	int width, height;
	GLuint fbo = 0, colorTex = 0, depthRbo = 0;
	GLuint quadVAO = 0, quadVBO = 0;
	Shader shader;
};

#endif
