///
///  Model.hpp
///
///
///  Created by Bret Jackson on 2/2/17.
///
///  \brief Model is used to load a 3d model file from disk. It will automatically create mesh objects uploaded to VBOs
///

#ifndef Model_hpp
#define Model_hpp

#include <iostream>
#include <iomanip>
#include <memory>
#include <string>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/ProgressHandler.hpp>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "Mesh.h"
#include "Texture.h"
#include "GLSLProgram.h"

namespace basicgraphics {

	typedef std::shared_ptr<class Importer> ImporterRef;

	class ProgressReporter : public Assimp::ProgressHandler
	{
	public:
		ProgressReporter();
		~ProgressReporter();
		bool Update(float percentage = -1.f);
		void reset();
	private:
		bool _firstUpdate;
	};

	class Model : public std::enable_shared_from_this<Model>
	{
	public:

		/*!
		 * Tries to load a model from disk. Scale can be used to scale the vertex locations of the model. If the model contains textures than materialColor will be ignored.
		 */
        Model(const std::string &filename, const double scale, glm::vec4 materialColor = glm::vec4(1.0));

		virtual ~Model();

		virtual void draw(GLSLProgram &shader);
        
        void setMaterialColor(const glm::vec4 &color);
        
        struct BoneInfo {
            glm::mat4 BoneOffset;
            glm::mat4 FinalTransformation;
            
            BoneInfo() {
                BoneOffset = glm::mat4(0.0);
                FinalTransformation = glm::mat4(0.0);
            }
        };
        
        void boneTransform(float timeInSecs, std::vector<glm::mat4> &transforms, const aiScene* scene);
        void printBoneName(float index);

	private:

		glm::vec4 _materialColor;

		std::unique_ptr<Assimp::Importer> _importer;
		//std::unique_ptr<ProgressReporter> _reporter;
		std::vector< std::shared_ptr<Mesh> > _meshes;
		std::vector< std::shared_ptr<Texture> > _textures;
        
        std::map<string, uint> _boneMapping;
        std::vector<BoneInfo> _boneInfo;
        uint _numBones;
        
        glm::mat4 _globalInverseTransform;

		void importMesh(const std::string &filename, int &numIndices, const double scale);
		void processNode(aiNode* node, const aiScene* scene, const glm::mat4 scaleMat);
        void ReadNodeHeirarchy(float AnimationTime, aiNode* node, const aiScene *scene, const glm::mat4& ParentTransform);
        const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const string NodeName);

		std::shared_ptr<Mesh> processMesh(aiMesh* mesh, const aiScene* scene, const glm::mat4 scaleMat);
        
        void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
        void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
        void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
        uint FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
        uint FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
        uint FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);
        
		std::vector<std::shared_ptr<Texture> > loadMaterialTextures(aiMaterial* mat, aiTextureType type);
	};

}

inline glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4* from)
{
    glm::mat4 to;
    
    
    to[0][0] = (GLfloat)from->a1; to[0][1] = (GLfloat)from->b1;  to[0][2] = (GLfloat)from->c1; to[0][3] = (GLfloat)from->d1;
    to[1][0] = (GLfloat)from->a2; to[1][1] = (GLfloat)from->b2;  to[1][2] = (GLfloat)from->c2; to[1][3] = (GLfloat)from->d2;
    to[2][0] = (GLfloat)from->a3; to[2][1] = (GLfloat)from->b3;  to[2][2] = (GLfloat)from->c3; to[2][3] = (GLfloat)from->d3;
    to[3][0] = (GLfloat)from->a4; to[3][1] = (GLfloat)from->b4;  to[3][2] = (GLfloat)from->c4; to[3][3] = (GLfloat)from->d4;
    
    return to;
}

inline glm::mat4 aiMatrix3x3ToGlm(const aiMatrix3x3* from)
{
    glm::mat4 to;
    
    
    to[0][0] = (GLfloat)from->a1; to[0][1] = (GLfloat)from->b1;  to[0][2] = (GLfloat)from->c1; to[0][3] = 0.0f;
    to[1][0] = (GLfloat)from->a2; to[1][1] = (GLfloat)from->b2;  to[1][2] = (GLfloat)from->c2; to[1][3] = 0.0f;
    to[2][0] = (GLfloat)from->a3; to[2][1] = (GLfloat)from->b3;  to[2][2] = (GLfloat)from->c3; to[2][3] = 0.0f;
    to[3][0] = 0.0f;              to[3][1] = 0.0f;               to[3][2] = 0.0f;              to[3][3] = 1.0f;
    
    return to;
}

#endif /* Model_hpp */
