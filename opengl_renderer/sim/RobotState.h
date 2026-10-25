#ifndef ROBOT_STATE_H
#define ROBOT_STATE_H

// Joint angles in RADIANS (see RobotConfig.h for axes and signs). Zero = arm pointing straight up +Z.
struct RobotState
{
	float base     = 0.0f;	// about Z
	float shoulder = 0.0f;	// about X
	float elbow    = 0.0f;	// about X
};

#endif
