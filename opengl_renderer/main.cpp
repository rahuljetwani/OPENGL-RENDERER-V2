#include "model.h"
#include "ShadowMap.h"
#include "RobotArm.h"
#include "Environment.h"
#include "HDRPipeline.h"
#include "Profiler.h"
#include "UI.h"

const GLuint SHADOW_UNIT = 8;				// texture unit for the shadow map (material textures use 0,1,2...)
const GLuint IBL_UNIT = 9;					// 9 = irradiance, 10 = prefilter, 11 = BRDF LUT

const unsigned int width = 1280;			// width of the window
const unsigned int height = 720;			// height of the window

const bool PRINT_LINK_BOUNDS = true;		// startup checkpoint: prints the raw bounds of each link

int main()
{
	glfwInit();   //initalize GLFW

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);    //tell GLFW version of OpenGL we are using
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);	  //which is 3 in our case
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);   //tell GLFW we are using "CORE" profile
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);		  // the HDR framebuffer has a fixed size

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

	// default.frag is the PBR shader
	Shader shaderProgram("default.vert", "default.frag");
	Shader skyboxShader("skybox.vert", "skybox.frag");
	Shader shadowMapProgram("shadowmap.vert", "shadowmap.frag");
	Shader lightShader("light.vert", "light.frag");

	// Directional light (the "sun"). lightPos is only used to place the little light cube.
	glm::vec3 lightPos = glm::vec3(1.5f, 3.0f, 1.0f);
	glm::vec3 lightTarget = glm::vec3(0.0f, 0.5f, 0.0f);
	glm::vec3 lightDirection = glm::normalize(lightTarget - lightPos);	// direction the rays travel
	glm::vec3 sunColor = glm::vec3(1.0f, 0.97f, 0.9f);
	float sunIntensity = 4.0f;				// HDR: can be much bigger than 1
	float exposureEV = 0.0f;

	// --- Temp light cube (uses your existing light.vert / light.frag) ---
	std::vector<Vertex> lightVertices =
	{
		Vertex{glm::vec3(-0.05f, -0.05f,  0.05f)},
		Vertex{glm::vec3(-0.05f, -0.05f, -0.05f)},
		Vertex{glm::vec3( 0.05f, -0.05f, -0.05f)},
		Vertex{glm::vec3( 0.05f, -0.05f,  0.05f)},
		Vertex{glm::vec3(-0.05f,  0.05f,  0.05f)},
		Vertex{glm::vec3(-0.05f,  0.05f, -0.05f)},
		Vertex{glm::vec3( 0.05f,  0.05f, -0.05f)},
		Vertex{glm::vec3( 0.05f,  0.05f,  0.05f)}
	};

	std::vector<GLuint> lightIndices =
	{
		0,1,2,  0,2,3,   // bottom
		4,5,6,  4,6,7,   // top
		0,4,7,  0,7,3,   // front
		1,5,6,  1,6,2,   // back
		3,7,6,  3,6,2,   // right
		0,4,5,  0,5,1    // left
	};

	VAO lightVAO;
	lightVAO.Bind();
	VBO lightVBO(lightVertices);
	EBO lightEBO(lightIndices);
	lightVAO.LinkAttrib(lightVBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
	lightVAO.Unbind();
	lightVBO.Unbind();
	lightEBO.Unbind();

	glEnable(GL_DEPTH_TEST);			// Enables the Depth Buffer
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	Camera camera(width, height, glm::vec3(0.0f, 0.7f, 2.5f)); // Creates camera object (the arm is ~1.25 tall now)

	// The digital twin: floor -> base -> forearm -> arm. All numbers come from arm.config.
	RobotArm arm;
	if (!arm.IsLoaded()) std::cout << "RobotArm: a model failed to load, check the models/ paths" << std::endl;
	if (PRINT_LINK_BOUNDS) arm.PrintLinkBounds();	// before the first Update(): transforms are still identity
	arm.Update();

	Environment environment;			// skybox + image based lighting (takes a moment at startup)
	HDRPipeline hdr(width, height);		// floating point framebuffer + tone mapping

	// Shadow map: the arm moves, so frame the light around everything it can reach, not just today's pose
	ShadowMap shadowMap(2048);
	glm::vec3 sceneCentre;
	float sceneRadius;
	arm.GetShadowSphere(sceneCentre, sceneRadius);

	//MAIN WHILE LOOP
	while (!glfwWindowShouldClose(window))
	{
		gProfiler.BeginFrame();

		// ---- UI panels (the sliders change arm.state directly) ----
		UI::NewFrame();
		UI::StatsPanel(gProfiler);
		UI::RobotPanel(arm, exposureEV, sunIntensity);

		// ---- update: joint angles -> FK -> model transforms, camera ----
		{
			CpuScope timer(CPU_UPDATE);
			arm.Update();
			if (!UI::WantsMouse()) camera.inputs(window);	// dragging a slider must not turn the camera
			camera.updateMatrix(45.0f, 0.1f, 100.0f);
		}

		// ---- shadow pass: render the scene's depth as seen from the light ----
		gProfiler.GpuBegin(GPU_SHADOW);
		shadowMap.UpdateDirectional(lightDirection, sceneCentre, sceneRadius);
		shadowMap.Begin();
		shadowMapProgram.Activate();
		glUniformMatrix4fv(glGetUniformLocation(shadowMapProgram.ID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(shadowMap.lightSpace));
		arm.DrawDepth(shadowMapProgram);
		shadowMap.End();				// restores the previous framebuffer + viewport
		gProfiler.GpuEnd();

		// ---- main pass: PBR lighting into the floating point HDR framebuffer ----
		gProfiler.GpuBegin(GPU_FORWARD);
		hdr.BeginScene();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// light cube (brighter than 1.0 because the image is HDR)
		lightShader.Activate();
		camera.Matrix(lightShader, "camMatrix");
		glm::mat4 lightModel = glm::translate(glm::mat4(1.0f), lightPos);
		glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(lightModel));
		glUniform4f(glGetUniformLocation(lightShader.ID, "lightColor"), sunColor.x * 6.0f, sunColor.y * 6.0f, sunColor.z * 6.0f, 1.0f);
		lightVAO.Bind();
		glDrawElements(GL_TRIANGLES, (GLsizei)lightIndices.size(), GL_UNSIGNED_INT, 0);

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

		arm.Draw(shaderProgram, camera);

		// skybox last, so it only fills the pixels nothing else covered
		glm::mat4 view = glm::lookAt(camera.Position, camera.Position + camera.orientation, camera.up);
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);
		environment.DrawSky(skyboxShader, view, projection);

		hdr.EndScene();
		gProfiler.GpuEnd();

		// ---- post: exposure + tone mapping onto the window ----
		gProfiler.GpuBegin(GPU_POST);
		hdr.Resolve(exposureEV);
		gProfiler.GpuEnd();

		UI::Render();					// the panels go on top of the finished image
		gProfiler.EndFrame();

		glfwSwapBuffers(window);		//swap back buffer with front buffer
		glfwPollEvents();				//taking care of all GLFW events
	}

	UI::Shutdown();
	lightShader.Delete();
	glfwDestroyWindow(window);		//delete window before close
	glfwTerminate();				//terminate GLFW
	return 0;
}
