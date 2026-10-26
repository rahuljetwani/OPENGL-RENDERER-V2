#include "Primitives.h"
#include <cmath>

static const float PI = 3.14159265f;

static Vertex makeVertex(const glm::vec3& p, const glm::vec3& n, const glm::vec2& uv)
{
	Vertex v;
	v.position = p;
	v.normal = n;
	v.color = glm::vec3(1.0f);		// the shader multiplies by this, so keep it white
	v.textUV = uv;
	return v;
}

// adds a quad (4 corners in counter-clockwise order, seen from outside)
static void addQuad(std::vector<Vertex>& vs, std::vector<GLuint>& is,
	const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& d, const glm::vec3& n)
{
	GLuint first = (GLuint)vs.size();
	vs.push_back(makeVertex(a, n, glm::vec2(0, 0)));
	vs.push_back(makeVertex(b, n, glm::vec2(1, 0)));
	vs.push_back(makeVertex(c, n, glm::vec2(1, 1)));
	vs.push_back(makeVertex(d, n, glm::vec2(0, 1)));
	GLuint quad[6] = { 0, 1, 2, 0, 2, 3 };
	for (GLuint q : quad) is.push_back(first + q);
}

mesh Primitives::Box(const glm::vec3& size, const Material& material)
{
	std::vector<Vertex> vs;
	std::vector<GLuint> is;
	std::vector<Texture> tex;

	glm::vec3 h = size * 0.5f;

	addQuad(vs, is, { -h.x,-h.y, h.z}, { h.x,-h.y, h.z}, { h.x, h.y, h.z}, {-h.x, h.y, h.z}, { 0, 0, 1});	// front
	addQuad(vs, is, { h.x,-h.y,-h.z}, {-h.x,-h.y,-h.z}, {-h.x, h.y,-h.z}, { h.x, h.y,-h.z}, { 0, 0,-1});	// back
	addQuad(vs, is, { h.x,-h.y, h.z}, { h.x,-h.y,-h.z}, { h.x, h.y,-h.z}, { h.x, h.y, h.z}, { 1, 0, 0});	// right
	addQuad(vs, is, {-h.x,-h.y,-h.z}, {-h.x,-h.y, h.z}, {-h.x, h.y, h.z}, {-h.x, h.y,-h.z}, {-1, 0, 0});	// left
	addQuad(vs, is, {-h.x, h.y, h.z}, { h.x, h.y, h.z}, { h.x, h.y,-h.z}, {-h.x, h.y,-h.z}, { 0, 1, 0});	// top
	addQuad(vs, is, {-h.x,-h.y,-h.z}, { h.x,-h.y,-h.z}, { h.x,-h.y, h.z}, {-h.x,-h.y, h.z}, { 0,-1, 0});	// bottom

	return mesh(vs, is, tex, material);
}

mesh Primitives::Plane(float size, const Material& material)
{
	std::vector<Vertex> vs;
	std::vector<GLuint> is;
	std::vector<Texture> tex;

	float h = size * 0.5f;
	addQuad(vs, is, {-h, 0, h}, { h, 0, h}, { h, 0,-h}, {-h, 0,-h}, {0, 1, 0});

	return mesh(vs, is, tex, material);
}

mesh Primitives::Sphere(float radius, int segments, const Material& material)
{
	std::vector<Vertex> vs;
	std::vector<GLuint> is;
	std::vector<Texture> tex;

	int rings = segments;
	int slices = segments * 2;

	for (int r = 0; r <= rings; r++)
	{
		float v = (float)r / rings;
		float theta = v * PI;						// 0 = top, PI = bottom
		for (int s = 0; s <= slices; s++)
		{
			float u = (float)s / slices;
			float phi = u * 2.0f * PI;
			glm::vec3 n(std::sin(theta) * std::cos(phi), std::cos(theta), std::sin(theta) * std::sin(phi));
			vs.push_back(makeVertex(n * radius, n, glm::vec2(u, 1.0f - v)));
		}
	}

	for (int r = 0; r < rings; r++)
		for (int s = 0; s < slices; s++)
		{
			GLuint a = r * (slices + 1) + s;
			GLuint b = a + slices + 1;
			// counter-clockwise seen from outside
			is.push_back(a); is.push_back(a + 1); is.push_back(b);
			is.push_back(a + 1); is.push_back(b + 1); is.push_back(b);
		}

	return mesh(vs, is, tex, material);
}

mesh Primitives::Cylinder(float radius, float height, int segments, const Material& material)
{
	std::vector<Vertex> vs;
	std::vector<GLuint> is;
	std::vector<Texture> tex;

	float h = height * 0.5f;

	// side
	for (int s = 0; s <= segments; s++)
	{
		float u = (float)s / segments;
		float a = u * 2.0f * PI;
		glm::vec3 n(std::cos(a), 0.0f, std::sin(a));
		vs.push_back(makeVertex(glm::vec3(n.x * radius, -h, n.z * radius), n, glm::vec2(u, 0)));
		vs.push_back(makeVertex(glm::vec3(n.x * radius,  h, n.z * radius), n, glm::vec2(u, 1)));
	}
	for (int s = 0; s < segments; s++)
	{
		GLuint b0 = s * 2, t0 = b0 + 1, b1 = b0 + 2, t1 = b0 + 3;
		// counter-clockwise seen from outside
		is.push_back(b0); is.push_back(t0); is.push_back(b1);
		is.push_back(t0); is.push_back(t1); is.push_back(b1);
	}

	// caps
	for (int cap = 0; cap < 2; cap++)
	{
		float y = cap == 0 ? h : -h;
		glm::vec3 n(0.0f, cap == 0 ? 1.0f : -1.0f, 0.0f);
		GLuint centre = (GLuint)vs.size();
		vs.push_back(makeVertex(glm::vec3(0, y, 0), n, glm::vec2(0.5f, 0.5f)));
		for (int s = 0; s <= segments; s++)
		{
			float a = (float)s / segments * 2.0f * PI;
			vs.push_back(makeVertex(glm::vec3(std::cos(a) * radius, y, std::sin(a) * radius), n,
				glm::vec2(std::cos(a) * 0.5f + 0.5f, std::sin(a) * 0.5f + 0.5f)));
		}
		for (int s = 0; s < segments; s++)
		{
			GLuint p0 = centre + 1 + s, p1 = p0 + 1;
			// top is wound the other way round than the bottom
			if (cap == 0) { is.push_back(centre); is.push_back(p1); is.push_back(p0); }
			else          { is.push_back(centre); is.push_back(p0); is.push_back(p1); }
		}
	}

	return mesh(vs, is, tex, material);
}
