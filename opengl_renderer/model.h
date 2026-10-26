#ifndef MODEL_CLASS_H
#define MODEL_CLASS_H

#include <string>
#include <vector>
#include <unordered_map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.h"

// Loads a model file with Assimp (fbx, glb/gltf, obj, ...).
class model
{
public:
	// ignoreNodeTransforms = true drops the scale / rotation / position stored in the file,
	// so the mesh sits on its own pivot. The robot links need this because the sim places them.
	// Only use it for files that contain a single node.
	model(const std::string& file, bool ignoreNodeTransforms = false);

	bool IsLoaded() const { return loaded; }

	// places the whole model in the world
	glm::mat4 transform = glm::mat4(1.0f);

	void Draw(Shader& shader, Camera& camera);		// colour pass, skips meshes outside the view
	void DrawDepth(Shader& shader);					// shadow pass

	// world space bounding box (includes `transform`)
	void GetBounds(glm::vec3& outMin, glm::vec3& outMax);

	// scales the model so its biggest side is `size`, and puts it on y = 0 at the origin
	void FitToUnitSize(float size = 1.0f);

	// overrides metallic / roughness on every mesh (fbx files have no PBR data)
	void SetPBR(float metallic, float roughness);

private:
	struct Node
	{
		int parent = -1;
		std::vector<unsigned int> meshIndices;		// entries of `meshes` this node draws
		glm::mat4 local = glm::mat4(1.0f);
		glm::mat4 world = glm::mat4(1.0f);			// parent's world * local
	};

	bool loaded = false;
	bool ignoreNodeTransforms = false;
	std::string directory;							// folder of the model file, textures are loaded from here

	std::vector<mesh> meshes;
	std::vector<Node> nodes;						// a parent always comes before its children
	std::unordered_map<std::string, Texture> textureCache;	// a shared image is only loaded once
	std::vector<std::pair<unsigned int, unsigned int>> visible;	// (node, mesh) pairs that passed culling

	int processNode(const aiNode* node, int parent);
	mesh processMesh(const aiScene* scene, const aiMesh* aMesh);
	void loadDiffuseTexture(const aiScene* scene, const aiMaterial* mat, std::vector<Texture>& out);
};
#endif
