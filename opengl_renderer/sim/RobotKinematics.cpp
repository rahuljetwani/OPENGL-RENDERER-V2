#include "RobotKinematics.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

namespace
{
	const glm::vec3 AXIS_X(1.0f, 0.0f, 0.0f);
	const glm::vec3 AXIS_Z(0.0f, 0.0f, 1.0f);

	// "move to the hinge, then rotate about it"
	glm::mat4 hinge(const glm::vec3& offset, const glm::vec3& axis, float radians)
	{
		return glm::translate(glm::mat4(1.0f), offset) * glm::rotate(glm::mat4(1.0f), radians, axis);
	}

	float clampTo(float v, const JointLimit& l) { return std::min(std::max(v, l.min), l.max); }
}

RobotPose ForwardKinematics(const RobotConfig& cfg, const RobotState& state)
{
	RobotPose pose;
	pose.floor   = glm::translate(glm::mat4(1.0f), cfg.floorOffset);
	pose.base    = hinge(cfg.baseOffset, AXIS_Z, state.base);
	pose.forearm = pose.base    * hinge(cfg.shoulderOffset, AXIS_X, state.shoulder);
	pose.arm     = pose.forearm * hinge(cfg.elbowOffset,    AXIS_X, state.elbow);
	pose.tip     = glm::vec3(pose.arm * glm::vec4(cfg.tipOffset, 1.0f));
	return pose;
}

RobotState ClampToLimits(const RobotConfig& cfg, const RobotState& state)
{
	RobotState s;
	s.base     = clampTo(state.base,     cfg.baseLimit);
	s.shoulder = clampTo(state.shoulder, cfg.shoulderLimit);
	s.elbow    = clampTo(state.elbow,    cfg.elbowLimit);
	return s;
}
