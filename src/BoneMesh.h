///
///  BoneMesh.h
///
///  \brief Allocates and stores a mesh with bone data in a VBO/VAO on the gpu.
///

#ifndef BoneMesh_hpp
#define BoneMesh_hpp

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
//#include <glad/glad.h>

#ifdef _WIN32
#include "GL/glew.h"
#include "GL/wglew.h"
#elif (!defined(__APPLE__))
#include "GL/glxew.h"
#endif

// OpenGL Headers
#if defined(WIN32)
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#elif defined(__APPLE__)
#define GL_GLEXT_PROTOTYPES
#include <OpenGL/gl3.h>
#include <OpenGL/glext.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#endif

#include "Texture.h"
#include "GLSLProgram.h"


class BoneMesh : public std::enable_shared_from_this<BoneMesh>
{
public:
    
    #define NUM_BONES_PER_VERTEX 8
    
    struct Vertex {        
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoord0;
        uint IDs[NUM_BONES_PER_VERTEX];
        float weights[NUM_BONES_PER_VERTEX];

        Vertex() {           
            position = glm::vec3(0.0);
            normal = glm::vec3(0.0);
            texCoord0 = glm::vec2(0.0);
            std::memset(IDs, 0, sizeof(uint) * NUM_BONES_PER_VERTEX);
            std::memset(weights, 0, sizeof(float) * NUM_BONES_PER_VERTEX);
        };
        
        void AddBoneData(int BoneID, float Weight);
    };
    
    
    // Creates a vao and vbo. Usage should be GL_STATIC_DRAW, GL_DYNAMIC_DRAW, etc. Leave data empty to just allocate but not upload.
    BoneMesh(std::vector<std::shared_ptr<basicgraphics::Texture> > textures, GLenum primitiveType, GLenum usage, int allocateVertexByteSize, int allocateIndexByteSize, int vertexOffset, const std::vector<Vertex> &data, int numIndices = 0, int indexByteSize = 0, int* index = nullptr);
    virtual ~BoneMesh();

    virtual void draw(basicgraphics::GLSLProgram &shader);
    
    void setMaterialColor(const glm::vec4 &color);
    
    // Returns the number of bytes allocated in the vertexVBO
    int getAllocatedVertexByteSize() const;
    int getAllocatedIndexByteSize() const;
    // Returns the number of bytes actually filled with data in the vertexVBO
    int getFilledVertexByteSize() const;
    int getFilledIndexByteSize() const;
    int getNumIndices() const;
    
    GLuint getVAOID() const;
    
    
    // Update the vbos. startByteOffset+dataByteSize must be <= allocatedByteSize
    void updateVertexData(int startByteOffset, int vertexOffset, const std::vector<Vertex> &data);
    void updateIndexData(int totalNumIndices, int startByteOffset, int indexByteSize, int* index);
    
private:
        
    GLuint _vaoID;
    GLuint _vertexVBO;
    GLuint _indexVBO;
    GLenum _primitiveType;
    
    int _allocatedVertexByteSize;
    int _allocatedIndexByteSize;
    int _filledVertexByteSize;
    int _filledIndexByteSize;
    int _numIndices;
    
    glm::vec4 _materialColor;
    
    std::vector<std::shared_ptr<basicgraphics::Texture> > _textures;
};

#endif /* BoneMesh_hpp */
