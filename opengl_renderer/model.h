#ifndef MODEL_CLASS_H
#define MODEL_CLASS_H

#include <string>
#include <vector>
#include <unordered_map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.h"

// Loads any format Assimp understands (glTF/GLB, FBX, OBJ, DAE, STL, 3DS, ...).
//
// The scene graph is KEPT (not flattened), so a robot arm exported as
//   base -> shoulder -> upper_arm -> elbow -> forearm -> wrist -> gripper
// can be animated at runtime by node name:  arm.SetJointAngle("elbow", axis, radians);
class model
{
public:
	// ignoreNodeTransforms = true: every node's transform is replaced with identity, so the meshes come out as raw
	// vertex data sitting at their own pivot. Use it for robot links exported from Blender as FBX, whose nodes carry
	// a x100 scale, a 90deg rotation and the rest-pose translation that the sim (FK) wants to own instead.
	// (Only meant for single-node files: it throws away the relative placement of nodes inside the file.)
	model(const std::string& file, bool ignoreNodeTransforms = false);

	bool IsLoaded() const { return loaded; }

	// Root transform applied on top of the whole hierarchy (position / scale / rotate the model in the world)
	glm::mat4 transform = glm::mat4(1.0f);

	void Draw(Shader& shader, Camera& camera);		// colour pass (frustum-culls every mesh against the camera first)
	void DrawDepth(Shader& shader);					// shadow pass (geometry only)

	// --- framing helpers ---
	// World-space bounding box of the whole model (includes `transform` and current joint poses)
	void GetBounds(glm::vec3& outMin, glm::vec3& outMax);
	// Scales + moves `transform` so the model's largest side equals `size` and it stands on y = 0, centred on x/z
	void FitToUnitSize(float size = 1.0f);

	// --- PBR ---
	// Overrides the metal/roughness of every mesh (FBX files carry no PBR data, glTF files do and keep theirs by default)
	void SetPBR(float metallic, float roughness);

	// --- joints / hierarchy ---
	bool HasNode(const std::string& name) const;
	// Replace a node's local transform outright
	bool SetNodeLocalTransform(const std::string& name, const glm::mat4& local);
	// Rotate a node by `radians` around `axis` (in the node's own local space), relative to its ORIGINAL pose.
	// Calling it every frame with a new angle is exactly what you want for a robot joint.
	bool SetJointAngle(const std::string& name, const glm::vec3& axis, float radians);
	// Put everything back into the pose it was imported in
	void ResetPose();
	void PrintHierarchy() const;

private:
	struct Node
	{
		std::string name;
		int parent = -1;
		std::vector<int> children;
		std::vector<unsigned int> meshIndices;	// indices into `meshes`
		glm::mat4 bindLocal = glm::mat4(1.0f);	// pose as imported
		glm::mat4 local = glm::mat4(1.0f);		// current pose
		glm::mat4 world = glm::mat4(1.0f);		// parent.world * local (rebuilt by updateWorld)
	};

	bool loaded = false;
	bool ignoreNodeTransforms = false;
	std::string directory;

	std::vector<mesh> meshes;						// one per aiMesh, shared by every node that references it
	std::vector<Node> nodes;						// depth-first order: a parent always comes before its children
	std::unordered_map<std::string, int> nodeByName;
	std::unordered_map<std::string, Texture> textureCache;	// "path" + type -> texture, so shared images load once
	bool worldDirty = true;
	std::vector<std::pair<unsigned int, unsigned int>> visibleScratch;	// (node, mesh) pairs that survived culling (reused every frame)

	int processNode(const aiNode* node, int parent);   // returns the new node's index
	mesh processMesh(const aiScene* scene, const aiMesh* aMesh);
	void loadMaterialTextures(const aiScene* scene, const aiMaterial* mat, aiTextureType aiType,
		const char* typeName, bool srgb, std::vector<Texture>& out);
	void updateWorld();
	void printNode(int index, int depth) const;
};
#endif
