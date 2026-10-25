#ifndef ROBOT_KINEMATICS_H
#define ROBOT_KINEMATICS_H

#include <glm/glm.hpp>
#include "RobotConfig.h"
#include "RobotState.h"

// World matrices (sim frame: Blender units, Z-up) of every link, each placing that link's mesh from
// its own pivot-at-origin local space into the world. Plus the world position of the arm tip.
struct RobotPose
{
	glm::mat4 floor   = glm::mat4(1.0f);	// fixed
	glm::mat4 base    = glm::mat4(1.0f);	// base joint
	glm::mat4 forearm = glm::mat4(1.0f);	// base * shoulder joint
	glm::mat4 arm     = glm::mat4(1.0f);	// forearm * elbow joint
	glm::vec3 tip     = glm::vec3(0.0f);	// end of the arm, world (sim) frame
};

// Forward kinematics.  No OpenGL, no model headers.
//   base    = T(baseOffset)     * Rz(state.base)
//   forearm = base    * T(shoulderOffset) * Rx(state.shoulder)
//   arm     = forearm * T(elbowOffset)    * Rx(state.elbow)
//   tip     = arm * tipOffset
RobotPose ForwardKinematics(const RobotConfig& cfg, const RobotState& state);

// Returns `state` with every angle clamped to the limits in `cfg`.
RobotState ClampToLimits(const RobotConfig& cfg, const RobotState& state);

#endif
