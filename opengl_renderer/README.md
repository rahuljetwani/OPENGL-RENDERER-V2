# OpenGL Renderer V1

A **C++ / OpenGL learning project** I’m building to understand real-time graphics and rendering at a lower level.

I’m a computer science student interested in **graphics programming, rendering, and real-time 3D**. This project was one of my early steps into understanding **3D graphics in detail and what happens under the hood**, beyond using higher-level tools and game engines.

The goal is to learn the concepts by implementing them myself and seeing how the different parts of a 3D rendering pipeline come together.

**Status:** Ongoing learning project
**Language:** C++
**API:** OpenGL 3.3 Core

## What I've Implemented

* OpenGL 3.3 Core setup with GLFW, GLAD and GLM
* Custom VAO, VBO and EBO wrappers
* Basic shader management
* Camera and input handling
* Textures and materials
* Basic lighting
* Point lights and spotlights
* Depth and stencil testing
* Framebuffers and post-processing
* Cubemaps / skybox
* 3D model loading
* GLSL shader experiments

### Model Loading

I initially built a **custom model importer as a learning exercise** to understand how model data is actually parsed and turned into meshes.

It uses `nlohmann/json` to read the model data and handles things such as vertices, normals, UVs, indices, textures and node transformations.

Now that I understand the concepts involved, using something like **Assimp** would be the more practical choice for a proper renderer. It would provide a much more complete asset-import pipeline without having to maintain the importer myself.

## Rendering Experiments

Some of the things currently implemented or tested:

* Basic colored triangle
* Point light
* Spotlight
* Multiple 3D model imports
* Framebuffer rendering
* Black & white post-processing
* Inverse color post-processing
* Stencil testing
* Cubemap / skybox

## Screenshots

### Basic Triangle

![Basic Triangle]([screenshots/triangle.png](https://github.com/rahuljetwani/OPENGL-RENDERER-V1/blob/5bf6aeb28933c3e7649d2d358c34df586d1cedce/screenshots/triangle.jpg))

### Lighting

![Point Light]([screenshots/point-light.png](https://github.com/rahuljetwani/OPENGL-RENDERER-V1/blob/5bf6aeb28933c3e7649d2d358c34df586d1cedce/screenshots/pointLight.PNG))

![Spotlight]([screenshots/spotlight.png](https://github.com/rahuljetwani/OPENGL-RENDERER-V1/blob/5bf6aeb28933c3e7649d2d358c34df586d1cedce/screenshots/spotLight.PNG))

### Model Loading

![Model 1]([screenshots/model-1.png](https://github.com/rahuljetwani/OPENGL-RENDERER-V1/blob/5bf6aeb28933c3e7649d2d358c34df586d1cedce/screenshots/lady.PNG))

![Model 2]([screenshots/model-2.png](https://github.com/rahuljetwani/OPENGL-RENDERER-V1/blob/5bf6aeb28933c3e7649d2d358c34df586d1cedce/screenshots/modelEx1.PNG))

### Framebuffer Effects

![Black & White]([screenshots/black-white.png](https://github.com/rahuljetwani/OPENGL-RENDERER-V1/blob/5bf6aeb28933c3e7649d2d358c34df586d1cedce/screenshots/blacknwhite.PNG))

![Inverse]([screenshots/inverse.png](https://github.com/rahuljetwani/OPENGL-RENDERER-V1/blob/5bf6aeb28933c3e7649d2d358c34df586d1cedce/screenshots/inverse%20color.PNG))

![Edge Highlighting]([screenshots/stencil.png](https://github.com/rahuljetwani/OPENGL-RENDERER-V1/blob/5bf6aeb28933c3e7649d2d358c34df586d1cedce/screenshots/framebuffer.PNG))

### Cubemap / Skybox

![Skybox]([screenshots/skybox.png](https://github.com/rahuljetwani/OPENGL-RENDERER-V1/blob/5bf6aeb28933c3e7649d2d358c34df586d1cedce/screenshots/cubemap.PNG))

> Screenshot filenames are placeholders and should be changed to match the actual files in the repository.

## What's Next

I’ll keep extending the renderer as I learn more about real-time rendering.

Some of the areas I want to explore next:

* PBR
* Deferred shading
* SSAO
* Better lighting and shadows
* Bloom and HDR
* Normal mapping
* Image-based lighting
* Better render-pass architecture
* Rendering and GPU performance optimization
* Improved resource and material management
* Assimp / a more complete asset pipeline

## Learning Resources

Most of the concepts in this project come from:

* [LearnOpenGL](https://learnopengl.com/)
* Victor Gordan's OpenGL tutorials
* Mike Shah's computer graphics / graphics programming material
* OpenGL documentation

The code is adapted and implemented while learning, so parts of the renderer will change as my understanding improves.

## Why This Exists

I wanted to get an early and practical understanding of **how 3D graphics actually work underneath the tools I normally use**.

Building the renderer has helped me work through concepts such as buffers, shaders, coordinate spaces, cameras, lighting, textures, framebuffers, model data and the GPU rendering pipeline.

It is intentionally an ongoing project, and I expect parts of it to be rewritten or replaced as I learn better approaches.

> **Learn it → build it → understand it → improve it.**

**🚧 Ongoing learning project**

### Author

**Rahul Jetwani**
Computer Science Student · Game Developer & Real-Time Graphics Programmer

Interested in **graphics programming, real-time rendering, real-time 3D, and engine development**.
