//
//  Created by Mark Coretsopoulos on 10/26/18.
//

#include "App.hpp"

#define FONTSTASH_IMPLEMENTATION
#include <fontstash.h>
#define GLFONTSTASH_IMPLEMENTATION
#include <glfontstash.h>

#include <config/VRDataIndex.h>
#include <main/VRSystem.h>

#include <iostream>
using namespace std;
using namespace glm;


App::App(int argc, char** argv) : VRApp(argc, argv) {
    _startTime = VRSystem::getTime();
}

App::~App()
{
    shutdown();
}

void App::onAnalogChange(const VRAnalogEvent &event) {
}


void App::onButtonDown(const VRButtonEvent &event) {
    // This routine is called for all Button_Down events.  Check event->getName()
    // to see exactly which button has been pressed down.
}

void App::onButtonUp(const VRButtonEvent &event) {
    // This routine is called for all Button_Up events.  Check event->getName()
    // to see exactly which button has been pressed down.
}

void App::onCursorMove(const VRCursorEvent &event){
    // This routine is called for all mouse move events. You can get the absolute position
    // or the relative position within the window scaled 0--1.
}

void App::onRenderGraphicsContext(const VRGraphicsState &renderState){
    // This routine is called once per graphics context at the start of the
    // rendering process.  So, this is the place to initialize textures,
    // load models, or do other operations that you only want to do once per
    // frame.
    
    // Is this the first frame that we are rendering after starting the app?
    if (renderState.isInitialRenderCall()) {
        //For windows, we need to initialize a few more things for it to recognize all of the
        // opengl calls.
#ifndef __APPLE__
        glewExperimental = GL_TRUE;
        GLenum err = glewInit();
        if (GLEW_OK != err)
        {
            std::cout << "Error initializing GLEW." << std::endl;
        }
#endif
        
        glEnable(GL_DEPTH_TEST);
        glClearDepth(1.0f);
        glDepthFunc(GL_LEQUAL);
        
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        
        glEnable(GL_MULTISAMPLE);
        
        // This sets the background color that is used to clear the canvas
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        
        // This load shaders from disk, we do it once when the program starts up.
        reloadShaders();
        
        //import a new model to use in the program
        _modelMesh.reset(new AnimatedModel("free3Dmodel.dae", 1.0, vec4(1.0)));
    }
}

void App::onRenderGraphicsScene(const VRGraphicsState &renderState){
    // This routine is called once per eye/camera.  This is the place to actually
    // draw the scene.
    
    // clear the canvas and other buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    // Setup the camera with a good initial position and view direction to see the model
    glm::vec3 eye_world = glm::vec3(20, 12, 5);
    glm::vec3 center (0, 0, 0);
    
    glm::mat4 view = glm::lookAt(eye_world, center, glm::vec3(0, 1, 0));
    
    GLfloat windowHeight = renderState.index().getValue("FramebufferHeight");
    GLfloat windowWidth = renderState.index().getValue("FramebufferWidth");
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), windowWidth / windowHeight, 0.01f, 500.0f);
    
    // Set up model matrix;
    glm::mat4 model = glm::mat4(1.0);
    
    
    // Update shader variables
    _shader.use();
    _shader.setUniform("view_mat", view);
    _shader.setUniform("projection_mat", projection);
    _shader.setUniform("model_mat", model);
    _shader.setUniform("normal_mat", mat3(transpose(inverse(model))));
    _shader.setUniform("eye_world", eye_world);
    
    float time = (float) (VRSystem::getTime() - _startTime);
    vector<glm::mat4> transforms;
    
    _modelMesh->boneTransform(time, transforms);
    //printf("%f\n", time);
    
    // Draw the model
    _modelMesh->draw(_shader);
    
}

void App::reloadShaders(){
    _shader.compileShader("vertex.glsl", basicgraphics::GLSLShader::VERTEX);
    _shader.compileShader("fragment.glsl", basicgraphics::GLSLShader::FRAGMENT);
    _shader.link();
    _shader.use();
}

