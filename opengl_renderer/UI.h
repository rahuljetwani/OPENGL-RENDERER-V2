#ifndef UI_H
#define UI_H

#include <glad/glad.h>		// must come before GLFW
#include <GLFW/glfw3.h>
#include "Profiler.h"
#include "RobotArm.h"
#include "Scene.h"

// Dear ImGui. Each frame: NewFrame, the panels, draw the scene, Render.
namespace UI
{
	void Init(GLFWwindow* window);
	void Shutdown();
	void NewFrame();
	void Render();

	// true when the mouse is over a panel, so the camera should ignore it
	bool WantsMouse();

	// fps, CPU / GPU times, draw calls, triangles
	void StatsPanel(const Profiler& profiler);

	// joint sliders, lighting, robot material and scene options
	void ControlPanel(RobotArm& arm, Scene& scene, float& exposureEV, float& sunIntensity);
}

#endif
