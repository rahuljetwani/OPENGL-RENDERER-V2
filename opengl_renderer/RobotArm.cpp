#include "RobotArm.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <algorithm>

RobotArm::RobotArm(const RobotConfig& cfg)
	: config(cfg),
	  floorModel(cfg.floorPath, true),		// true = ignoreNodeTransforms: raw mesh at its pivot
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

// The ONE place the sim frame is converted for rendering:
//   Z-up -> Y-up  (-90 deg about X: sim +Z becomes +Y, sim +Y becomes -Z, so "forward" points into the screen)
//   then worldScale (sim units -> renderer units)
glm::mat4 RobotArm::computeRootFix() const
{
	return glm::scale(glm::mat4(1.0f), glm::vec3(config.worldScale))
		* glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
}

void RobotArm::Update()
{
	state   = ClampToLimits(config, state);
	pose    = ForwardKinematics(config, state);
	rootFix = computeRootFix();

	floorModel.transform   = rootFix * pose.floor;
	baseModel.transform    = rootFix * pose.base;
	forearmModel.transform = rootFix * pose.forearm;
	armModel.transform     = rootFix * pose.arm;
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

void RobotArm::GetShadowSphere(glm::vec3& centre, float& radius) const
{
	// Every link hangs off the shoulder hinge, so nothing can get further from it than the two link
	// lengths plus a little for thickness. (+0.6 = forearm/arm half-thickness and some margin)
	glm::vec3 shoulder = config.baseOffset + config.shoulderOffset;
	float reach = glm::length(config.elbowOffset) + glm::length(config.tipOffset) + 0.6f;
	float floorReach = glm::length(glm::vec2(3.0f, 3.0f)) + glm::length(shoulder);	// floor corners
	glm::mat4 fix = computeRootFix();
	centre = glm::vec3(fix * glm::vec4(shoulder, 1.0f));
	radius = std::max(reach, floorReach) * config.worldScale;
}

void RobotArm::PrintLinkBounds()
{
	struct Item { const char* name; model* m; } items[] =
	{
		{ "floor",   &floorModel   },
		{ "base",    &baseModel    },
		{ "forearm", &forearmModel },
		{ "arm",     &armModel     },
	};
	for (const Item& it : items)
	{
		glm::vec3 mn, mx;
		it.m->GetBounds(mn, mx);
		std::cout << "[bounds] " << it.name
			<< "  x " << mn.x << " .. " << mx.x
			<< "  y " << mn.y << " .. " << mx.y
			<< "  z " << mn.z << " .. " << mx.z << "\n";
	}
}
