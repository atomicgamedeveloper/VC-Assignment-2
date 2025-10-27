
/*
 * redTriangle.cpp
 *
 * Adapted from http://opengl-tutorial.org
 * by Stefanie Zollmann
 *
 * Simple Demonstration for getting started with opening a simple window that 
 * does not nothing but rendering a red triangle
 *
 */


/* ------------------------------------------------------------------------- */
/* ---- INCLUDES ----------------------------------------------------------- */
/*
 * Include standard headers
 */

#include <fstream>        // NEW  – for diagnostic
#include <cstring>        // NEW  – for strerror

#include <glm/gtx/component_wise.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include <iostream>
using namespace std;
#include <glad/glad.h>

#define GLAD_GL_IMPLEMENTATION 

/*
 * Include GLFW
 * Multi-platform library for creating windows, contexts and surfaces, receiving input and events.
 */
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
// Include GLM
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
using namespace glm;

#include <common/Shader.hpp>
#include <common/Camera.hpp>
#include <common/Scene.hpp>
#include <common/Object.hpp>
#include <common/Triangle.hpp>
#include <common/ColorShader.hpp>

#include <common/Quad.hpp>

/* ---- Helper Functions  ------------------------------------------------------- */

/*
 *  initWindow
 *
 *  This is used to set up a simple window using GLFW.
 *  Returns true if sucessful otherwise false.
 */
bool initWindow(std::string windowName){
    
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return false;
    }
    
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Open a window and create its OpenGL context
    window = glfwCreateWindow( 1024, 768, windowName.c_str(), NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible.\n" );
        getchar();
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    
    return true;
    
}

/*
 *  main
 *
 *  This is the main function that does all the work.
 *  Creates the window and than calls renderLoop.
 */
int main( void )
{


    if (__cplusplus == 202002L) std::cout << "C++20\n";
    else if (__cplusplus == 201703L) std::cout << "C++17\n";
    else if (__cplusplus == 201402L) std::cout << "C++14\n";
    else if (__cplusplus == 201103L) std::cout << "C++11\n";
    else if (__cplusplus == 199711L) std::cout << "C++98\n";
    else std::cout << "pre-standard C++\n";


    initWindow("Shader LAB Part 01");
	glfwMakeContextCurrent(window);

	// Initialize glad
	int version = gladLoadGL();
    if (version == 0) {
        printf("Failed to initialize OpenGL context\n");
        return -1;
    }

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Dark blue background
	glClearColor(0.1f, 0.0f, 0.4f, 0.0f);

    
    //create a Vertex Array Object and set it as the current one
    //we will not go into detail here. but this can be used to optimise the performance by storing all of the state needed to supply vertex data
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

    // create a triangle and set a shader
    Triangle* myTriangle = new Triangle();

    Triangle* myOtherTriangle = new Triangle();

    Quad* myQuad = new Quad();

    Quad* myOtherQuad = new Quad();

     //the basic vertex shader transforms the vertices using the Model-View Projection matrix
	// and the basic fragment shader outputs a predefined color
    Shader* shader = new Shader( "basicShader");
    
    /* ----  DEBUG: show the exact path we are going to open ---- */
    std::string vName = std::string(SHADER_DIR) + "simpleColor.vert";
    std::cout << "Trying to open: >>>" << vName << "<<<" << std::endl;
    std::ifstream test(vName.c_str());
    if (!test)
        std::cerr << "Failed - errno " << strerror(errno) << std::endl;
    else
        std::cout << "Found simpleColor.vert OK" << std::endl;
    test.close();
    /* ---------------------------------------------------------- */

    ColorShader* shader1 = new ColorShader(SHADER_DIR "simpleColor");
    shader1->setColor(glm::vec4(1.0, 0.0, 1.0, 1.0));
    myTriangle->setShader(shader1);
    myTriangle->setTranslate(glm::vec3(1, 0, 0));
    //myTriangle->setShader(shader);

    ColorShader* shaderOther1 = new ColorShader(SHADER_DIR "simpleColor");
    shaderOther1->setColor(glm::vec4(0.5, 0.0, 0.0, 1.0));
    myOtherTriangle->setShader(shaderOther1);
    myOtherTriangle->setTranslate(glm::vec3(3, 1, 0));
    //myTriangle->setShader(shader);

    ColorShader* shader2 = new ColorShader(SHADER_DIR "simpleColor");
    shader2->setColor(glm::vec4(0.0,0.0,1.0, 1.0));
    myQuad->setShader(shader2);

    ColorShader* shaderOther2 = new ColorShader(SHADER_DIR "simpleColor");
    shaderOther2->setColor(glm::vec4(0.0,1.0,0.0, 0.3));
    myOtherQuad->setShader(shaderOther2);
    myOtherQuad->setTranslate(glm::vec3(2, 2, 2));

    Scene* myScene = new Scene();
    myScene->addObject(myTriangle);
    myScene->addObject(myQuad);
    myScene->addObject(myOtherTriangle);
    myScene->addObject(myOtherQuad);

    Camera* myCamera = new Camera();
    //place the camera at z= -5
	myCamera->setPosition(glm::vec3(0,0,-5));
    
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && glfwWindowShouldClose(window) == 0 ){// Clear the screen
		
		
		glClear( GL_COLOR_BUFFER_BIT );

		myScene->render(myCamera); // will render all the objects that are part of the scene
		
        glDisableVertexAttribArray(0);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

    }
    

    glDeleteVertexArrays(1, &VertexArrayID);
	
    delete myScene;
    delete myCamera;

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

