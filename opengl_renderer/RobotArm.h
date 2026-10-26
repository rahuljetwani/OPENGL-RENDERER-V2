#ifndef ROBOT_ARM_H
#define ROBOT_ARM_H

#include "model.h"
#include "sim/RobotConfig.h"
#include "sim/RobotState.h"
#include "sim/RobotKinematics.h"

// Draws the robot. The sim folder does the maths, this class only owns the models.
//
//   arm.state.shoulder = 0.4f;   // change joints any time
//   arm.Update();                // once per frame
class RobotArm
{
public:
	RobotConfig config;			// before the models, their constructors read the paths
	RobotState  state;

	explicit RobotArm(const RobotConfig& cfg = RobotConfig());
	RobotArm(const RobotArm&) = delete;
	RobotArm& operator=(const RobotArm&) = delete;

	bool IsLoaded() const;

	// clamp joints, run FK, move the models
	void Update();

	// call after changing the material values in config
	void ApplyMaterials();

	void Draw(Shader& shader, Camera& camera);
	void DrawDepth(Shader& shader);

	// where the tip is in the renderer world (valid after Update)
	glm::vec3 TipWorld() const;

private:
	model floorModel;
	model baseModel;
	model forearmModel;
	model armModel;

	RobotPose pose;
	glm::mat4 rootFix = glm::mat4(1.0f);
};

#endif
