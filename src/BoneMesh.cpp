//
//  BoneMesh.cpp
//
//  Created by Trung Nguyen 12/12/2018.
//
//

#include "BoneMesh.h"

#include "glm/ext.hpp"


BoneMesh::BoneMesh(std::vector<std::shared_ptr<basicgraphics::Texture> > textures, GLenum primitiveType, GLenum usage, int allocateVertexByteSize, int allocateIndexByteSize, int vertexOffset, const std::vector<Vertex> &data, int numIndices /*=0*/, int indexByteSize/*=0*/, int* index/*=nullptr*/)
{
    _textures = textures;
    
    _materialColor = glm::vec4(1.0);
    
    assert(data.size() - vertexOffset >= 0);
    int dataByteSize = sizeof(Vertex) * (data.size() - vertexOffset);
    
    _allocatedVertexByteSize = allocateVertexByteSize;
    _allocatedIndexByteSize = allocateIndexByteSize;
    _filledVertexByteSize = dataByteSize;
    _filledIndexByteSize = indexByteSize;
    _numIndices = numIndices;
    _primitiveType = primitiveType;
    
    // create the vao
    glGenVertexArrays(1, &_vaoID);
    glBindVertexArray(_vaoID);
    
    // create the vbo
    glGenBuffers(1, &_vertexVBO);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexVBO);
    
    // initialize size
    glBufferData(GL_ARRAY_BUFFER, allocateVertexByteSize, NULL, usage);
    
    if (dataByteSize > 0) {
        //buffer data
        glBufferSubData(GL_ARRAY_BUFFER, 0, dataByteSize, &data[0]);
    }
    
    // set up vertex attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord0));
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, IDs));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weights));
    
    // Create indexstream
    glGenBuffers(1, &_indexVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexVBO);
    
    // copy data into the buffer object
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, allocateIndexByteSize, NULL, usage);
    
    if (indexByteSize > 0 && index != nullptr) {
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indexByteSize, index);
    }
    
    // unbind the vao
    glBindVertexArray(0);
}

BoneMesh::~BoneMesh()
{
    //Assumes object is deleted with the correct context current
    glDeleteBuffers(1, &_vertexVBO);
    glDeleteBuffers(1, &_indexVBO);
    glDeleteVertexArrays(1, &_vaoID);
}

void BoneMesh::draw(basicgraphics::GLSLProgram &shader) {
    
    bool translucent = false;
    if (_textures.size() > 0) {
        //std::cout<<"Mesh has texture"<<std::endl;
        shader.setUniform("hasTexture", 1);
        shader.setUniform("materialColor", vec4(0.0, 0.0, 0.0, 1.0));
        
        for (int i = 0; i < _textures.size(); i++) {
            
            if (!_textures[i]->isOpaque()) {
                translucent = true;
                glDisable(GL_DEPTH_TEST);
                //Note: This isn't going to work properly because the surfaces are not sorted back to front. Transparent surfaces should be drawn after all the opaque geometry.
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
            _textures[i]->bind(i);
            shader.setUniform("textureSampler", i);
        }
    }
    else {
        //std::cout<<"Mesh does not have texture"<<std::endl;
        shader.setUniform("hasTexture", 0);
        shader.setUniform("materialColor", _materialColor);
        if (_materialColor.a != 1.0) {
            translucent = true;
            glDisable(GL_DEPTH_TEST);
            //Note: This isn't going to work properly because the surfaces are not sorted back to front. Transparent surfaces should be drawn after all the opaque geometry.
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
    }
    
    glBindVertexArray(this->getVAOID());
    glDrawElements(_primitiveType, _numIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    
    if (translucent) {
        glBlendFunc(GL_ONE, GL_ZERO);
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
    }
    
    // Reset state
    for (int i = 0; i < _textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void BoneMesh::setMaterialColor(const glm::vec4 &color)
{
    _materialColor = color;
}

int BoneMesh::getAllocatedVertexByteSize() const
{
    return _allocatedVertexByteSize;
}

int BoneMesh::getAllocatedIndexByteSize() const
{
    return _allocatedIndexByteSize;
}

int BoneMesh::getFilledVertexByteSize() const
{
    return _filledVertexByteSize;
}

int BoneMesh::getFilledIndexByteSize() const
{
    return _filledIndexByteSize;
}

int BoneMesh::getNumIndices() const
{
    return _numIndices;
}

GLuint BoneMesh::getVAOID() const
{
    return _vaoID;
}

void BoneMesh::updateVertexData(int startByteOffset, int vertexOffset, const std::vector<Vertex> &data)
{
    assert(startByteOffset <= _filledVertexByteSize);
    
    int dataByteSize = sizeof(Vertex)*((int)data.size() - vertexOffset);
    
    int totalBytes = startByteOffset + dataByteSize;
    if (_filledVertexByteSize < totalBytes) {
        _filledVertexByteSize = totalBytes;
    }
    
    assert(_filledVertexByteSize <= _allocatedVertexByteSize);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexVBO);
    
    glBufferSubData(GL_ARRAY_BUFFER, startByteOffset, dataByteSize, &data[0]);
}

void BoneMesh::updateIndexData(int totalNumIndices, int startByteOffset, int indexByteSize, int* index)
{
    assert(startByteOffset <= _filledIndexByteSize);
    _numIndices = totalNumIndices;
    int totalBytes = startByteOffset + indexByteSize;
    if (_filledIndexByteSize < totalBytes) {
        _filledIndexByteSize = totalBytes;
    }
    assert(_filledIndexByteSize <= _allocatedIndexByteSize);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexVBO);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, startByteOffset, indexByteSize, index);
}

//Adds bone data to a vertex. Looks for the next open slot on the VBO, and puts the boneID and weight in that slot
void BoneMesh::Vertex::AddBoneData(int BoneID, float Weight) {
    for (int i = 0; i < NUM_BONES_PER_VERTEX; i++) {
        if (weights[i] == 0) {
            IDs[i] = BoneID;
            weights[i] = Weight;
            
//            std::cout << std::endl << "Position: " << glm::to_string(position) << std::endl << "ID: ";
//            for (uint id: IDs) { std::cout << id << "\t"; };
//            std::cout << std::endl << "Weights: ";
//            float sum = 0;
//            for (float weight: weights) { std::cout << weight << "\t"; sum += weight; };
//            std::cout << std::endl << sum << std::endl;
            
            return;
        }
    }
    
    // more bones than we have space for
    assert(0);
}
