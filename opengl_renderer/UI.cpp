#include "UI.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <cstdio>

static const float RAD2DEG = 57.29577951f;

// 1200000 -> "1.2 M"
static void formatCount(char* out, size_t size, unsigned long long v)
{
	if (v >= 1000000ULL)     std::snprintf(out, size, "%.1f M", v / 1.0e6);
	else if (v >= 10000ULL)  std::snprintf(out, size, "%.1f K", v / 1.0e3);
	else                     std::snprintf(out, size, "%llu", v);
}

static void msRow(const char* label, float ms)
{
	ImGui::Text("%-12s %6.2f ms", label, ms);
}

static void countRow(const char* label, unsigned long long v)
{
	char text[32];
	formatCount(text, sizeof(text), v);
	ImGui::Text("%-12s %9s", label, text);
}

void UI::Init(GLFWwindow* window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO().IniFilename = nullptr;		// no imgui.ini file
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;	// the camera controls the cursor
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
}

void UI::Shutdown()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void UI::NewFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void UI::Render()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool UI::WantsMouse()
{
	return ImGui::GetIO().WantCaptureMouse;
}

void UI::StatsPanel(const Profiler& p)
{
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::Begin("RENDERER", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Text("%-12s %6.0f", "FPS", p.fps);
	ImGui::Text("%-12s %6.1f ms", "Frame Time", p.frameMs);

	ImGui::SeparatorText("CPU");
	msRow("Update", p.cpuMs[CPU_UPDATE]);
	msRow("Culling", p.cpuMs[CPU_CULLING]);
	msRow("Submission", p.cpuMs[CPU_SUBMISSION]);

	ImGui::SeparatorText("GPU");
	msRow("Shadow", p.gpuMs[GPU_SHADOW]);
	msRow("Forward PBR", p.gpuMs[GPU_FORWARD]);
	msRow("Post FX", p.gpuMs[GPU_POST]);

	ImGui::SeparatorText("DRAW CALLS");
	countRow("Opaque", p.stats.opaqueDraws);
	countRow("Transparent", p.stats.transparentDraws);
	countRow("Total", p.stats.opaqueDraws + p.stats.transparentDraws);

	ImGui::SeparatorText("TRIANGLES");
	countRow("Visible", p.stats.trisVisible);
	countRow("Submitted", p.stats.trisSubmitted);

	ImGui::End();
}

void UI::ControlPanel(RobotArm& arm, Scene& scene, float& exposureEV, float& sunIntensity)
{
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 10.0f, 10.0f), ImGuiCond_FirstUseEver, ImVec2(1.0f, 0.0f));
	ImGui::SetNextWindowSize(ImVec2(340, 0), ImGuiCond_FirstUseEver);
	ImGui::Begin("CONTROLS");

	const RobotConfig& cfg = arm.config;
	RobotState& state = arm.state;

	// angles are stored in radians, the slider shows degrees
	ImGui::SeparatorText("JOINT ANGLES");
	ImGui::SliderAngle("Base", &state.base, cfg.baseLimit.min * RAD2DEG, cfg.baseLimit.max * RAD2DEG);
	ImGui::SliderAngle("Shoulder", &state.shoulder, cfg.shoulderLimit.min * RAD2DEG, cfg.shoulderLimit.max * RAD2DEG);
	ImGui::SliderAngle("Elbow", &state.elbow, cfg.elbowLimit.min * RAD2DEG, cfg.elbowLimit.max * RAD2DEG);

	ImGui::SeparatorText("LIGHTING");
	ImGui::SliderFloat("Exposure (EV)", &exposureEV, -4.0f, 4.0f);
	ImGui::SliderFloat("Sun intensity", &sunIntensity, 0.0f, 20.0f);

	ImGui::SeparatorText("ROBOT MATERIAL");
	bool changed = false;
	changed |= ImGui::SliderFloat("Metallic", &arm.config.linkMetallic, 0.0f, 1.0f);
	changed |= ImGui::SliderFloat("Roughness", &arm.config.linkRoughness, 0.0f, 1.0f);
	if (changed) arm.ApplyMaterials();

	ImGui::SeparatorText("SCENE");
	ImGui::Checkbox("Material spheres", &scene.showShowcase);

	ImGui::End();
}
