#include "model.h"
#include "Profiler.h"
#include <iostream>
#include <algorithm>
#include <limits>
#include <cctype>

// glTF writes image paths like "Material%20Colors.jpg", turn them back into normal text
static std::string uriDecode(const std::string& in)
{
	std::string out;
	for (size_t i = 0; i < in.size(); i++)
	{
		if (in[i] == '%' && i + 2 < in.size() && isxdigit((unsigned char)in[i + 1]) && isxdigit((unsigned char)in[i + 2]))
		{
			out += (char)std::stoi(in.substr(i + 1, 2), nullptr, 16);
			i += 2;
		}
		else out += in[i];
	}
	return out;
}

// Assimp matrices are row-major, glm is column-major
static glm::mat4 toGlm(const aiMatrix4x4& m)
{
	return glm::mat4(
		m.a1, m.b1, m.c1, m.d1,
		m.a2, m.b2, m.c2, m.d2,
		m.a3, m.b3, m.c3, m.d3,
		m.a4, m.b4, m.c4, m.d4);
}

// the 8 corners of a local box, moved into world space, as a new box
static void worldBox(const mesh& m, const glm::mat4& M, glm::vec3& outMin, glm::vec3& outMax)
{
	outMin = glm::vec3(std::numeric_limits<float>::max());
	outMax = glm::vec3(std::numeric_limits<float>::lowest());
	for (int c = 0; c < 8; c++)
	{
		glm::vec3 corner((c & 1) ? m.boundsMax.x : m.boundsMin.x,
						 (c & 2) ? m.boundsMax.y : m.boundsMin.y,
						 (c & 4) ? m.boundsMax.z : m.boundsMin.z);
		glm::vec3 p = glm::vec3(M * glm::vec4(corner, 1.0f));
		outMin = glm::min(outMin, p);
		outMax = glm::max(outMax, p);
	}
}

// Six planes of the camera view. A box is outside if it is fully behind one plane.
struct Frustum
{
	glm::vec4 planes[6];

	explicit Frustum(const glm::mat4& m)
	{
		auto row = [&](int i) { return glm::vec4(m[0][i], m[1][i], m[2][i], m[3][i]); };
		planes[0] = row(3) + row(0);	// left
		planes[1] = row(3) - row(0);	// right
		planes[2] = row(3) + row(1);	// bottom
		planes[3] = row(3) - row(1);	// top
		planes[4] = row(3) + row(2);	// near
		planes[5] = row(3) - row(2);	// far
	}

	bool Sees(const glm::vec3& mn, const glm::vec3& mx) const
	{
		for (const glm::vec4& p : planes)
		{
			// corner of the box that is furthest along the plane normal
			glm::vec3 v(p.x >= 0.0f ? mx.x : mn.x, p.y >= 0.0f ? mx.y : mn.y, p.z >= 0.0f ? mx.z : mn.z);
			if (glm::dot(glm::vec3(p), v) + p.w < 0.0f) return false;
		}
		return true;
	}
};

model::model(const std::string& file, bool ignoreNodes)
{
	ignoreNodeTransforms = ignoreNodes;
	Assimp::Importer importer;

	// don't create the extra "$AssimpFbx$" pivot nodes for fbx files
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

	// no PreTransformVertices / OptimizeGraph, they would flatten the node tree
	unsigned int flags =
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_JoinIdenticalVertices |
		aiProcess_ImproveCacheLocality |
		aiProcess_SortByPType |
		aiProcess_GenUVCoords |
		aiProcess_RemoveRedundantMaterials |
		aiProcess_FindInvalidData;

	const aiScene* scene = importer.ReadFile(file, flags);
	if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
	{
		std::cout << "[model] could not load '" << file << "': " << importer.GetErrorString() << "\n";
		return;
	}

	size_t slash = file.find_last_of("/\\");
	directory = (slash == std::string::npos) ? "" : file.substr(0, slash + 1);

	meshes.reserve(scene->mNumMeshes);
	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
		meshes.push_back(processMesh(scene, scene->mMeshes[i]));

	processNode(scene->mRootNode, -1);

	// parents come first in `nodes`, so one pass gives every world matrix
	for (Node& n : nodes)
		n.world = (n.parent < 0) ? n.local : nodes[n.parent].world * n.local;

	loaded = true;
	std::cout << "[model] loaded '" << file << "' (" << meshes.size() << " meshes)\n";
}

int model::processNode(const aiNode* aNode, int parent)
{
	int index = (int)nodes.size();
	nodes.emplace_back();
	nodes[index].parent = parent;
	nodes[index].local = ignoreNodeTransforms ? glm::mat4(1.0f) : toGlm(aNode->mTransformation);
	for (unsigned int i = 0; i < aNode->mNumMeshes; i++)
		nodes[index].meshIndices.push_back(aNode->mMeshes[i]);

	for (unsigned int i = 0; i < aNode->mNumChildren; i++)
		processNode(aNode->mChildren[i], index);

	return index;
}

mesh model::processMesh(const aiScene* scene, const aiMesh* aMesh)
{
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;
	Material material;

	for (unsigned int i = 0; i < aMesh->mNumVertices; i++)
	{
		Vertex v;
		v.position = glm::vec3(aMesh->mVertices[i].x, aMesh->mVertices[i].y, aMesh->mVertices[i].z);

		v.normal = aMesh->HasNormals()
			? glm::vec3(aMesh->mNormals[i].x, aMesh->mNormals[i].y, aMesh->mNormals[i].z)
			: glm::vec3(0.0f, 1.0f, 0.0f);

		v.color = aMesh->HasVertexColors(0)
			? glm::vec3(aMesh->mColors[0][i].r, aMesh->mColors[0][i].g, aMesh->mColors[0][i].b)
			: glm::vec3(1.0f);

		v.textUV = aMesh->HasTextureCoords(0)
			? glm::vec2(aMesh->mTextureCoords[0][i].x, aMesh->mTextureCoords[0][i].y)
			: glm::vec2(0.0f);

		vertices.push_back(v);
	}

	for (unsigned int f = 0; f < aMesh->mNumFaces; f++)
	{
		const aiFace& face = aMesh->mFaces[f];
		if (face.mNumIndices != 3) continue;		// skip stray lines and points
		for (unsigned int j = 0; j < 3; j++)
			indices.push_back(face.mIndices[j]);
	}

	if (aMesh->mMaterialIndex < scene->mNumMaterials)
	{
		const aiMaterial* mat = scene->mMaterials[aMesh->mMaterialIndex];

		aiColor4D c;
		if (mat->Get(AI_MATKEY_BASE_COLOR, c) == AI_SUCCESS)			// glTF
			material.baseColor = glm::vec4(c.r, c.g, c.b, c.a);
		else if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, c) == AI_SUCCESS)	// obj / fbx
			material.baseColor = glm::vec4(c.r, c.g, c.b, c.a);

		float factor;
		if (mat->Get(AI_MATKEY_METALLIC_FACTOR, factor) == AI_SUCCESS)  material.metallic = factor;
		if (mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, factor) == AI_SUCCESS) material.roughness = factor;

		loadDiffuseTexture(scene, mat, textures);
	}

	return mesh(vertices, indices, textures, material);
}

void model::loadDiffuseTexture(const aiScene* scene, const aiMaterial* mat, std::vector<Texture>& out)
{
	// newer Assimp reports glTF colour as BASE_COLOR, older formats as DIFFUSE
	aiTextureType type = aiTextureType_BASE_COLOR;
	if (mat->GetTextureCount(type) == 0) type = aiTextureType_DIFFUSE;
	if (mat->GetTextureCount(type) == 0) return;

	aiString path;
	if (mat->GetTexture(type, 0, &path) != AI_SUCCESS) return;

	std::string key = path.C_Str();
	auto found = textureCache.find(key);
	if (found != textureCache.end())
	{
		out.push_back(found->second);
		return;
	}

	// image stored inside the file (glb), Assimp names it "*0", "*1", ...
	if (const aiTexture* embedded = scene->GetEmbeddedTexture(path.C_Str()))
	{
		if (embedded->mHeight == 0)
		{
			// still a png / jpg, mWidth is the byte count
			Texture t = Texture::FromEncodedMemory(
				reinterpret_cast<const unsigned char*>(embedded->pcData), (int)embedded->mWidth, "diffuse", true);
			textureCache.emplace(key, t);
			out.push_back(t);
		}
		else
		{
			// raw pixels
			std::vector<unsigned char> rgba(embedded->mWidth * embedded->mHeight * 4);
			for (unsigned int i = 0; i < embedded->mWidth * embedded->mHeight; i++)
			{
				rgba[i * 4 + 0] = embedded->pcData[i].r;
				rgba[i * 4 + 1] = embedded->pcData[i].g;
				rgba[i * 4 + 2] = embedded->pcData[i].b;
				rgba[i * 4 + 3] = embedded->pcData[i].a;
			}
			Texture t = Texture::FromPixels(rgba.data(), (int)embedded->mWidth, (int)embedded->mHeight, 4, "diffuse", true);
			textureCache.emplace(key, t);
			out.push_back(t);
		}
		return;
	}

	// normal image file next to the model
	std::string rel = uriDecode(path.C_Str());
	std::replace(rel.begin(), rel.end(), '\\', '/');
	Texture t((directory + rel).c_str(), "diffuse", true);
	textureCache.emplace(key, t);
	out.push_back(t);
}

void model::SetPBR(float metallic, float roughness)
{
	for (mesh& m : meshes)
	{
		m.material.metallic = metallic;
		m.material.roughness = roughness;
	}
}

void model::Draw(Shader& shader, Camera& camera)
{
	if (!loaded) return;

	// 1) find the meshes that are inside the camera view
	visible.clear();
	{
		CpuScope timer(CPU_CULLING);
		Frustum frustum(camera.cameraMatrix);
		for (unsigned int ni = 0; ni < nodes.size(); ni++)
		{
			glm::mat4 M = transform * nodes[ni].world;
			for (unsigned int mi : nodes[ni].meshIndices)
			{
				gStats.trisSubmitted += meshes[mi].TriangleCount();

				glm::vec3 mn, mx;
				worldBox(meshes[mi], M, mn, mx);
				if (frustum.Sees(mn, mx)) visible.emplace_back(ni, mi);
			}
		}
	}

	// 2) draw them
	{
		CpuScope timer(CPU_SUBMISSION);
		for (const auto& v : visible)
			meshes[v.second].Draw(shader, camera, transform * nodes[v.first].world);
	}
}

void model::DrawDepth(Shader& shader)
{
	if (!loaded) return;

	for (const Node& n : nodes)
		for (unsigned int mi : n.meshIndices)
			meshes[mi].DrawDepth(shader, transform * n.world);
}

void model::GetBounds(glm::vec3& outMin, glm::vec3& outMax)
{
	outMin = glm::vec3(std::numeric_limits<float>::max());
	outMax = glm::vec3(std::numeric_limits<float>::lowest());
	if (!loaded) { outMin = outMax = glm::vec3(0.0f); return; }

	for (const Node& n : nodes)
		for (unsigned int mi : n.meshIndices)
		{
			glm::vec3 mn, mx;
			worldBox(meshes[mi], transform * n.world, mn, mx);
			outMin = glm::min(outMin, mn);
			outMax = glm::max(outMax, mx);
		}
}

void model::FitToUnitSize(float size)
{
	transform = glm::mat4(1.0f);
	glm::vec3 mn, mx;
	GetBounds(mn, mx);

	glm::vec3 extent = mx - mn;
	float largest = std::max(extent.x, std::max(extent.y, extent.z));
	if (largest <= 0.0f) return;

	float s = size / largest;
	glm::vec3 centre = 0.5f * (mn + mx);

	// bottom centre goes to the origin, then scale
	transform = glm::scale(glm::mat4(1.0f), glm::vec3(s))
		* glm::translate(glm::mat4(1.0f), glm::vec3(-centre.x, -mn.y, -centre.z));
}
