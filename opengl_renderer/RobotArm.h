#ifndef ROBOT_ARM_H
#define ROBOT_ARM_H

#include "model.h"
#include "sim/RobotConfig.h"
#include "sim/RobotState.h"
#include "sim/RobotKinematics.h"

// The DRAW side of the digital twin. The sim (sim/) knows nothing about OpenGL; this class owns the four
// models and glues the sim's matrices onto them.
//
//   arm.state.shoulder = 0.4f;   // change joints any time...
//   arm.Update();                // ...then once per frame: clamp -> FK -> model.transform
//   arm.DrawDepth(...) / arm.Draw(...)
class RobotArm
{
public:
	RobotConfig config;		// declared before the models: their constructors read the paths from it
	RobotState  state;

	explicit RobotArm(const RobotConfig& cfg = RobotConfig());
	RobotArm(const RobotArm&) = delete;				// owns GPU buffers through its models
	RobotArm& operator=(const RobotArm&) = delete;

	bool IsLoaded() const;

	// clamp state to limits, run FK, and set every model's transform = rootFix * linkMatrix
	void Update();

	// push config.linkMetallic / linkRoughness / floor* onto the models (call after changing them)
	void ApplyMaterials();

	void Draw(Shader& shader, Camera& camera);		// colour pass
	void DrawDepth(Shader& shader);					// shadow pass

	// A sphere that contains every pose the arm can reach (renderer world). Constant for a given config,
	// which is what ShadowMap::UpdateDirectional wants.
	void GetShadowSphere(glm::vec3& centre, float& radius) const;

	// Prints each link's bounds. Call it BEFORE the first Update() to see raw link data (checkpoint:
	// forearm should print x +-0.5, y +-0.5, z -0.5 .. 5.52).
	void PrintLinkBounds();

private:
	model floorModel;
	model baseModel;
	model forearmModel;
	model armModel;

	RobotPose pose;
	glm::mat4 rootFix = glm::mat4(1.0f);

	glm::mat4 computeRootFix() const;
};

#endif
