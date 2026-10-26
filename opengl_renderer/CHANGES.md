# What changed

**Code cleanup**
- model, mesh, RobotArm, UI, Profiler, HDRPipeline: shorter code, short comments, unused code removed
- model: removed joint helpers (SetJointAngle, ResetPose, PrintHierarchy, names), kept loading, culling, bounds, FitToUnitSize
- mesh: removed unused shininess / specularStrength
- RobotArm: removed shadow sphere and PrintLinkBounds, added TipWorld() for the pick-and-place work
- main: removed the light cube (light.* shaders), the scene sets its own shadow area
- shadowmap.frag: junk comment removed

**Bug fix**
- Environment: skybox face list used posx.jpg twice and never posz.jpg

**New**
- Primitives (box, plane, sphere, cylinder) and Scene (ground, two conveyors, crates, safety lines, material spheres)
- Environment loads hdr/sample_sky.hdr (falls back to the jpg skybox)
- UI: one CONTROLS panel, with a checkbox for the material spheres
- Scene::AddModel(file, height, x, z, yaw) to drop in any model

**Files you can delete**
light.vert, light.frag, RenderSettings.h, post_blur.frag, post_bright.frag (until you do bloom), ibl_cubecopy.frag, README_UPDATE.txt, a.jpg, .vs/, x64/
