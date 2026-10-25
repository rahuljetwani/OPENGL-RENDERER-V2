#include "model.h"
#include "Profiler.h"
#include <iostream>
#include <algorithm>
#include <limits>
#include <cctype>

// Assimp's UV convention has (0,0) at the bottom-left, same as OpenGL, and Texture.cpp flips.
// If textures look upsidedown on model flip this and only this one to true.
static const bool FLIP_UVS = false;

// glTF stores image paths as URIs ("Material%20Colors.jpg"); Assimp passes them through still encoded.
static std::string uriDecode(const std::string& in)
{
	std::string out;
	out.reserve(in.size());
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

static glm::mat4 toGlm(const aiMatrix4x4& m)
{
	// Assimp is row-major, glm is column-major (constructor takes column by column)
	return glm::mat4(
		m.a1, m.b1, m.c1, m.d1,
		m.a2, m.b2, m.c2, m.d2,
		m.a3, m.b3, m.c3, m.d3,
		m.a4, m.b4, m.c4, m.d4);
}

model::model(const std::string& file, bool ignoreNodes)
{
	ignoreNodeTransforms = ignoreNodes;
	Assimp::Importer importer;

	// FBX files often come with helper "$AssimpFbx$" pivot nodes; collapse them so joint names stay clean.
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

	unsigned int flags =
		aiProcess_Triangulate |				// everything becomes triangles
		aiProcess_GenSmoothNormals |		// only generates normals for meshes that have none
		aiProcess_JoinIdenticalVertices |	// gives us a real index buffer
		aiProcess_ImproveCacheLocality |	// GPU vertex-cache friendly index order
		aiProcess_SortByPType |				// separates triangles from stray lines/points
		aiProcess_GenUVCoords |				// turns non-UV mappings (spherical...) into UVs
		aiProcess_RemoveRedundantMaterials |
		aiProcess_FindInvalidData;
	if (FLIP_UVS) flags |= aiProcess_FlipUVs;
	// NOTE: deliberately NOT using aiProcess_PreTransformVertices / aiProcess_OptimizeGraph:
	// they bake or merge the node hierarchy, which would destroy the robot joints.

	const aiScene* scene = importer.ReadFile(file, flags);

	if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
	{
		std::cout << "[model] Assimp failed to load '" << file << "': " << importer.GetErrorString() << "\n";
		return;
	}

	size_t slash = file.find_last_of("/\\");
	directory = (slash == std::string::npos) ? "" : file.substr(0, slash + 1);

	meshes.reserve(scene->mNumMeshes);		// keep mesh objects from being copied around while we fill this
	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
		meshes.push_back(processMesh(scene, scene->mMeshes[i]));

	processNode(scene->mRootNode, -1);

	loaded = true;
	updateWorld();

	std::cout << "[model] Loaded '" << file << "': "
		<< meshes.size() << " meshes, " << nodes.size() << " nodes, "
		<< textureCache.size() << " textures\n";
}

// ------------------------------------------------------------------------------------------
// Import
// ------------------------------------------------------------------------------------------

int model::processNode(const aiNode* aNode, int parent)
{
	int index = (int)nodes.size();
	nodes.emplace_back();

	// (don't hold a Node& across the recursion below, the vector may reallocate)
	nodes[index].name = aNode->mName.C_Str();
	nodes[index].parent = parent;
	// ignoreNodeTransforms: drop the file's scale / rotation / translation, keep the raw mesh at its pivot
	nodes[index].bindLocal = ignoreNodeTransforms ? glm::mat4(1.0f) : toGlm(aNode->mTransformation);
	nodes[index].local = nodes[index].bindLocal;
	for (unsigned int i = 0; i < aNode->mNumMeshes; i++)
		nodes[index].meshIndices.push_back(aNode->mMeshes[i]);

	// Node names are how joints are addressed. If a name is duplicated, the first one wins.
	nodeByName.emplace(nodes[index].name, index);

	for (unsigned int i = 0; i < aNode->mNumChildren; i++)
	{
		int child = processNode(aNode->mChildren[i], index);
		nodes[index].children.push_back(child);
	}
	return index;
}

mesh model::processMesh(const aiScene* scene, const aiMesh* aMesh)
{
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;
	Material material;

	// --- vertices ---
	vertices.reserve(aMesh->mNumVertices);
	for (unsigned int i = 0; i < aMesh->mNumVertices; i++)
	{
		Vertex v;
		v.position = glm::vec3(aMesh->mVertices[i].x, aMesh->mVertices[i].y, aMesh->mVertices[i].z);

		v.normal = aMesh->HasNormals()
			? glm::vec3(aMesh->mNormals[i].x, aMesh->mNormals[i].y, aMesh->mNormals[i].z)
			: glm::vec3(0.0f, 1.0f, 0.0f);

		// per-vertex colour channel if the file has one, otherwise white
		v.color = aMesh->HasVertexColors(0)
			? glm::vec3(aMesh->mColors[0][i].r, aMesh->mColors[0][i].g, aMesh->mColors[0][i].b)
			: glm::vec3(1.0f);

		v.textUV = aMesh->HasTextureCoords(0)
			? glm::vec2(aMesh->mTextureCoords[0][i].x, aMesh->mTextureCoords[0][i].y)
			: glm::vec2(0.0f);

		vertices.push_back(v);
	}

	// --- indices ---
	indices.reserve(aMesh->mNumFaces * 3);
	for (unsigned int f = 0; f < aMesh->mNumFaces; f++)
	{
		const aiFace& face = aMesh->mFaces[f];
		if (face.mNumIndices != 3) continue;	// lines/points (SortByPType keeps them apart, we just skip them)
		for (unsigned int j = 0; j < 3; j++)
			indices.push_back(face.mIndices[j]);
	}

	// --- material ---
	if (aMesh->mMaterialIndex < scene->mNumMaterials)
	{
		const aiMaterial* mat = scene->mMaterials[aMesh->mMaterialIndex];

		aiColor4D c;
		if (mat->Get(AI_MATKEY_BASE_COLOR, c) == AI_SUCCESS)			// glTF / PBR base colour factor
			material.baseColor = glm::vec4(c.r, c.g, c.b, c.a);
		else if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, c) == AI_SUCCESS)	// OBJ/FBX/DAE diffuse colour
			material.baseColor = glm::vec4(c.r, c.g, c.b, c.a);

		float shininess;
		if (mat->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS && shininess > 1.0f)
			material.shininess = std::min(shininess, 256.0f);

		// glTF (and some FBX) carry real metal/roughness factors; formats without them keep the defaults
		float factor;
		if (mat->Get(AI_MATKEY_METALLIC_FACTOR, factor) == AI_SUCCESS)  material.metallic  = factor;
		if (mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, factor) == AI_SUCCESS) material.roughness = factor;

		// Base colour: newer Assimp reports glTF as BASE_COLOR, older formats as DIFFUSE
		loadMaterialTextures(scene, mat, aiTextureType_BASE_COLOR, "diffuse", true, textures);
		if (textures.empty())
			loadMaterialTextures(scene, mat, aiTextureType_DIFFUSE, "diffuse", true, textures);
		loadMaterialTextures(scene, mat, aiTextureType_SPECULAR, "specular", false, textures);
	}

	return mesh(vertices, indices, textures, material);
}

void model::loadMaterialTextures(const aiScene* scene, const aiMaterial* mat, aiTextureType aiType,
	const char* typeName, bool srgb, std::vector<Texture>& out)
{
	if (mat->GetTextureCount(aiType) == 0) return;

	aiString path;
	if (mat->GetTexture(aiType, 0, &path) != AI_SUCCESS) return;	// (first texture of this type only, for now)

	std::string key = std::string(path.C_Str()) + "|" + typeName;
	auto cached = textureCache.find(key);
	if (cached != textureCache.end())
	{
		out.push_back(cached->second);
		return;
	}

	// 1) texture embedded inside the file (.glb, some .fbx): Assimp names it "*0", "*1", ...
	if (const aiTexture* embedded = scene->GetEmbeddedTexture(path.C_Str()))
	{
		if (embedded->mHeight == 0)
		{
			// compressed (png/jpg bytes); mWidth is the byte length
			Texture t = Texture::FromEncodedMemory(
				reinterpret_cast<const unsigned char*>(embedded->pcData), (int)embedded->mWidth, typeName, srgb);
			textureCache.emplace(key, t);
			out.push_back(t);
		}
		else
		{
			// raw texels, stored as BGRA -> reorder to RGBA
			std::vector<unsigned char> rgba(embedded->mWidth * embedded->mHeight * 4);
			for (unsigned int i = 0; i < embedded->mWidth * embedded->mHeight; i++)
			{
				rgba[i * 4 + 0] = embedded->pcData[i].r;
				rgba[i * 4 + 1] = embedded->pcData[i].g;
				rgba[i * 4 + 2] = embedded->pcData[i].b;
				rgba[i * 4 + 3] = embedded->pcData[i].a;
			}
			Texture t = Texture::FromPixels(rgba.data(), (int)embedded->mWidth, (int)embedded->mHeight, 4, typeName, srgb);
			textureCache.emplace(key, t);
			out.push_back(t);
		}
		return;
	}

	// 2) external image file next to the model
	std::string rel = uriDecode(path.C_Str());
	std::replace(rel.begin(), rel.end(), '\\', '/');		// Windows-authored paths
	Texture t((directory + rel).c_str(), typeName, srgb);
	textureCache.emplace(key, t);
	out.push_back(t);
}

// ------------------------------------------------------------------------------------------
// Scene graph
// ------------------------------------------------------------------------------------------

void model::updateWorld()
{
	// nodes are stored parent-first, so one linear pass is enough
	for (size_t i = 0; i < nodes.size(); i++)
	{
		nodes[i].world = (nodes[i].parent < 0)
			? nodes[i].local
			: nodes[nodes[i].parent].world * nodes[i].local;
	}
	worldDirty = false;
}

bool model::HasNode(const std::string& name) const
{
	return nodeByName.find(name) != nodeByName.end();
}

bool model::SetNodeLocalTransform(const std::string& name, const glm::mat4& local)
{
	auto it = nodeByName.find(name);
	if (it == nodeByName.end()) return false;
	nodes[it->second].local = local;
	worldDirty = true;
	return true;
}

bool model::SetJointAngle(const std::string& name, const glm::vec3& axis, float radians)
{
	auto it = nodeByName.find(name);
	if (it == nodeByName.end()) return false;
	Node& n = nodes[it->second];
	n.local = n.bindLocal * glm::rotate(glm::mat4(1.0f), radians, glm::normalize(axis));
	worldDirty = true;
	return true;
}

void model::ResetPose()
{
	for (Node& n : nodes) n.local = n.bindLocal;
	worldDirty = true;
}

void model::SetPBR(float metallic, float roughness)
{
	for (mesh& m : meshes)
	{
		m.material.metallic = metallic;
		m.material.roughness = roughness;
	}
}

void model::PrintHierarchy() const
{
	if (!nodes.empty()) printNode(0, 0);
}

void model::printNode(int index, int depth) const
{
	const Node& n = nodes[index];
	std::cout << std::string(depth * 2, ' ') << "- " << (n.name.empty() ? "<unnamed>" : n.name);
	if (!n.meshIndices.empty()) std::cout << "  [" << n.meshIndices.size() << " mesh]";
	std::cout << "\n";
	for (int c : n.children) printNode(c, depth + 1);
}

// ------------------------------------------------------------------------------------------
// Drawing
// ------------------------------------------------------------------------------------------

// Six frustum planes from a view-projection matrix (Gribb/Hartmann). A point is inside when dot(plane.xyz, p) + plane.w >= 0 for all six.
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
	// world-space AABB test (conservative: never culls something visible)
	bool Intersects(const glm::vec3& mn, const glm::vec3& mx) const
	{
		for (const glm::vec4& p : planes)
		{
			glm::vec3 v(p.x >= 0.0f ? mx.x : mn.x, p.y >= 0.0f ? mx.y : mn.y, p.z >= 0.0f ? mx.z : mn.z);	// corner furthest along the plane normal
			if (glm::dot(glm::vec3(p), v) + p.w < 0.0f) return false;
		}
		return true;
	}
};

void model::Draw(Shader& shader, Camera& camera)
{
	if (!loaded) return;
	if (worldDirty) updateWorld();

	// ---- 1) culling: decide which meshes are worth a draw call ----
	visibleScratch.clear();
	{
		CpuScope scope(CPU_CULLING);
		Frustum frustum(camera.cameraMatrix);
		for (unsigned int ni = 0; ni < nodes.size(); ni++)
		{
			const Node& n = nodes[ni];
			if (n.meshIndices.empty()) continue;
			glm::mat4 M = transform * n.world;
			for (unsigned int m : n.meshIndices)
			{
				const mesh& ms = meshes[m];
				gStats.trisSubmitted += ms.TriangleCount();

				glm::vec3 wmin(std::numeric_limits<float>::max()), wmax(std::numeric_limits<float>::lowest());
				for (int c = 0; c < 8; c++)
				{
					glm::vec3 corner((c & 1) ? ms.boundsMax.x : ms.boundsMin.x,
									 (c & 2) ? ms.boundsMax.y : ms.boundsMin.y,
									 (c & 4) ? ms.boundsMax.z : ms.boundsMin.z);
					glm::vec3 p = glm::vec3(M * glm::vec4(corner, 1.0f));
					wmin = glm::min(wmin, p);
					wmax = glm::max(wmax, p);
				}
				if (frustum.Intersects(wmin, wmax)) visibleScratch.emplace_back(ni, m);
			}
		}
	}

	// ---- 2) submission: issue the draw calls ----
	{
		CpuScope scope(CPU_SUBMISSION);
		for (const auto& v : visibleScratch)
			meshes[v.second].Draw(shader, camera, transform * nodes[v.first].world);
	}
}

void model::DrawDepth(Shader& shader)
{
	if (!loaded) return;
	if (worldDirty) updateWorld();

	for (const Node& n : nodes)
		for (unsigned int m : n.meshIndices)
			meshes[m].DrawDepth(shader, transform * n.world);
}

// ------------------------------------------------------------------------------------------
// Bounds / framing
// ------------------------------------------------------------------------------------------

void model::GetBounds(glm::vec3& outMin, glm::vec3& outMax)
{
	outMin = glm::vec3(std::numeric_limits<float>::max());
	outMax = glm::vec3(std::numeric_limits<float>::lowest());
	if (!loaded) { outMin = outMax = glm::vec3(0.0f); return; }
	if (worldDirty) updateWorld();

	for (const Node& n : nodes)
	{
		glm::mat4 M = transform * n.world;
		for (unsigned int m : n.meshIndices)
		{
			const mesh& ms = meshes[m];
			// transform all 8 corners of the mesh's local box
			for (int c = 0; c < 8; c++)
			{
				glm::vec3 corner(
					(c & 1) ? ms.boundsMax.x : ms.boundsMin.x,
					(c & 2) ? ms.boundsMax.y : ms.boundsMin.y,
					(c & 4) ? ms.boundsMax.z : ms.boundsMin.z);
				glm::vec3 p = glm::vec3(M * glm::vec4(corner, 1.0f));
				outMin = glm::min(outMin, p);
				outMax = glm::max(outMax, p);
			}
		}
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

	// move the box's bottom-centre to the origin, then scale
	transform = glm::scale(glm::mat4(1.0f), glm::vec3(s))
		* glm::translate(glm::mat4(1.0f), glm::vec3(-centre.x, -mn.y, -centre.z));
}
