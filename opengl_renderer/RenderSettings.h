#ifndef RENDER_SETTINGS_H
#define RENDER_SETTINGS_H

// Everything the ImGui "Rendering" panel tweaks. Plain data, no GL.
enum ToneMapper { TONEMAP_ACES = 0, TONEMAP_REINHARD = 1, TONEMAP_HABLE = 2, TONEMAP_CLAMP = 3 };

struct RenderSettings
{
	// --- HDR / post ---
	float exposureEV = 0.0f;			// exposure in stops: multiplier = 2^EV
	int   tonemap = TONEMAP_ACES;
	bool  bloom = true;
	float bloomThreshold = 1.0f;		// HDR luminance above this starts to glow
	float bloomIntensity = 0.25f;

	// --- lighting ---
	float sunIntensity = 4.0f;			// radiance of the directional light (HDR, can be > 1)
	float iblIntensity = 1.0f;			// ambient light from the environment
	bool  showSky = true;
	float skyIntensity = 1.0f;
	float skyBlur = 0.0f;				// mip level of the backdrop

	// --- misc ---
	bool vsync = true;
	bool drawLightCube = true;
};

#endif
