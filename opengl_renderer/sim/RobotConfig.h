#ifndef ROBOT_CONFIG_H
#define ROBOT_CONFIG_H

#include <string>
#include <glm/glm.hpp>

// =====================================================================================================
//  ROBOT SIM CONVENTIONS  -  every later bug traces back to these, so they live in one place.
// =====================================================================================================
//
//  FRAME      The sim works in Blender's frame: right-handed, Z-UP.  (X right, Y forward, Z up)
//             At zero pose the arm points straight up the +Z axis.
//
//  JOINTS     base      rotates about +Z   (yaw, spins everything above the floor)
//             shoulder  rotates about +X   (pitch, hinge sits at the bottom of forearm.fbx)
//             elbow     rotates about +X   (pitch, hinge sits at the bottom of arm.fbx)
//
//  SIGNS      Right-hand rule about each axis. Positive shoulder/elbow tips the arm toward -Y
//             (Rx(+a) takes +Z to (0, -sin a, cos a)). Positive base turns +X toward +Y (counter-clockwise from above).
//
//  ANGLES     Radians. Always.
//
//  UNITS      Blender units. The FBX files carry a x100 scale and a 90deg Z rotation in their node transforms;
//             model.cpp ignores those (ignoreNodeTransforms), so mesh vertices are used as raw Blender-unit data.
//             The ONLY place the sim is converted for rendering is RobotArm::rootFix (Z-up -> Y-up + worldScale).
//
//  LINK NAMES Chain order is   floor (fixed) -> base -> forearm -> arm
//             base      = base.fbx,     pivot on the floor, driven by the BASE joint
//             forearm   = forearm.fbx,  pivot at the SHOULDER hinge
//             arm       = arm.fbx,      pivot at the ELBOW hinge, its tip is 5.0 above the pivot
//
//  OFFSETS    Each offset is "parent pivot -> this pivot", expressed in the parent's frame at zero pose.
//             They were read from the FBX node translations (/100):
//                 base   pivot z = 0.787   (world origin = floor bottom)
//                 fore   pivot z = 2.500   -> 2.500 - 0.787 = 1.713 above base
//                 arm    pivot z = 7.509   -> 7.509 - 2.500 = 5.009 above shoulder
//             Total height at zero pose = 0.787 + 1.713 + 5.009 + 5.0 = 12.509
// =====================================================================================================

struct JointLimit
{
	float min = -3.14159265f;
	float max =  3.14159265f;
};

// Plain data only: no OpenGL, no model headers. A JSON loader can fill this same struct later.
struct RobotConfig
{
	// ---- geometry (Blender units, Z-up) -------------------------------------------------------
	glm::vec3 floorOffset    = glm::vec3(0.0f, 0.0f, 0.38176f);	// floor.fbx pivot is its centre; floor spans z = 0 .. 0.7635
	glm::vec3 baseOffset     = glm::vec3(0.0f, 0.0f, 0.787f);	// world origin   -> base pivot
	glm::vec3 shoulderOffset = glm::vec3(0.0f, 0.0f, 1.713f);	// base pivot     -> shoulder hinge
	glm::vec3 elbowOffset    = glm::vec3(0.0f, 0.0f, 5.009f);	// shoulder hinge -> elbow hinge
	glm::vec3 tipOffset      = glm::vec3(0.0f, 0.0f, 5.0f);		// elbow hinge    -> end of the arm

	// ---- joint limits (radians). PLACEHOLDERS: set these to the real hardware limits ----------
	JointLimit baseLimit     { -3.14159265f,  3.14159265f };	// +-180 deg
	JointLimit shoulderLimit { -1.57079633f,  1.57079633f };	// +-90 deg  (arm horizontal at the extremes)
	JointLimit elbowLimit    { -2.35619449f,  2.35619449f };	// +-135 deg

	// ---- assets (relative to the working directory, like the shaders) -------------------------
	std::string floorPath   = "models/floor.fbx";
	std::string basePath    = "models/base.fbx";
	std::string forearmPath = "models/forearm.fbx";
	std::string armPath     = "models/arm.fbx";

	// ---- rendering ---------------------------------------------------------------------------
	float worldScale = 0.1f;	// sim units -> renderer units. 12.5-unit arm * 0.1 = 1.25 tall in the scene

	// PBR surface of the links (the FBX files carry no metal/roughness data). Tweakable live from the ImGui panel.
	float linkMetallic  = 0.35f;	// base, forearm, arm
	float linkRoughness = 0.38f;
	float floorMetallic  = 0.0f;
	float floorRoughness = 0.85f;
};

#endif
