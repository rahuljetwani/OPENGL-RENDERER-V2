#include "model.h"
#include "ShadowMap.h"
#include "RobotArm.h"
#include "Scene.h"
#include "Environment.h"
#include "HDRPipeline.h"
#include "Profiler.h"
#include "UI.h"
#include <iostream>

const GLuint SHADOW_UNIT = 8;			// texture unit of the shadow map (models use 0, 1, 2 ...)
const GLuint IBL_UNIT = 9;				// 9, 10, 11 = irradiance, prefilter, BRDF table

const unsigned int width = 1280;
const unsigned int height = 720;

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);		// the HDR framebuffer has a fixed size

	GLFWwindow* window = glfwCreateWindow(width, height, "OpenGL Digital Twin", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create the window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	gladLoadGL();
	glViewport(0, 0, width, height);

	UI::Init(window);

	Shader shaderProgram("default.vert", "default.frag");		// PBR shader
	Shader skyboxShader("skybox.vert", "skybox.frag");
	Shader shadowMapProgram("shadowmap.vert", "shadowmap.frag");

	// the sun: lightDirection is the way the rays travel
	glm::vec3 lightDirection = glm::normalize(glm::vec3(-0.5f, -1.0f, -0.4f));
	glm::vec3 sunColor = glm::vec3(1.0f, 0.97f, 0.9f);
	float sunIntensity = 4.0f;
	float exposureEV = 0.0f;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	Camera camera(width, height, glm::vec3(0.0f, 0.9f, 3.3f));

	RobotArm arm;
	if (!arm.IsLoaded()) std::cout << "A robot model failed to load, check the models/ folder" << std::endl;
	arm.Update();

	Scene scene;
	Environment environment("hdr/sample_sky.hdr");	// falls back to the jpg skybox if the file is missing
	HDRPipeline hdr(width, height);

	// the shadow map covers the whole scene, so the arm can move anywhere inside it
	ShadowMap shadowMap(2048);
	glm::vec3 sceneCentre;
	float sceneRadius;
	scene.GetShadowSphere(sceneCentre, sceneRadius);

	while (!glfwWindowShouldClose(window))
	{
		gProfiler.BeginFrame();

		UI::NewFrame();
		UI::StatsPanel(gProfiler);
		UI::ControlPanel(arm, scene, exposureEV, sunIntensity);

		{
			CpuScope timer(CPU_UPDATE);
			arm.Update();
			if (!UI::WantsMouse()) camera.inputs(window);	// don't turn the camera while dragging a slider
			camera.updateMatrix(45.0f, 0.1f, 100.0f);
		}

		// shadow pass
		gProfiler.GpuBegin(GPU_SHADOW);
		shadowMap.UpdateDirectional(lightDirection, sceneCentre, sceneRadius);
		shadowMap.Begin();
		shadowMapProgram.Activate();
		glUniformMatrix4fv(glGetUniformLocation(shadowMapProgram.ID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(shadowMap.lightSpace));
		arm.DrawDepth(shadowMapProgram);
		scene.DrawDepth(shadowMapProgram);
		shadowMap.End();
		gProfiler.GpuEnd();

		// main pass, into the HDR framebuffer
		gProfiler.GpuBegin(GPU_FORWARD);
		hdr.BeginScene();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderProgram.Activate();
		glm::vec3 sunRadiance = sunColor * sunIntensity;
		glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), sunRadiance.x, sunRadiance.y, sunRadiance.z, 1.0f);
		glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightDirection"), lightDirection.x, lightDirection.y, lightDirection.z);
		glUniform1f(glGetUniformLocation(shaderProgram.ID, "shadowBias"), 0.0005f);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(shadowMap.lightSpace));
		glUniform1f(glGetUniformLocation(shaderProgram.ID, "shadowTexelWorld"), shadowMap.WorldTexelSize());
		shadowMap.Bind(SHADOW_UNIT);
		glUniform1i(glGetUniformLocation(shaderProgram.ID, "shadowMap"), SHADOW_UNIT);
		environment.BindIBL(shaderProgram, IBL_UNIT);

		scene.Draw(shaderProgram, camera);
		arm.Draw(shaderProgram, camera);

		// sky last, so it only fills what is still empty
		glm::mat4 view = glm::lookAt(camera.Position, camera.Position + camera.orientation, camera.up);
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);
		environment.DrawSky(skyboxShader, view, projection);

		hdr.EndScene();
		gProfiler.GpuEnd();

		// tone mapping onto the window
		gProfiler.GpuBegin(GPU_POST);
		hdr.Resolve(exposureEV);
		gProfiler.GpuEnd();

		UI::Render();
		gProfiler.EndFrame();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	UI::Shutdown();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
