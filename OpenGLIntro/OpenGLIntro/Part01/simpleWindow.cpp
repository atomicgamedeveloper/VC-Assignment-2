/*
 * SimpleWindow.cpp
 *
 * Adapted from http://www.opengl-tutorial.org
 * by Stefanie Zollmann
 * 
 * Simple Demonstration for getting started with opening a simple window that does not nothing but is prepared for displaying output using OpenGL
 *
 */


/* ------------------------------------------------------------------------- */
/* ---- INCLUDES ----------------------------------------------------------- */
/*
 * Include standard headers
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>


/* 
 * Include Glad
 */
 
#define GLAD_GL_IMPLEMENTATION
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
/*
 * Include GLFW 
 * Multi-platform library for creating windows, contexts and surfaces, receiving input and events.
 */
#include <GLFW/glfw3.h>
GLFWwindow* window;

/* 
 *Include GLM - OpenGL Maths
 */
#include <glm/glm.hpp>
using namespace glm;


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
    window = glfwCreateWindow( 1024*2, 768/2, windowName.c_str(), NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. .\n" );
        getchar();
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    
    return true;
    
}

/*
 *  renderLoop
 *
 *  This function is doing the actual render work 
 *  In this example it does not nothing but only clearing the screen
 *  In the next examples we will see how the rendering will be done here.
 */
void renderLoop(){
    
   // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && glfwWindowShouldClose(window) == 0 ){
        // Clear the screen.
        glClear( GL_COLOR_BUFFER_BIT );
            
        // Draw nothing so far, leave this open for next part of lab!
            
        // Swap buffers
        glfwSwapBuffers(window);
        //as for events
        glfwPollEvents();
        
    }
}


/*
 *  main
 *
 *  This is the main function that does all the work.
 *  Creates the window and than calls renderLoop.
 */
int main( void )
{
	//Init a basic window
    bool windowInitSucess = initWindow("Part01");
    if(!windowInitSucess){
        return -1;
    }

    int version = gladLoadGL();
    if (version == 0) {
        printf("Failed to initialize OpenGL context\n");
        return -1;
    }
    

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.5f, 0.0f);
    
    // Do the actual rendering work
    renderLoop();


	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

