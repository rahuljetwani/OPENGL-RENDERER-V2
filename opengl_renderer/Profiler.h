#ifndef PROFILER_H
#define PROFILER_H

#include <glad/glad.h>
#include <chrono>

// Counters that are reset every frame (filled in by mesh / model)
struct RenderStats
{
	unsigned int opaqueDraws = 0;
	unsigned int transparentDraws = 0;
	unsigned long long trisSubmitted = 0;	// triangles of every mesh we asked to draw
	unsigned long long trisVisible = 0;		// triangles that survived frustum culling and were drawn
};
extern RenderStats gStats;

enum CpuStage { CPU_UPDATE, CPU_CULLING, CPU_SUBMISSION, CPU_STAGE_COUNT };
enum GpuStage { GPU_SHADOW, GPU_FORWARD, GPU_POST, GPU_STAGE_COUNT };

// CPU timers use std::chrono, GPU timers use OpenGL timer queries (read back one frame late).
// Numbers are smoothed a little so they are readable in the UI.
class Profiler
{
public:
	void BeginFrame();
	void EndFrame();

	void GpuBegin(GpuStage s);
	void GpuEnd();

	float fps = 0.0f;
	float frameMs = 0.0f;
	float cpuMs[CPU_STAGE_COUNT] = {};
	float gpuMs[GPU_STAGE_COUNT] = {};
	RenderStats stats;				// counters of the last finished frame

	double cpuAccum[CPU_STAGE_COUNT] = {};

private:
	std::chrono::steady_clock::time_point lastFrame;
	bool firstFrame = true;

	GLuint queries[GPU_STAGE_COUNT][2] = {};
	bool issued[GPU_STAGE_COUNT][2] = {};
	int parity = 0;
};
extern Profiler gProfiler;

// adds the time spent in a scope to a CPU stage
struct CpuScope
{
	CpuStage stage;
	std::chrono::steady_clock::time_point start;
	CpuScope(CpuStage s) : stage(s), start(std::chrono::steady_clock::now()) {}
	~CpuScope()
	{
		gProfiler.cpuAccum[stage] += std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
	}
};

#endif
