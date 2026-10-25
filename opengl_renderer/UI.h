#ifndef UI_H
#define UI_H

#include <glad/glad.h>		// must come before GLFW
#include <GLFW/glfw3.h>
#include "Profiler.h"
#include "RobotArm.h"

// Dear ImGui. Per frame:  UI::NewFrame();  panels...;  (draw the scene)  UI::Render();
namespace UI
{
	void Init(GLFWwindow* window);
	void Shutdown();
	void NewFrame();
	void Render();

	bool WantsMouse();		// pointer is over a panel, so the camera should ignore the mouse

	// FPS, CPU / GPU times, draw calls, triangles
	void StatsPanel(const Profiler& profiler);

	// joint angle sliders + a few lighting / material controls
	void RobotPanel(RobotArm& arm, float& exposureEV, float& sunIntensity);
}

#endif
