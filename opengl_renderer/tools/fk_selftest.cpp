// Standalone FK check (no OpenGL, no Assimp):
//   g++ -std=c++17 -ILibraries/include -Isim tools/fk_selftest.cpp sim/RobotKinematics.cpp -o fk_selftest && ./fk_selftest
#include "RobotKinematics.h"
#include <cstdio>
#include <cmath>

static int failures = 0;
static void expectTip(const char* name, const RobotState& s, glm::vec3 want)
{
	RobotConfig cfg;
	RobotPose p = ForwardKinematics(cfg, s);
	bool ok = glm::length(p.tip - want) < 1e-3f;
	std::printf("%-34s tip (%7.3f %7.3f %7.3f)  want (%7.3f %7.3f %7.3f)  %s\n", name,
		p.tip.x, p.tip.y, p.tip.z, want.x, want.y, want.z, ok ? "ok" : "FAIL");
	if (!ok) failures++;
}

int main()
{
	const float H = 3.14159265f / 2.0f;
	expectTip("zero pose: straight up",           {0, 0, 0},   {0, 0, 12.509f});
	expectTip("shoulder +90: leans toward -Y",    {0, H, 0},   {0, -10.009f, 2.5f});
	expectTip("shoulder +90, elbow +90: folds",   {0, H, H},   {0, -5.009f, -2.5f});
	expectTip("shoulder 0, elbow +90: forearm up",{0, 0, H},   {0, -5.0f, 7.509f});
	expectTip("base +90, shoulder +90: toward +X",{H, H, 0},   {10.009f, 0, 2.5f});
	expectTip("base only: tip stays on the axis", {1.2f, 0, 0},{0, 0, 12.509f});

	RobotConfig cfg; RobotState wild{9.0f, -9.0f, 9.0f};
	RobotState c = ClampToLimits(cfg, wild);
	bool ok = c.base == cfg.baseLimit.max && c.shoulder == cfg.shoulderLimit.min && c.elbow == cfg.elbowLimit.max;
	std::printf("%-34s %s\n", "limits clamp", ok ? "ok" : "FAIL"); if (!ok) failures++;

	// chain check: moving the shoulder moves arm but not base; moving the elbow moves only arm
	RobotPose a = ForwardKinematics(cfg, {0, 0, 0});
	RobotPose b = ForwardKinematics(cfg, {0, 0.5f, 0});
	RobotPose d = ForwardKinematics(cfg, {0, 0, 0.5f});
	bool chain = a.base == b.base && a.forearm != b.forearm && a.arm != b.arm
	          && a.forearm == d.forearm && a.arm != d.arm;
	std::printf("%-34s %s\n", "chain: joint moves itself + children", chain ? "ok" : "FAIL"); if (!chain) failures++;

	std::printf(failures ? "\n%d FAILED\n" : "\nall ok\n", failures);
	return failures;
}
