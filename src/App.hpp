//
//  Created by Mark Coretsopoulos on 10/26/18.
//

#ifndef App_hpp
#define App_hpp



#include <stdio.h>
#include <api/MinVR.h>
using namespace MinVR;

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

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

#include <BasicGraphics.h>
using namespace basicgraphics;

class App : public VRApp {
public:
    
    App(int argc, char** argv);
    virtual ~App();
    
    /** USER INTERFACE CALLBACKS **/
    virtual void onAnalogChange(const VRAnalogEvent &state) override;
    virtual void onButtonDown(const VRButtonEvent &state) override;
    virtual void onButtonUp(const VRButtonEvent &state) override;
    virtual void onCursorMove(const VRCursorEvent &state) override;
    
    
    /** RENDERING CALLBACKS **/
    virtual void onRenderGraphicsScene(const VRGraphicsState& state) override;
    virtual void onRenderGraphicsContext(const VRGraphicsState& state) override;
    
protected:
    
    double _lastTime;
    double _curFrameTime;
    
    virtual void reloadShaders();
    basicgraphics::GLSLProgram _shader;
    
    std::unique_ptr<Model> _modelMesh;

    
};


#endif /* App_hpp */
