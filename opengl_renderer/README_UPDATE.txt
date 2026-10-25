PBR + HDR + ImGui update  -  merge into your existing project folder (the one with mesh.h, Camera.h, models/ ...)
=========================================================================================================

1. Close Visual Studio. Back up your project folder.
2. Copy everything in this zip's opengl_renderer/ folder INTO your project folder, replacing files when asked.
   (Keep the sub-folders: sim/  imgui/  hdr/  tools/)
3. Open the .sln and Build -> Rebuild Solution. Run from VS (F5); working directory = project folder.

NEW FILES
  Profiler.h/.cpp  Environment.h/.cpp  HDRPipeline.h/.cpp  UI.h/.cpp  RenderSettings.h
  imgui/                       Dear ImGui 1.91.9 (core + GLFW/OpenGL3 backends, demo window disabled)
  ibl_*.vert/.frag             image based lighting shaders      post_bright.frag  post_blur.frag
  hdr/sample_sky.hdr           procedural HDR sky; copy it to hdr/environment.hdr to switch the lighting to true HDR

CHANGED FILES
  main.cpp  model.h/.cpp  mesh.h/.cpp  Camera.cpp (aspect-ratio fix)  RobotArm.h/.cpp  sim/RobotConfig.h
  default.frag (now PBR)  skybox.frag  framebuffer.frag (now HDR composite)  opengl_renderer.vcxproj

CONTROLS
  Sliders in the ROBOT ARM panel, or  Q/E base  R/F shoulder  T/G elbow  |  M demo  |  0 zero pose  |  Esc quit
  WASD / Space / Ctrl / Shift + hold left mouse = camera (as before; ignored while the pointer is over a panel)

TEST SWITCHES (optional)  --shot out.ppm --frames 10   --pose 40 50 70   --demo   --cam px py pz dx dy dz
