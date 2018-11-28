//
//  Model.cpp
//  
//
//  Created by Bret Jackson on 2/2/17.
//
//

#include "Model.h"

namespace basicgraphics {

	ProgressReporter::ProgressReporter()
	{
		_firstUpdate = true;
	}

	ProgressReporter::~ProgressReporter()
	{
	}

	void ProgressReporter::reset()
	{
		_firstUpdate = true;
	}

	bool ProgressReporter::Update(float percentage)
	{
		if (_firstUpdate) {
			std::cout << std::endl << "Importing Progress:       ";
			_firstUpdate = false;
		}
		std::cout << "\b\b\b\b\b" << std::setfill(' ') << std::setw(4) << percentage << "%";
		flush(std::cout);
		return true;
	}

	Model::Model(const std::string &filename, const double scale, glm::vec4 materialColor /*=glm::vec4(1.0)*/) : _materialColor(materialColor)
	{
		//TODO not entirely sure this is threadsafe, although assimp says the library is as long as you have separate importer objects
		Assimp::Logger::LogSeverity severity = Assimp::Logger::NORMAL;
		// Create a logger instance for Console Output
		Assimp::DefaultLogger::create("", severity, aiDefaultLogStream_STDOUT);

		int numIndices = 0;
		importMesh(filename, numIndices, scale);
	}

	Model::Model(const std::string &fileContents, glm::vec4 materialColor /*=glm::vec4(1.0)*/) : _materialColor(materialColor)
	{
		Assimp::Logger::LogSeverity severity = Assimp::Logger::NORMAL;
		// Create a logger instance for Console Output
		Assimp::DefaultLogger::create("", severity, aiDefaultLogStream_STDOUT);

		importMeshFromString(fileContents);
	}

	Model::~Model()
	{
		// Kill it after the work is done
		Assimp::DefaultLogger::kill();
	}

	void Model::draw(GLSLProgram &shader) {
		for (int i = 0; i < _meshes.size(); i++) {
			_meshes[i]->draw(shader);
		}
	}

	void Model::importMesh(const std::string &filename, int &numIndices, const double scale/*=1.0*/)
	{
		if (_importer.get() == nullptr) {
			_importer.reset(new Assimp::Importer());
		}

		const aiScene* scene = _importer->ReadFile(filename, aiProcess_Triangulate);

		// If the import failed, report it
		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			Assimp::DefaultLogger::get()->info(_importer->GetErrorString());
			return;
		}


		glm::mat4 scaleMat(1.0);
		scaleMat[0][0] = scale;
		scaleMat[1][1] = scale;
		scaleMat[2][2] = scale;

		this->processNode(scene->mRootNode, scene, scaleMat);

		_importer->FreeScene();

	}

	void Model::importMeshFromString(const std::string &fileContents) {
		if (_importer.get() == nullptr) {
			_importer.reset(new Assimp::Importer());
		}

		size_t size = sizeof(unsigned char) * fileContents.size();

		const aiScene* scene = _importer->ReadFileFromMemory(fileContents.c_str(), size, aiProcessPreset_TargetRealtime_Quality, ".nff");
		// If the import failed, report it
		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			Assimp::DefaultLogger::get()->info(_importer->GetErrorString());
			return;
		}


		glm::mat4 scaleMat(1.0);

		this->processNode(scene->mRootNode, scene, scaleMat);

		_importer->FreeScene();
	}

	// Processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void Model::processNode(aiNode* node, const aiScene* scene, const glm::mat4 scaleMat)
	{
		// Process each mesh located at the current node
		for (GLuint i = 0; i < node->mNumMeshes; i++)
		{
			// The node object only contains indices to index the actual objects in the scene.
			// The scene contains all the data, node is just to keep stuff organized (like relations between nodes).
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			this->_meshes.push_back(this->processMesh(mesh, scene, scaleMat));
		}
		// After we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for (GLuint i = 0; i < node->mNumChildren; i++)
		{
			this->processNode(node->mChildren[i], scene, scaleMat);
		}

	}

	std::shared_ptr<Mesh> Model::processMesh(aiMesh* mesh, const aiScene* scene, const glm::mat4 scaleMat)
	{
		// Data to fill
		std::vector<Mesh::Vertex> cpuVertexArray;
		std::vector<int>			 cpuIndexArray;
		std::vector<std::shared_ptr<Texture>  > textures;

		// Walk through each of the mesh's vertices
		for (GLuint i = 0; i < mesh->mNumVertices; i++)
		{
			Mesh::Vertex vertex;

			glm::vec4 position;
			position.x = mesh->mVertices[i].x;
			position.y = mesh->mVertices[i].y;
			position.z = mesh->mVertices[i].z;
			position.w = 1.0;
			glm::vec3 normal;
			normal.x = mesh->mNormals[i].x;
			normal.y = mesh->mNormals[i].y;
			normal.z = mesh->mNormals[i].z;

			vertex.position = (scaleMat * position);
            vertex.normal = glm::normalize(normal);

			// Texture Coordinates
			if (mesh->mTextureCoords[0]) {
				vertex.texCoord0 = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
			}
			else {
				vertex.texCoord0 = glm::vec2(0.0f, 0.0f);
			}

			cpuVertexArray.push_back(vertex);
		}

		for (GLuint i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];

			for (GLuint j = 0; j < face.mNumIndices; j++) {
				cpuIndexArray.push_back(face.mIndices[j]);
			}
		}
		// Process materials
		if (scene->HasMaterials())
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			// We assume a convention for sampler names in the shaders. Each diffuse texture should be named
			// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
			// Same applies to other texture as the following list summarizes:
			// Diffuse: texture_diffuseN
			// Specular: texture_specularN
			// Normal: texture_normalN

			std::vector<std::shared_ptr<Texture> > diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE);
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		}

		const int numVertices = cpuVertexArray.size();
		const int cpuVertexByteSize = sizeof(Mesh::Vertex) * numVertices;
		const int cpuIndexByteSize = sizeof(int) * cpuIndexArray.size();
		std::shared_ptr<Mesh> gpuMesh(new Mesh(textures, GL_TRIANGLES, GL_STATIC_DRAW, cpuVertexByteSize, cpuIndexByteSize, 0, cpuVertexArray, cpuIndexArray.size(), cpuIndexByteSize, &cpuIndexArray[0]));
		gpuMesh->setMaterialColor(_materialColor);
		return gpuMesh;
	}

	// Checks all material textures of a given type and loads the textures if they're not loaded yet.
	// The required info is returned as a Texture struct.
	std::vector<std::shared_ptr<Texture> > Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type)
	{
		std::vector<std::shared_ptr<Texture> > textures;
		for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			// Check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
			GLboolean skip = false;
			for (GLuint j = 0; j < _textures.size(); j++)
			{
				if (_textures[j]->getFileName() == str.C_Str())
				{
					textures.push_back(_textures[j]);
					skip = true; // A texture with the same filepath has already been loaded, continue to next one. (optimization)
					break;
				}
			}
			if (!skip)
			{   // If texture hasn't been loaded already, load it
				std::shared_ptr<Texture> texture = Texture::create2DTextureFromFile(str.C_Str());

				texture->setTexParameteri(GL_TEXTURE_WRAP_S, GL_REPEAT);
				texture->setTexParameteri(GL_TEXTURE_WRAP_T, GL_REPEAT);
				texture->setTexParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				texture->setTexParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				textures.push_back(texture);
				this->_textures.push_back(texture);  // Store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
			}
		}
		return textures;
	}

    void Model::setMaterialColor(const glm::vec4 &color){
        _materialColor = color;
        for(int i=0; i < _meshes.size(); i++){
            _meshes[i]->setMaterialColor(color);
        }
    }

}