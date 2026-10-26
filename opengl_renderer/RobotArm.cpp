#include "RobotArm.h"
#include <glm/gtc/matrix_transform.hpp>

RobotArm::RobotArm(const RobotConfig& cfg)
	: config(cfg),
	  floorModel(cfg.floorPath, true),
	  baseModel(cfg.basePath, true),
	  forearmModel(cfg.forearmPath, true),
	  armModel(cfg.armPath, true)
{
	ApplyMaterials();
}

void RobotArm::ApplyMaterials()
{
	floorModel.SetPBR(config.floorMetallic, config.floorRoughness);
	baseModel.SetPBR(config.linkMetallic, config.linkRoughness);
	forearmModel.SetPBR(config.linkMetallic, config.linkRoughness);
	armModel.SetPBR(config.linkMetallic, config.linkRoughness);
}

bool RobotArm::IsLoaded() const
{
	return floorModel.IsLoaded() && baseModel.IsLoaded() && forearmModel.IsLoaded() && armModel.IsLoaded();
}

void RobotArm::Update()
{
	state = ClampToLimits(config, state);
	pose = ForwardKinematics(config, state);

	// Z-up to Y-up, then scale down to renderer units
	rootFix = glm::scale(glm::mat4(1.0f), glm::vec3(config.worldScale))
		* glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	floorModel.transform   = rootFix * pose.floor;
	baseModel.transform    = rootFix * pose.base;
	forearmModel.transform = rootFix * pose.forearm;
	armModel.transform     = rootFix * pose.arm;
}

glm::vec3 RobotArm::TipWorld() const
{
	return glm::vec3(rootFix * glm::vec4(pose.tip, 1.0f));
}

void RobotArm::Draw(Shader& shader, Camera& camera)
{
	floorModel.Draw(shader, camera);
	baseModel.Draw(shader, camera);
	forearmModel.Draw(shader, camera);
	armModel.Draw(shader, camera);
}

void RobotArm::DrawDepth(Shader& shader)
{
	floorModel.DrawDepth(shader);
	baseModel.DrawDepth(shader);
	forearmModel.DrawDepth(shader);
	armModel.DrawDepth(shader);
}
