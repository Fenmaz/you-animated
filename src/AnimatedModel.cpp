//
//  AnimatedModel.cpp
//
//  Created by Trung Nguyen on 12/12/2018.
//

#include "AnimatedModel.h"
#include "App.hpp"

#include "glm/ext.hpp"


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

AnimatedModel::AnimatedModel(const std::string &filename, const double scale, glm::vec4 materialColor): _materialColor(materialColor)
{
    //TODO not entirely sure this is threadsafe, although assimp says the library is as long as you have separate importer objects
    Assimp::Logger::LogSeverity severity = Assimp::Logger::NORMAL;
    // Create a logger instance for Console Output
    Assimp::DefaultLogger::create("", severity, aiDefaultLogStream_STDOUT);

    int numIndices = 0;
    
    importMesh(filename, numIndices, scale);
    
}


AnimatedModel::~AnimatedModel()
{
    // Kill it after the work is done
    _importer->FreeScene();
    Assimp::DefaultLogger::kill();
}

void AnimatedModel::draw(basicgraphics::GLSLProgram &shader) {
    for (int i = 0; i < _meshes.size(); i++) {
        //cout<<"Drawing a mesh"<<endl;
        shader.setUniform("bones", _finalTransformation);
        _meshes[i]->draw(shader);
    }
}

void AnimatedModel::importMesh(const std::string &filename, int &numIndices, const double scale/*=1.0*/)
{
    if (_importer.get() == nullptr) {
        _importer.reset(new Assimp::Importer());
    }

    scene = _importer->ReadFile(filename, aiProcess_Triangulate);
    
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
    
    _globalInverseTransform = glm::inverse(aiMatrix4x4ToGlm(&scene->mRootNode->mTransformation));
    this->processNode(scene->mRootNode, scene, scaleMat);
}


// Processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
void AnimatedModel::processNode(aiNode* node, const aiScene* scene, const glm::mat4 scaleMat)
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

//Find the node animation that matches the node name
const aiNodeAnim* AnimatedModel::FindNodeAnim(const aiAnimation* pAnimation, const string NodeName){
    for (uint i = 0 ; i < pAnimation->mNumChannels ; i++) {
        const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];
        
        if (string(pNodeAnim->mNodeName.data) == NodeName) {
            return pNodeAnim;
        }
    }
    
    return NULL;
}



//Recursively loop through the nodes of the scene graph to calculate the animation and pass to the children of that node
void AnimatedModel::ReadNodeHeirarchy(float AnimationTime, aiNode* node, const aiScene* scene, const glm::mat4& ParentTransform){
    
    string NodeName(node->mName.data);
    
    const aiAnimation* pAnimation = scene->mAnimations[0];
    
    mat4 NodeTransformation(aiMatrix4x4ToGlm(&(node->mTransformation)));
    
    //Find animation for a given node from name and aiAnimation of scene
    //Eventually, change this to a map for more efficient animation lookup
    const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnimation, NodeName);
    
    //calculate scaling, rotation, and tranlation matricies from node animation and time
    if(pNodeAnim){
        aiVector3D Scaling;
        CalcInterpolatedScaling(Scaling, AnimationTime, pNodeAnim);
        mat4 ScalingM = glm::scale(glm::mat4(1.0f), glm::vec3(Scaling.x, Scaling.y, Scaling.z));
        
        aiQuaternion RotationQ;
        CalcInterpolatedRotation(RotationQ, AnimationTime, pNodeAnim);
        aiMatrix3x3 rotQM = RotationQ.GetMatrix();
        mat4 RotationM = aiMatrix3x3ToGlm(&(rotQM));
        
        aiVector3D Translation;
        CalcInterpolatedPosition(Translation, AnimationTime, pNodeAnim);
        mat4 TranslationM= glm::translate(glm::mat4(1.0f), glm::vec3(Translation.x, Translation.y, Translation.z));
        
        //Combine all three into transformation matrix of node
        NodeTransformation = TranslationM * RotationM * ScalingM;
    }
    
    //Multiply node transformation by parent to get resulting transform
    mat4 GlobalTransformation = ParentTransform * NodeTransformation;
    
    //Set bone transformation from node
    if(_boneMapping.find(NodeName) != _boneMapping.end()){
        uint BoneIndex = _boneMapping[NodeName];
        _boneInfo[BoneIndex].FinalTransformation = _globalInverseTransform * GlobalTransformation * _boneInfo[BoneIndex].BoneOffset;
        _finalTransformation[BoneIndex] = _globalInverseTransform * GlobalTransformation * _boneOffset[BoneIndex];
        //std::cout << glm::to_string(_finalTransformation[BoneIndex]) << std::endl;
    }
    
    //Pass tranformation to children with recursive call
    for(uint i = 0; i < node->mNumChildren; i++){
        ReadNodeHeirarchy(AnimationTime, node->mChildren[i], scene, GlobalTransformation);
    }
}


std::shared_ptr<BoneMesh> AnimatedModel::processMesh(aiMesh* mesh, const aiScene* scene, const glm::mat4 scaleMat)
{
    // Data to fill
    std::vector<BoneMesh::Vertex> cpuVertexArray;
    std::vector<int> cpuIndexArray;
    std::vector<std::shared_ptr<basicgraphics::Texture>> textures;

    // Walk through each of the mesh's vertices
    for (GLuint i = 0; i < mesh->mNumVertices; i++)
    {
        BoneMesh::Vertex vertex;

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
    
    
    for (uint i = 0 ; i < mesh->mNumBones ; i++) {
        int boneIndex = 0;
        std::string boneName(mesh->mBones[i]->mName.data);
        
        if (_boneMapping.find(boneName) == _boneMapping.end()) {
            //std::cout << _numBones << std::endl;
            boneIndex = _numBones;
            _boneMapping[boneName] = boneIndex;
            
            BoneInfo bi;
            bi.BoneOffset = aiMatrix4x4ToGlm(&mesh->mBones[i]->mOffsetMatrix);
            _boneInfo.push_back(bi);
            
            _boneOffset[boneIndex] = aiMatrix4x4ToGlm(&mesh->mBones[i]->mOffsetMatrix);
            _finalTransformation[boneIndex] = glm::mat4(1.0);
            
            //std::cout << glm::to_string(_boneOffset[boneIndex]) << std::endl;
            
            _numBones++;
        }
        else {
            boneIndex = _boneMapping[boneName];
        }
        
        for (uint j = 0 ; j < mesh->mBones[i]->mNumWeights ; j++) {
            uint vertexID = mesh->mBones[i]->mWeights[j].mVertexId;
            float weight = mesh->mBones[i]->mWeights[j].mWeight;
            cpuVertexArray[vertexID].AddBoneData(boneIndex, weight);
        }
    }


    // Process the index array
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

        std::vector<std::shared_ptr<basicgraphics::Texture> > diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE);
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    }

    const int numVertices = cpuVertexArray.size();
    const int cpuVertexByteSize = sizeof(BoneMesh::Vertex) * numVertices;
    const int cpuIndexByteSize = sizeof(int) * cpuIndexArray.size();
    
    std::shared_ptr<BoneMesh> gpuMesh(new BoneMesh(textures, GL_TRIANGLES, GL_STATIC_DRAW, cpuVertexByteSize, cpuIndexByteSize, 0, cpuVertexArray, cpuIndexArray.size(), cpuIndexByteSize, &cpuIndexArray[0]));
    
    gpuMesh->setMaterialColor(_materialColor);
    return gpuMesh;
}

void AnimatedModel::boneTransform(float timeInSecs, std::vector<glm::mat4> &transforms)
{
    
    
    float ticksPerSec = scene->mAnimations[0]->mTicksPerSecond != 0 ? scene->mAnimations[0]->mTicksPerSecond : 25.0f;
    float timeInTicks = timeInSecs * ticksPerSec;
    float animationTime = fmod(timeInTicks, scene->mAnimations[0]->mDuration);
    
    ReadNodeHeirarchy(animationTime, scene->mRootNode , scene, glm::mat4(1.0f));
    
    transforms.resize(_numBones);
    
    for (uint i = 0; i < _numBones; i++) {
        transforms[i] = _boneInfo[i].FinalTransformation;
        //transform[i] = _finalTransformation[i];
    }
}

void AnimatedModel::CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    if (pNodeAnim->mNumPositionKeys == 1) {
        Out = pNodeAnim->mPositionKeys[0].mValue;
        return;
    }
    
    //find animation closest to current time, and next animation
    uint PositionIndex = FindPosition(AnimationTime, pNodeAnim);
    uint NextPositionIndex = (PositionIndex + 1);
    
    //make sure there is a next animation, and calculate time difference
    assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
    float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
    
    //Calculate how far along we are to the next animation (btw 0 and 1)
    float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    
    //Combine into a tranformation that is between start and end
    const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
    const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
    aiVector3D Delta = End - Start;
    Out = Start + Factor * Delta;
}

void AnimatedModel::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    // we need at least two values to interpolate...
    if (pNodeAnim->mNumRotationKeys == 1) {
        Out = pNodeAnim->mRotationKeys[0].mValue;
        return;
    }
    
    //find animation closest to current time, and next animation
    uint RotationIndex = FindRotation(AnimationTime, pNodeAnim);
    uint NextRotationIndex = (RotationIndex + 1);
    
    //make sure there is a next animation, and calculate time difference
    assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
    float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
    
    //Calculate how far along we are to the next animation (btw 0 and 1)
    float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    
    //Combine into a tranformation that is between start and end
    const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
    const aiQuaternion& EndRotationQ   = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
    aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
    Out = Out.Normalize();
}

void AnimatedModel::CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    if (pNodeAnim->mNumScalingKeys == 1) {
        Out = pNodeAnim->mScalingKeys[0].mValue;
        return;
    }
    
    //find animation closest to current time, and next animation
    uint ScalingIndex = FindScaling(AnimationTime, pNodeAnim);
    uint NextScalingIndex = (ScalingIndex + 1);
    
    //make sure there is a next animation, and calculate time difference
    assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
    float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
    
    //Calculate how far along we are to the next animation (btw 0 and 1)
    float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    
    //Combine into a tranformation that is between start and end
    const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
    const aiVector3D& End   = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
    aiVector3D Delta = End - Start;
    Out = Start + Factor * Delta;
}

//Find closest translation animation to a given animation time
uint AnimatedModel::FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    for (uint i = 0 ; i < pNodeAnim->mNumPositionKeys - 1 ; i++) {
        if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
            return i;
        }
    }
    
    assert(0);
    
    return 0;
}

//Find closest rotation animation to a given animation time
uint AnimatedModel::FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    assert(pNodeAnim->mNumRotationKeys > 0);
    
    for (uint i = 0 ; i < pNodeAnim->mNumRotationKeys - 1 ; i++) {
        if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
            return i;
        }
    }
    
    assert(0);
    
    return 0;
}

//Find closest scaling animation to a given animation time
uint AnimatedModel::FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    assert(pNodeAnim->mNumScalingKeys > 0);
    
    for (uint i = 0 ; i < pNodeAnim->mNumScalingKeys - 1 ; i++) {
        if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
            return i;
        }
    }
    
    assert(0);
    
    return 0;
}

// Checks all material textures of a given type and loads the textures if they're not loaded yet.
// The required info is returned as a Texture struct.
std::vector<std::shared_ptr<basicgraphics::Texture> > AnimatedModel::loadMaterialTextures(aiMaterial* mat, aiTextureType type)
{
    std::vector<std::shared_ptr<basicgraphics::Texture> > textures;
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
            std::shared_ptr<basicgraphics::Texture> texture = basicgraphics::Texture::create2DTextureFromFile(str.C_Str());

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


//Set color of model based on given color
void AnimatedModel::setMaterialColor(const glm::vec4 &color){
    _materialColor = color;
    for(int i=0; i < _meshes.size(); i++){
        _meshes[i]->setMaterialColor(color);
    }
}
