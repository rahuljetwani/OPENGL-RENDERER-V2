#ifndef ROBOT_CONFIG_H
#define ROBOT_CONFIG_H

#include <string>
#include <glm/glm.hpp>

// Robot sim rules (the sim uses Blender's frame):
//  - Z is up, angles are in radians, units are Blender units
//  - base turns around Z, shoulder and elbow turn around X
//  - a positive shoulder / elbow leans the arm toward -Y
//  - chain: floor (fixed) -> base -> forearm -> arm
//  - each offset is "parent pivot -> this pivot" at zero pose
//  - the only Z-up to Y-up conversion is rootFix in RobotArm.cpp

struct JointLimit
{
	float min = -3.14159265f;
	float max =  3.14159265f;
};

// plain data only, no OpenGL here
struct RobotConfig
{
	// geometry
	glm::vec3 floorOffset    = glm::vec3(0.0f, 0.0f, 0.38176f);
	glm::vec3 baseOffset     = glm::vec3(0.0f, 0.0f, 0.787f);
	glm::vec3 shoulderOffset = glm::vec3(0.0f, 0.0f, 1.713f);
	glm::vec3 elbowOffset    = glm::vec3(0.0f, 0.0f, 5.009f);
	glm::vec3 tipOffset      = glm::vec3(0.0f, 0.0f, 5.0f);

	// joint limits (placeholders, set the real ones later)
	JointLimit baseLimit     { -3.14159265f,  3.14159265f };
	JointLimit shoulderLimit { -1.57079633f,  1.57079633f };
	JointLimit elbowLimit    { -2.35619449f,  2.35619449f };

	// model files
	std::string floorPath   = "models/floor.fbx";
	std::string basePath    = "models/base.fbx";
	std::string forearmPath = "models/forearm.fbx";
	std::string armPath     = "models/arm.fbx";

	// sim units -> renderer units
	float worldScale = 0.1f;

	// surface look (fbx files have no metal / roughness)
	float linkMetallic   = 0.35f;
	float linkRoughness  = 0.38f;
	float floorMetallic  = 0.0f;
	float floorRoughness = 0.85f;
};

#endif
