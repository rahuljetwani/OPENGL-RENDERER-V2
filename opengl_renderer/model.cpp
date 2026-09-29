#include"model.h"

model::model(const char* file)
{
	std::string text = get_file_contents(file);
	JSON = json::parse(text);

	model::file = file;
	data = getData();

	traverseNode(0);
}

void model::Draw(Shader& shader, Camera& camera, float sizeScale)
{
	for (unsigned int i = 0; i < meshes.size(); i++)
	{
		meshes[i].mesh::Draw(
			shader,
			camera,
			matricesMeshes[i],
			glm::vec3(0.0f, 0.0f, 0.0f),   // translation
			glm::quat(1.0f, 0.0f, 0.0f, 0.0f), // rotation
			glm::vec3(sizeScale)           // <- your 0-1 size control
		);
	}
}

void model::loadMesh(unsigned int indMesh)
{
	std::cout << "\n========================================\n";
	std::cout << "LOADING MESH: " << indMesh << "\n";
	std::cout << "========================================\n";

	json primitive = JSON["meshes"][indMesh]["primitives"][0];

	std::cout << "POSITION = "
		<< primitive["attributes"]["POSITION"]
		<< " | type = "
		<< primitive["attributes"]["POSITION"].type_name()
		<< "\n";

	std::cout << "NORMAL = "
		<< primitive["attributes"]["NORMAL"]
		<< " | type = "
		<< primitive["attributes"]["NORMAL"].type_name()
		<< "\n";

	std::cout << "TEXCOORD_0 = "
		<< primitive["attributes"]["TEXCOORD_0"]
		<< " | type = "
		<< primitive["attributes"]["TEXCOORD_0"].type_name()
		<< "\n";

	std::cout << "INDICES = "
		<< primitive["indices"]
		<< " | type = "
		<< primitive["indices"].type_name()
		<< "\n";

	unsigned int posAccInd =
		primitive["attributes"]["POSITION"].get<unsigned int>();

	unsigned int normalAccInd =
		primitive["attributes"]["NORMAL"].get<unsigned int>();

	unsigned int texAccInd =
		primitive["attributes"]["TEXCOORD_0"].get<unsigned int>();

	unsigned int indAccInd =
		primitive["indices"].get<unsigned int>();

	std::cout << "POSITION accessor: " << posAccInd << "\n";
	std::cout << "NORMAL accessor: " << normalAccInd << "\n";
	std::cout << "TEXCOORD accessor: " << texAccInd << "\n";
	std::cout << "INDEX accessor: " << indAccInd << "\n";

	std::vector<float> posVec =
		getFloats(JSON["accessors"][posAccInd]);

	std::cout << "Positions loaded: " << posVec.size() << "\n";

	std::vector<glm::vec3> positions =
		groupFloatvec3(posVec);

	std::vector<float> normalVec =
		getFloats(JSON["accessors"][normalAccInd]);

	std::cout << "Normals loaded: " << normalVec.size() << "\n";

	std::vector<glm::vec3> normals =
		groupFloatvec3(normalVec);

	std::vector<float> texVec =
		getFloats(JSON["accessors"][texAccInd]);

	std::cout << "UVs loaded: " << texVec.size() << "\n";

	std::vector<glm::vec2> texUVs =
		groupFloatvec2(texVec);

	std::vector<Vertex> vertices =
		assmebleVertices(positions, normals, texUVs);

	std::cout << "Vertices created: "
		<< vertices.size()
		<< "\n";

	std::vector<GLuint> indices =
		getIndices(JSON["accessors"][indAccInd]);

	std::cout << "Indices loaded: "
		<< indices.size()
		<< "\n";

	std::vector<Texture> textures =
		getTextures();

	std::cout << "Textures loaded: "
		<< textures.size()
		<< "\n";

	meshes.push_back(mesh(vertices, indices, textures));

	std::cout << "MESH COMPLETE\n";
}

void model::traverseNode(unsigned int nextNode, glm::mat4 matrix)
{
	std::cout << "\n========================================\n";
	std::cout << "ENTERING NODE: " << nextNode << "\n";
	std::cout << "========================================\n";

	json node = JSON["nodes"][nextNode];

	std::cout << "NODE JSON:\n";
	std::cout << node.dump(4) << "\n";

	glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);

	if (node.find("translation") != node.end())
	{
		std::cout << "Found translation\n";

		float transValues[3];

		for (unsigned int i = 0; i < node["translation"].size(); i++)
		{
			std::cout << "translation[" << i << "] = "
				<< node["translation"][i]
				<< " | type = "
				<< node["translation"][i].type_name()
				<< "\n";

			transValues[i] = node["translation"][i].get<float>();
		}

		translation = glm::make_vec3(transValues);
	}

	glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

	if (node.find("rotation") != node.end())
	{
		std::cout << "Found rotation\n";

		float rotValues[4] =
		{
			node["rotation"][3].get<float>(),
			node["rotation"][0].get<float>(),
			node["rotation"][1].get<float>(),
			node["rotation"][2].get<float>()
		};

		rotation = glm::make_quat(rotValues);
	}

	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

	if (node.find("scale") != node.end())
	{
		std::cout << "Found scale\n";

		float scaleValues[3];

		for (unsigned int i = 0; i < node["scale"].size(); i++)
		{
			std::cout << "scale[" << i << "] = "
				<< node["scale"][i]
				<< " | type = "
				<< node["scale"][i].type_name()
				<< "\n";

			scaleValues[i] = node["scale"][i].get<float>();
		}

		scale = glm::make_vec3(scaleValues);
	}

	glm::mat4 matNode = glm::mat4(1.0f);

	if (node.find("matrix") != node.end())
	{
		std::cout << "Found matrix\n";

		float matValues[16];

		for (unsigned int i = 0; i < node["matrix"].size(); i++)
		{
			std::cout << "matrix[" << i << "] = "
				<< node["matrix"][i]
				<< " | type = "
				<< node["matrix"][i].type_name()
				<< "\n";

			matValues[i] = node["matrix"][i].get<float>();
		}

		matNode = glm::make_mat4(matValues);
	}

	glm::mat4 trans = glm::mat4(1.0f);
	glm::mat4 rot = glm::mat4(1.0f);
	glm::mat4 sca = glm::mat4(1.0f);

	trans = glm::translate(trans, translation);
	rot = glm::mat4_cast(rotation);
	sca = glm::scale(sca, scale);

	glm::mat4 matNextNode = matrix * matNode * trans * rot * sca;

	if (node.find("mesh") != node.end())
	{
		std::cout << "Node has mesh: "
			<< node["mesh"]
			<< "\n";

		translationsMeshes.push_back(translation);
		rotationsMeshes.push_back(rotation);
		scalesMeshes.push_back(scale);
		matricesMeshes.push_back(matNextNode);

		std::cout << "Calling loadMesh()...\n";

		loadMesh(node["mesh"]);

		std::cout << "loadMesh() completed.\n";
	}

	if (node.find("children") != node.end())
	{
		std::cout << "Children count: "
			<< node["children"].size()
			<< "\n";

		for (unsigned int i = 0; i < node["children"].size(); i++)
		{
			std::cout << "Going to child: "
				<< node["children"][i]
				<< "\n";

			traverseNode(node["children"][i], matNextNode);
		}
	}

	std::cout << "FINISHED NODE: " << nextNode << "\n";
}

std::vector<unsigned char> model::getData()
{
	std::string bytesText;
	std::string uri = JSON["buffers"][0]["uri"];

	std::string fileStr = std::string(file);
	std::string fileDirectory = fileStr.substr(0, fileStr.find_last_of('/') + 1);
	bytesText = get_file_contents((fileDirectory + uri).c_str());

	std::vector<unsigned char> data(bytesText.begin(), bytesText.end());
	return data;
}

std::vector<float> model::getFloats(json accessor)
{
	std::vector<float> floatVec;

	unsigned int bufferViewInd = accessor.value("bufferView", 1);
	unsigned int count = accessor["count"];
	unsigned int accByteOffset = accessor.value("byteOffset", 0);
	std::string type = accessor["type"];

	json bufferView = JSON["bufferViews"][bufferViewInd];

	// FIX: byteOffset is optional in glTF
	unsigned int byteOffset = bufferView.value("byteOffset", 0);

	unsigned int numPerVert;

	if (type == "SCALAR")
		numPerVert = 1;
	else if (type == "VEC2")
		numPerVert = 2;
	else if (type == "VEC3")
		numPerVert = 3;
	else if (type == "VEC4")
		numPerVert = 4;
	else
		throw std::invalid_argument("TYPE INVALID NOT SCALAR/VEC2/VEC3/VEC4");

	unsigned int beginningOfData = byteOffset + accByteOffset;
	unsigned int lengthOfData = count * 4 * numPerVert;

	for (unsigned int i = beginningOfData;
		i < beginningOfData + lengthOfData;
		i += 4)
	{
		unsigned char byte[] =
		{
			data[i],
			data[i + 1],
			data[i + 2],
			data[i + 3]
		};

		float value;

		std::memcpy(&value, byte, sizeof(float));

		floatVec.push_back(value);
	}

	return floatVec;
}

std::vector<GLuint> model::getIndices(json accessor)
{
	std::vector<GLuint> indices;

	unsigned int bufferViewInd = accessor.value("bufferView", 0);
	unsigned int count = accessor["count"];
	unsigned int accByteOffset = accessor.value("byteOffset", 0);
	unsigned int componentType = accessor["componentType"];

	json bufferView = JSON["bufferViews"][bufferViewInd];

	// FIX: byteOffset is optional in glTF
	unsigned int byteOffset = bufferView.value("byteOffset", 0);

	unsigned int beginningOfData = byteOffset + accByteOffset;

	if (componentType == 5125)
	{
		for (unsigned int i = beginningOfData;
			i < byteOffset + accByteOffset + count * 4;
			i += 4)
		{
			unsigned char byte[] =
			{
				data[i],
				data[i + 1],
				data[i + 2],
				data[i + 3]
			};

			unsigned int value;

			std::memcpy(&value, byte, sizeof(unsigned int));

			indices.push_back((GLuint)value);
		}
	}
	else if (componentType == 5123)
	{
		for (unsigned int i = beginningOfData;
			i < byteOffset + accByteOffset + count * 2;
			i += 2)
		{
			unsigned char byte[] =
			{
				data[i],
				data[i + 1]
			};

			unsigned short value;

			std::memcpy(&value, byte, sizeof(unsigned short));

			indices.push_back(GLuint(value));
		}
	}
	else if (componentType == 5122)
	{
		for (unsigned int i = beginningOfData;
			i < byteOffset + accByteOffset + count * 2;
			i += 2)
		{
			unsigned char byte[] =
			{
				data[i],
				data[i + 1]
			};

			short value;

			std::memcpy(&value, byte, sizeof(short));

			indices.push_back(GLuint(value));
		}
	}

	return indices;
}

std::vector<Texture> model::getTextures()
{
	std::vector<Texture> textures;

	std::string fileStr = std::string(file);
	std::string fileDirectory = fileStr.substr(0, fileStr.find_last_of('/') + 1);


	for (unsigned int i = 0; i < JSON["images"].size(); i++)
	{
		std::string texPath = JSON["images"][i]["uri"];

		bool skip = false;
		for (unsigned int j = 0; j < loadedTexName.size(); j++)
		{
			if (loadedTexName[j] == texPath)
			{
				textures.push_back(loadedTex[j]);
				skip = true;
				break;
			}
		}

		if (!skip)
		{
			if (texPath.find("baseColor") != std::string::npos)
			{
				Texture diffuse = Texture((fileDirectory + texPath).c_str(), "diffuse", loadedTex.size());
				textures.push_back(diffuse);
				loadedTex.push_back(diffuse);
				loadedTexName.push_back(texPath);
			}
			else if (texPath.find("metallicRoughness") != std::string::npos)
			{
				Texture specular = Texture((fileDirectory + texPath).c_str(), "specular", loadedTex.size());
				textures.push_back(specular);
				loadedTex.push_back(specular);
				loadedTexName.push_back(texPath);
			}
		}
	}
	return textures;
}

std::vector<Vertex> model::assmebleVertices
(
	std::vector<glm::vec3> positions,
	std::vector<glm::vec3> normals,
	std::vector<glm::vec2> texUVs
)
{
	std::vector<Vertex> vertices;
	for (int i = 0; i < positions.size(); i++)
	{
		vertices.push_back
		(
			Vertex
			{
				positions[i],
				normals[i],
				glm::vec3(1.0f, 1.0f, 1.0f),
				texUVs[i]
			}
		);
	}
	return vertices;
}

std::vector < glm::vec2> model::groupFloatvec2(std::vector<float> floatVec)
{
	std::vector<glm::vec2> vectors;
	for (int i = 0; i < floatVec.size(); i)
		vectors.push_back(glm::vec2(floatVec[i++], floatVec[i++]));	
	
	return vectors;
}
std::vector < glm::vec3> model::groupFloatvec3(std::vector<float> floatVec)
{
	std::vector<glm::vec3> vectors;
	for (int i = 0; i < floatVec.size(); i)
		vectors.push_back(glm::vec3(floatVec[i++], floatVec[i++], floatVec[i++]));

	return vectors;
}
std::vector < glm::vec4> model::groupFloatvec4(std::vector<float> floatVec)
{
	std::vector<glm::vec4> vectors;
	for (int i = 0; i < floatVec.size(); i)
		vectors.push_back(glm::vec4(floatVec[i++], floatVec[i++], floatVec[i++], floatVec[i++]));

	return vectors;
}
