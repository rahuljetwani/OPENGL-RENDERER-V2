#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include "mesh.h"
#include "model.h"

// The factory around the robot: ground, two conveyors, crates, safety lines,
// and rows of spheres that show off the PBR materials.
// Everything is built from code (Primitives), plus any extra model files you add.
class Scene
{
public:
	Scene();

	void Draw(Shader& shader, Camera& camera);
	void DrawDepth(Shader& shader);

	// a sphere that holds everything that should cast a shadow
	void GetShadowSphere(glm::vec3& centre, float& radius) const;

	// loads a model file, scales it to `height`, and puts it at x, z on the ground
	// (call it from the constructor to add your own props)
	void AddModel(const std::string& file, float height, float x, float z, float yawDegrees);

	// scene options for the UI
	bool showShowcase = true;			// the sphere rows at the back

private:
	struct Prop
	{
		mesh shape;
		glm::mat4 transform;
		bool showcase;					// belongs to the sphere rows
	};

	std::vector<Prop> props;
	std::vector<model> models;

	void add(const mesh& shape, const glm::vec3& position, float yawDegrees = 0.0f, bool showcase = false);
	void buildGround();
	void buildConveyor(float x, float zStart, float zEnd);
	void buildCrates();
	void buildSafetyLines();
	void buildShowcase();
};

#endif
