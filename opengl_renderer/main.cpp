#include "model.h"
#include "ShadowMap.h"

const GLuint SHADOW_UNIT = 8;				// texture unit for the shadow map (material textures use 0,1,2...)

const unsigned int width = 900;				//width of the screen
const unsigned int height = 900;			//height of the screen

float gamma = 2.2f;

float rectangleVertices[] =
{
	//coords		//texCoords
	 1.0f, -1.0f,	1.0f, 0.0f,
	-1.0f, -1.0f,	0.0f, 0.0f,
	-1.0f,  1.0f,	0.0f, 1.0f,

	 1.0f,  1.0f,	1.0f, 1.0f,
	 1.0f, -1.0f,	1.0f, 0.0f,
	-1.0f,  1.0f,	0.0f, 1.0f
};


float skyboxVertices[] =
{
	//   Coordinates
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f
};

unsigned int skyboxIndices[] =
{
	// Right
	1, 2, 6,
	6, 5, 1,
	// Left
	0, 4, 7,
	7, 3, 0,
	// Top
	4, 5, 6,
	6, 7, 4,
	// Bottom
	0, 3, 2,
	2, 1, 0,
	// Back
	0, 1, 5,
	5, 4, 0,
	// Front
	3, 7, 6,
	6, 2, 3
};


int main()
{
	glfwInit();   //initalize GLFW 
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);    //tell GLFW version of OpenGL we are using 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);	    //which is 3 in our case
	
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);   //tell GLFW we are using "CORE" profile 
																//which means we are only using morden functions

	GLFWwindow* window = glfwCreateWindow(width, height, "OpenGl window title", NULL, NULL);  //creatig window size 1280x720 and its name
	
	glfwMakeContextCurrent(window);												//this connects the current window to the context

	gladLoadGL();   //loading GLAD

	glViewport(0, 0, width, height);											//specify the viewport of openGL in the window


	// Generates Shader object using shaders default.vert and default.frag
	Shader shaderProgram("default.vert", "default.frag");
	Shader skyboxShader("skybox.vert", "skybox.frag");
	Shader framebufferProgram("framebuffer.vert", "framebuffer.frag");
	Shader shadowMapProgram("shadowmap.vert", "shadowmap.frag");

	// Directional light (the "sun" of the factory). lightPos is only used to place the little debug cube.
	glm::vec3 lightPos = glm::vec3(1.5f, 3.0f, 1.0f);
	glm::vec3 lightTarget = glm::vec3(0.0f, 0.5f, 0.0f);
	glm::vec3 lightDirection = glm::normalize(lightTarget - lightPos);	// direction the rays travel
	glm::vec4 lightColor = glm::vec4(1.0f, 0.97f, 0.9f, 1.0f);

	shaderProgram.Activate();
	glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightDirection"), lightDirection.x, lightDirection.y, lightDirection.z);
	glUniform1f(glGetUniformLocation(shaderProgram.ID, "shadowBias"), 0.0005f);
	skyboxShader.Activate();
	glUniform1i(glGetUniformLocation(skyboxShader.ID, "skybox"), 0);
	framebufferProgram.Activate();
	glUniform1i(glGetUniformLocation(framebufferProgram.ID, "screenTexture"), 0);
	glUniform1f(glGetUniformLocation(framebufferProgram.ID, "gamma"), gamma);

	// --- Temp light cube (uses your existing light.vert / light.frag) ---
	Shader lightShader("light.vert", "light.frag");

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

	glEnable(GL_FRAMEBUFFER_SRGB);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	

	Camera camera(width, height, glm::vec3(0.0f, 0.8f, 2.5f)); // Creates camera object

	// Any format Assimp supports works here (gltf, glb, fbx, obj, dae, stl...).ETC
	// Relative to the working directory, same as the shader files.
	model robot("models/floor.fbx");								//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	robot.FitToUnitSize(1.0f);		// 1 unit tall/wide, standing on y = 0  
	// robot.PrintHierarchy();		// prints the node tree

	// Ground plane so shadows have something to land on
	std::vector<Vertex> groundVerts =
	{
		Vertex{ glm::vec3(-3.0f, 0.0f,  3.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f), glm::vec2(0.0f, 0.0f) },
		Vertex{ glm::vec3( 3.0f, 0.0f,  3.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f), glm::vec2(1.0f, 0.0f) },
		Vertex{ glm::vec3( 3.0f, 0.0f, -3.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f), glm::vec2(1.0f, 1.0f) },
		Vertex{ glm::vec3(-3.0f, 0.0f, -3.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f), glm::vec2(0.0f, 1.0f) }
	};
	std::vector<GLuint> groundIndices = { 0, 1, 2,  0, 2, 3 };
	std::vector<Texture> groundTextures;
	Material groundMaterial;
	groundMaterial.baseColor = glm::vec4(0.45f, 0.47f, 0.50f, 1.0f);
	groundMaterial.specularStrength = 0.05f;
	mesh ground(groundVerts, groundIndices, groundTextures, groundMaterial);

	unsigned int rectVAO, rectVBO;
	glGenVertexArrays(1, &rectVAO);
	glGenBuffers(1, &rectVBO);
	glBindVertexArray(rectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), &rectangleVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));


	//FPS COUNTER
	double prevTime = 0.0;
	double crntTime = 0.0;
	double timeDiff;
	unsigned int counter = 0;



	unsigned int skyboxVAO, skyboxVBO, skyboxEBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glGenBuffers(1, &skyboxEBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	std::string faceCubemap[6] =
	{
		"F:/Rahul30.12.2020/openGL/Graphics/skybox/right.jpg",
		"F:/Rahul30.12.2020/openGL/Graphics/skybox/left.jpg",
		"F:/Rahul30.12.2020/openGL/Graphics/skybox/top.jpg",
		"F:/Rahul30.12.2020/openGL/Graphics/skybox/bottom.jpg",
		"F:/Rahul30.12.2020/openGL/Graphics/skybox/front.jpg",
		"F:/Rahul30.12.2020/openGL/Graphics/skybox/back.jpg"
	};

	unsigned int cubemapTexture;
	glGenTextures(1, &cubemapTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	for (unsigned int i = 0; i < 6; i++)
	{
		int width, height, nrChannels;
		unsigned char* data = stbi_load(faceCubemap[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			stbi_set_flip_vertically_on_load(false);
			glTexImage2D
			(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				GL_RGB,
				width,
				height,
				0,
				GL_RGB,
				GL_UNSIGNED_BYTE,
				data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "failed to Cubemap Texture" << faceCubemap[i] << std::endl;
			stbi_image_free(data);
		}
	}


	//frameBuffer //
	unsigned int FBO;
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	unsigned int framebufferTex;
	glGenTextures(1, &framebufferTex);
	glBindTexture(GL_TEXTURE_2D, framebufferTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTex, 0);

	unsigned int RBO;
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

	//---------------------------------------------------------------//
	//ERROR CHECK
		auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "FRAME_ERROR: " << fboStatus << std::endl;
	//---------------------------------------------------------------//


	// Shadow map: 2048^2 depth texture. The light frustum is fitted around the model ONCE with some margin
	// (so it also covers the model when joints move) - keeping the radius constant keeps shadow edges from shimmering
	ShadowMap shadowMap(2048);
	glm::vec3 boundsMin, boundsMax;
	robot.GetBounds(boundsMin, boundsMax);
	glm::vec3 sceneCentre = 0.5f * (boundsMin + boundsMax);
	float sceneRadius = 0.5f * glm::length(boundsMax - boundsMin) * 1.25f;

	//MAIN WHILE LOOP
	while(!glfwWindowShouldClose(window))
	{
		//FPSCOUNTER
		crntTime = glfwGetTime();
		timeDiff = crntTime - prevTime;
		counter++;
		if (timeDiff >= 1.0 / 30.0)
		{
			std::string FPS = std::to_string((1.0 / timeDiff) * counter);
			std::string ms = std::to_string((timeDiff / counter) * 1000);
			std::string updatedTitle = "OPENGL <3 - " + FPS + " FPS / " + ms + "ms";
			glfwSetWindowTitle(window, updatedTitle.c_str());
			prevTime = crntTime;
			counter = 0;
		}

		// ---- shadow pass: render the scene's depth as seen from the light ----
		shadowMap.UpdateDirectional(lightDirection, sceneCentre, sceneRadius);
		shadowMap.Begin();
		shadowMapProgram.Activate();
		glUniformMatrix4fv(glGetUniformLocation(shadowMapProgram.ID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(shadowMap.lightSpace));
		robot.DrawDepth(shadowMapProgram);
		shadowMap.End();				// restores the previous framebuffer + viewport

		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);						//color of background
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);			//clean the back buffer and asign new color to it

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		camera.inputs(window);										 // Handles camera inputs
		camera.updateMatrix(45.0f, 0.1f, 100.0f);					 // Updates and exports the camera matrix to the Vertex Shader

		lightShader.Activate();
		camera.Matrix(lightShader, "camMatrix");

		glm::mat4 lightModel = glm::translate(glm::mat4(1.0f), lightPos);
		glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(lightModel));
		glUniform4f(glGetUniformLocation(lightShader.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);

		lightVAO.Bind();
		glDrawElements(GL_TRIANGLES, lightIndices.size(), GL_UNSIGNED_INT, 0);

		// ---- colour pass ----
		shaderProgram.Activate();
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(shadowMap.lightSpace));
		glUniform1f(glGetUniformLocation(shaderProgram.ID, "shadowTexelWorld"), shadowMap.WorldTexelSize());
		shadowMap.Bind(SHADOW_UNIT);
		glUniform1i(glGetUniformLocation(shaderProgram.ID, "shadowMap"), SHADOW_UNIT);

		ground.Draw(shaderProgram, camera, glm::mat4(1.0f));
		robot.Draw(shaderProgram, camera);

		glDepthFunc(GL_LEQUAL);
		glDisable(GL_CULL_FACE);
		skyboxShader.Activate();
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);
		view = glm::mat4(glm::mat3(glm::lookAt(camera.Position, camera.Position + camera.orientation, camera.up)));
		projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		framebufferProgram.Activate();
		glBindVertexArray(rectVAO);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glBindTexture(GL_TEXTURE_2D, framebufferTex);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);														 //swap back buffer with front buffer
		glfwPollEvents();																 //taking care of all GLFW events
	}
	
	
	lightShader.Delete();
	glfwDestroyWindow(window);		//delete window before close
	glfwTerminate();				//terminate GLFW 
	return 0;
}

