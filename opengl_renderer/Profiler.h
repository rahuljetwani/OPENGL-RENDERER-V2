#ifndef PROFILER_H
#define PROFILER_H

#include <glad/glad.h>
#include <chrono>

// counters reset every frame (filled by mesh and model)
struct RenderStats
{
	unsigned int opaqueDraws = 0;
	unsigned int transparentDraws = 0;
	unsigned long long trisSubmitted = 0;	// all triangles we asked for
	unsigned long long trisVisible = 0;		// triangles actually drawn
};
extern RenderStats gStats;

enum CpuStage { CPU_UPDATE, CPU_CULLING, CPU_SUBMISSION, CPU_STAGE_COUNT };
enum GpuStage { GPU_SHADOW, GPU_FORWARD, GPU_POST, GPU_STAGE_COUNT };

// CPU times use std::chrono, GPU times use OpenGL timer queries (read one frame late).
// Values are smoothed so the UI is readable.
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
	RenderStats stats;				// last finished frame

	double cpuAccum[CPU_STAGE_COUNT] = {};

private:
	std::chrono::steady_clock::time_point lastFrame;
	bool firstFrame = true;

	GLuint queries[GPU_STAGE_COUNT][2] = {};
	bool issued[GPU_STAGE_COUNT][2] = {};
	int parity = 0;
};
extern Profiler gProfiler;

// adds the time of a scope to a CPU stage
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
