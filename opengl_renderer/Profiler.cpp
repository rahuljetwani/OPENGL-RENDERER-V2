#include "Profiler.h"

RenderStats gStats;
Profiler gProfiler;

static float smooth(float old, float value)
{
	return old == 0.0f ? value : old + (value - old) * 0.08f;
}

void Profiler::BeginFrame()
{
	auto now = std::chrono::steady_clock::now();
	if (!firstFrame)
	{
		frameMs = smooth(frameMs, std::chrono::duration<float, std::milli>(now - lastFrame).count());
		fps = 1000.0f / frameMs;
	}
	lastFrame = now;
	firstFrame = false;

	for (int i = 0; i < CPU_STAGE_COUNT; i++) cpuAccum[i] = 0.0;
	gStats = RenderStats();

	if (queries[0][0] == 0)
		for (int s = 0; s < GPU_STAGE_COUNT; s++)
			glGenQueries(2, queries[s]);

	// this frame writes queries[s][parity], the previous frame's results are in the other slot
	parity ^= 1;
	int prev = parity ^ 1;
	for (int s = 0; s < GPU_STAGE_COUNT; s++)
	{
		if (!issued[s][prev]) continue;
		GLint ready = 0;
		glGetQueryObjectiv(queries[s][prev], GL_QUERY_RESULT_AVAILABLE, &ready);
		if (!ready) continue;
		GLuint64 ns = 0;
		glGetQueryObjectui64v(queries[s][prev], GL_QUERY_RESULT, &ns);
		gpuMs[s] = smooth(gpuMs[s], (float)(ns / 1.0e6));
		issued[s][prev] = false;
	}
}

void Profiler::EndFrame()
{
	for (int i = 0; i < CPU_STAGE_COUNT; i++)
		cpuMs[i] = smooth(cpuMs[i], (float)cpuAccum[i]);
	stats = gStats;
}

void Profiler::GpuBegin(GpuStage s)
{
	glBeginQuery(GL_TIME_ELAPSED, queries[s][parity]);	// only one timer query can be active at a time
	issued[s][parity] = true;
}

void Profiler::GpuEnd()
{
	glEndQuery(GL_TIME_ELAPSED);
}
