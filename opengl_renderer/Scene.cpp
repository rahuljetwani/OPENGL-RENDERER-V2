#include "Scene.h"
#include "Primitives.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

// small helper to write materials in one line
static Material makeMaterial(float r, float g, float b, float metallic, float roughness)
{
	Material m;
	m.baseColor = glm::vec4(r, g, b, 1.0f);
	m.metallic = metallic;
	m.roughness = roughness;
	return m;
}

// Layout (renderer units, the robot stands at the origin and is 1.25 tall):
//   conveyor A at x = -0.75   (pick up here)
//   conveyor B at x = +0.75   (drop here)
//   sphere rows at the back (z = -1.8)
Scene::Scene()
{
	buildGround();
	buildConveyor(-0.75f, -0.9f, 0.9f);
	buildConveyor( 0.75f, -0.9f, 0.9f);
	buildCrates();
	buildSafetyLines();
	buildShowcase();

	// Add your own models here, for example:
	// AddModel("models/props/crate.glb", 0.25f, -0.75f, 0.0f, 0.0f);
}

void Scene::add(const mesh& shape, const glm::vec3& position, float yawDegrees, bool showcase)
{
	glm::mat4 t = glm::translate(glm::mat4(1.0f), position)
		* glm::rotate(glm::mat4(1.0f), glm::radians(yawDegrees), glm::vec3(0, 1, 0));
	props.push_back({ shape, t, showcase });
}

void Scene::AddModel(const std::string& file, float height, float x, float z, float yawDegrees)
{
	model m(file);
	if (!m.IsLoaded()) return;

	// FitToUnitSize uses the biggest side, so fit first, then correct for height
	m.FitToUnitSize(1.0f);
	glm::vec3 mn, mx;
	m.GetBounds(mn, mx);
	float s = height / (mx.y - mn.y);
	m.FitToUnitSize(s);

	m.transform = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0, z))
		* glm::rotate(glm::mat4(1.0f), glm::radians(yawDegrees), glm::vec3(0, 1, 0))
		* m.transform;
	models.push_back(m);
}

void Scene::buildGround()
{
	// big floor and a slightly lighter strip where the robot works
	add(Primitives::Plane(14.0f, makeMaterial(0.30f, 0.30f, 0.32f, 0.0f, 0.85f)), glm::vec3(0, 0, 0));
	add(Primitives::Box(glm::vec3(3.4f, 0.01f, 2.4f), makeMaterial(0.42f, 0.42f, 0.45f, 0.0f, 0.55f)), glm::vec3(0, 0.005f, 0));
}

void Scene::buildConveyor(float x, float zStart, float zEnd)
{
	float length = zEnd - zStart;
	float zMid = (zStart + zEnd) * 0.5f;
	float width = 0.34f;
	float beltTop = 0.20f;

	Material steel   = makeMaterial(0.75f, 0.76f, 0.78f, 1.0f, 0.30f);
	Material darkRub = makeMaterial(0.05f, 0.05f, 0.055f, 0.0f, 0.85f);
	Material paint   = makeMaterial(0.10f, 0.25f, 0.55f, 0.2f, 0.45f);

	// belt and the frame under it
	add(Primitives::Box(glm::vec3(width - 0.04f, 0.02f, length), darkRub), glm::vec3(x, beltTop - 0.01f, zMid));
	add(Primitives::Box(glm::vec3(width, 0.08f, length), paint), glm::vec3(x, beltTop - 0.06f, zMid));

	// side rails
	for (int side = -1; side <= 1; side += 2)
		add(Primitives::Box(glm::vec3(0.02f, 0.05f, length), steel), glm::vec3(x + side * (width * 0.5f), beltTop + 0.015f, zMid));

	// rollers at both ends (cylinders turned so their axis is X)
	for (int end = 0; end < 2; end++)
	{
		float z = end == 0 ? zStart : zEnd;
		mesh roller = Primitives::Cylinder(0.05f, width - 0.02f, 24, steel);
		glm::mat4 t = glm::translate(glm::mat4(1.0f), glm::vec3(x, beltTop - 0.04f, z))
			* glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 0, 1));
		props.push_back({ roller, t, false });
	}

	// legs
	for (int i = 0; i < 4; i++)
	{
		float lx = x + ((i & 1) ? 1 : -1) * (width * 0.5f - 0.03f);
		float lz = zMid + ((i & 2) ? 1 : -1) * (length * 0.5f - 0.08f);
		add(Primitives::Box(glm::vec3(0.04f, beltTop - 0.10f, 0.04f), steel), glm::vec3(lx, (beltTop - 0.10f) * 0.5f, lz));
	}
}

void Scene::buildCrates()
{
	Material cardboard = makeMaterial(0.60f, 0.42f, 0.22f, 0.0f, 0.75f);
	Material orange    = makeMaterial(0.90f, 0.35f, 0.08f, 0.0f, 0.40f);

	// boxes waiting on conveyor A
	add(Primitives::Box(glm::vec3(0.14f, 0.14f, 0.14f), cardboard), glm::vec3(-0.75f, 0.27f, 0.0f), 8.0f);
	add(Primitives::Box(glm::vec3(0.12f, 0.12f, 0.12f), cardboard), glm::vec3(-0.75f, 0.26f, 0.45f), -6.0f);
	add(Primitives::Box(glm::vec3(0.10f, 0.10f, 0.10f), orange),    glm::vec3(-0.75f, 0.25f, -0.4f), 15.0f);

	// one already delivered on conveyor B
	add(Primitives::Box(glm::vec3(0.12f, 0.12f, 0.12f), cardboard), glm::vec3(0.75f, 0.26f, -0.3f), 0.0f);

	// a small stack at the side of the work area
	add(Primitives::Box(glm::vec3(0.30f, 0.30f, 0.30f), cardboard), glm::vec3(1.9f, 0.15f, 0.2f), 12.0f);
	add(Primitives::Box(glm::vec3(0.30f, 0.30f, 0.30f), cardboard), glm::vec3(1.95f, 0.45f, 0.18f), -5.0f);
}

void Scene::buildSafetyLines()
{
	// yellow frame on the floor around the work area
	Material yellow = makeMaterial(0.95f, 0.75f, 0.05f, 0.0f, 0.5f);
	float hx = 1.6f, hz = 1.1f, w = 0.04f;
	add(Primitives::Box(glm::vec3(2 * hx, 0.004f, w), yellow), glm::vec3(0, 0.012f,  hz));
	add(Primitives::Box(glm::vec3(2 * hx, 0.004f, w), yellow), glm::vec3(0, 0.012f, -hz));
	add(Primitives::Box(glm::vec3(w, 0.004f, 2 * hz), yellow), glm::vec3( hx, 0.012f, 0));
	add(Primitives::Box(glm::vec3(w, 0.004f, 2 * hz), yellow), glm::vec3(-hx, 0.012f, 0));
}

void Scene::buildShowcase()
{
	// Row 1: metal, roughness goes from 0.05 to 1.0 -> mirror to brushed
	// Row 2: plastic, same roughness steps -> shows how the sky reflects on non-metals
	float radius = 0.13f;
	float spacing = 0.36f;
	int count = 7;

	for (int row = 0; row < 2; row++)
		for (int i = 0; i < count; i++)
		{
			float roughness = 0.05f + 0.95f * i / (count - 1);
			float x = (i - (count - 1) * 0.5f) * spacing;
			float z = -1.6f - row * 0.45f;
			Material m = (row == 0)
				? makeMaterial(0.95f, 0.78f, 0.35f, 1.0f, roughness)		// gold
				: makeMaterial(0.75f, 0.10f, 0.10f, 0.0f, roughness);		// red plastic
			add(Primitives::Sphere(radius, 32, m), glm::vec3(x, radius, z), 0.0f, true);
		}

	// one big chrome ball, the sky shows up clearly in it
	add(Primitives::Sphere(0.32f, 48, makeMaterial(0.95f, 0.95f, 0.97f, 1.0f, 0.05f)), glm::vec3(-2.2f, 0.32f, -0.4f), 0.0f, true);
}

void Scene::Draw(Shader& shader, Camera& camera)
{
	for (Prop& p : props)
	{
		if (p.showcase && !showShowcase) continue;
		p.shape.Draw(shader, camera, p.transform);
	}
	for (model& m : models) m.Draw(shader, camera);
}

void Scene::DrawDepth(Shader& shader)
{
	for (Prop& p : props)
	{
		if (p.showcase && !showShowcase) continue;
		p.shape.DrawDepth(shader, p.transform);
	}
	for (model& m : models) m.DrawDepth(shader);
}

void Scene::GetShadowSphere(glm::vec3& centre, float& radius) const
{
	centre = glm::vec3(0.0f, 0.3f, -0.6f);
	radius = 3.3f;
}
