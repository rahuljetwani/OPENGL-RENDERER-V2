# OPENGL-RENDERER-V2



A real-time 3D renderer I'm building from scratch in C++ and OpenGL. This is the follow-up to my [first renderer](https://github.com/rahuljetwani/OPENGL-RENDERER-V1), which covered the basics: textures, lighting and model loading.

V2 adds **physically based rendering (PBR)** and a **robotic arm**. I'm also starting to use it as a testbed for **digital twin** ideas and basic simulations.

> 🚧 **Work in progress.** This is under active development, so things will change and some parts are rough or unfinished.

## Screenshots

![Screenshot 1](opengl_renderer/screenshots/OpenGL%20Digital%20Twin%2026-10-2026%2014_31_56.png)
![Screenshot 2](opengl_renderer/screenshots/OpenGL%20Digital%20Twin%2026-10-2026%2014_32_47.png)
![Screenshot 3](opengl_renderer/screenshots/OpenGL%20Digital%20Twin%2026-10-2026%2014_34_17.png)


## What's new in V2

- **PBR shading:** materials use a metallic/roughness workflow, so surfaces respond to light much more realistically than the old Phong-style lighting in V1.
- **Robotic arm:** a jointed robot arm in the scene, driven by joint transforms.
- **Digital twin and simulation experiments:** early work on using the renderer as the visual side of a simulation, where the virtual arm can mirror the state of a simulated machine.

## Built with

- C++
- OpenGL, loaded through GLAD
- GLFW for windowing and input
- GLM for math
- stb_image for textures
- (https://github.com/nlohmann/json)](https://github.com/nlohmann/json) for cpp based json
- IMGUI for UI
- assimp for model loading 



## Getting started

**You'll need:** Windows, Visual Studio, and a GPU that supports OpenGL 3.3 or newer.

1. Clone the repo:
   ```bash
   git clone https://github.com/rahuljetwani/OPENGL-RENDERER-V2.git
   ```
2. Open the project inside the `opengl_renderer` folder in Visual Studio.
3. Build and run (x64 recommended).

## Controls


| `W` `A` `S` `D` | Move the camera |
| Mouse | Look around |
| `crtl` | down |
| `space` | up |


<!-- Replace with your real controls. -->

## Roadmap

- [ ] Improve the PBR pipeline (image-based lighting, shadows)
- [ ] Inverse kinematics for the robot arm
- [ ] Feed live or simulated data into the arm (digital twin)
- [ ] Basic physics and collision simulation
- [ ] Cleaner scene and code structure

## Version history

- **[V1](https://github.com/rahuljetwani/OPENGL-RENDERER-V1):** textures, lighting, model loading
- **V2 (this repo):** PBR, robot arm, early simulation work

## Acknowledgements

Learned a lot from [LearnOpenGL](https://learnopengl.com/).

## Author

[Rahul Jetwani](https://github.com/rahuljetwani)
