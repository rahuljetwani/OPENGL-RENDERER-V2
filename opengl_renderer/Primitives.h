#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include "mesh.h"

// Simple shapes made in code, so the scene needs no extra model files.
// All faces are counter-clockwise seen from outside (back face culling is on).
namespace Primitives
{
	// box centred on the origin
	mesh Box(const glm::vec3& size, const Material& material);

	// flat square on y = 0 facing up, centred on the origin
	mesh Plane(float size, const Material& material);

	// sphere centred on the origin
	mesh Sphere(float radius, int segments, const Material& material);

	// cylinder centred on the origin, its axis is Y
	mesh Cylinder(float radius, float height, int segments, const Material& material);
}

#endif
