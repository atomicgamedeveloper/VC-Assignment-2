#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include <glad/glad.h>
#define GLAD_GL_IMPLEMENTATION

#include <GLFW/glfw3.h>
GLFWwindow* window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <opencv2/opencv.hpp>

#include <common/Shader.hpp>
#include <common/Camera.hpp>
#include <common/Scene.hpp>
#include <common/Object.hpp>
#include <common/TextureShader.hpp>
#include <common/Quad.hpp>
#include <common/Texture.hpp>

using namespace std;

// Helper function to initialize the window
bool initWindow(std::string windowName);

/* ------------------------------------------------------------------------- */
/* main                                                                      */
/* ------------------------------------------------------------------------- */
int main(void) {
    // --- Step 1: Open camera (OpenCV part - already complete) -----------
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "Error: Could not open camera. Exiting." << endl;
        return -1;
    }
    std::cout << "Camera opened successfully." << endl;

    // --- Step 2: Initialize OpenGL context (GLFW & GLAD - already complete) ---
    if (!initWindow("OpenCV to OpenGL Exercise")) return -1;

    int version = gladLoadGL();
    if (version == 0) {
        fprintf(stderr, "Failed to initialize OpenGL context (GLAD)\n");
        cap.release();
        return -1;
    }
    int major = version >> 16;
    int minor = version & 0xFFFF;
    std::cout << "Loaded OpenGL " << major << "." << minor << "\n";

    // Basic OpenGL setup
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glClearColor(0.1f, 0.1f, 0.2f, 0.0f); // A dark blue background
    glEnable(GL_DEPTH_TEST);

    // --- Step 2½: create and bind a Vertex Array Object -----------------
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    // --------------------------------------------------------------------
    // --- Step 3: Prepare Scene, Shaders, and Objects ---------------------
    
    // We get one frame from the camera to determine its size.
    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) {
        cerr << "Error: couldn't capture an initial frame from camera. Exiting.\n";
        cap.release();
        glfwTerminate();
        return -1;
    }

    // Create objects needed for rendering.
    TextureShader* textureShader = new TextureShader("videoTextureShader.vert", "videoTextureShader.frag");
    Scene* myScene = new Scene();
    Camera* renderingCamera = new Camera();
    renderingCamera->setPosition(glm::vec3(0, 0, -2.5)); // Move camera back to see the quad

    // Calculate aspect ratio and create a quad with the correct dimensions.
    float videoAspectRatio = (float)frame.cols / (float)frame.rows*1.2;
    Quad* myQuad = new Quad(videoAspectRatio);
    myQuad->setShader(textureShader);
    myScene->addObject(myQuad);
    
    // This variable will hold our OpenGL texture.
    Texture* videoTexture = nullptr;

    // --- TODO: CREATE THE TEXTURE ---
    // GOAL: Create an OpenGL texture from the first camera `frame`.
    // HINT: The `Texture` class constructor is: Texture(frame.data, frame.cols, frame.rows, true)
    // HINT: Remember to flip the frame vertically for OpenGL's coordinate system BEFORE creating the texture.
    
    //flip(frame, frame, 1);
    //--- TODO: Flip the image on the x-axis
    videoTexture = new Texture(frame.data,
        frame.cols,
        frame.rows,
        true);  // <-- UNCOMMENT AND COMPLETE THIS LINE
    
    
    // We must tell the shader which texture to use.
    textureShader->setTexture(videoTexture); // <-- UNCOMMENT THIS LINE

    // --- Step 4: Main Render Loop (Completed for you) ---------------------
    while (!glfwWindowShouldClose(window)) {
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Check for ESC key press
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // --- Part 1: Update the texture with a new frame ---
        cap >> frame;
        if (!frame.empty() && videoTexture != nullptr) {
            //--- TODO: Flip the new frame and update the texture data on the GPU
            flip(frame, frame, 0);
            videoTexture->update(frame.data, frame.cols, frame.rows, true);
        }

        // --- Part 2: Render the scene ---
        // This tells the scene to draw all its objects from the camera's perspective.
        // --- Part 2: Render the scene ---
        myScene->render(renderingCamera);


        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- Cleanup -----------------------------------------------------------
    std::cout << "Closing application..." << endl;
    cap.release();
    delete myScene;
    delete renderingCamera;
    delete textureShader;
    delete videoTexture;

    glfwTerminate();
    return 0;
}

/* ------------------------------------------------------------------------- */
/* Helper: initWindow (GLFW) - No changes needed here                        */
/* ------------------------------------------------------------------------- */
bool initWindow(std::string windowName){
    if (!glfwInit()){
        fprintf(stderr, "Failed to initialize GLFW\n");
        return false;
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(1024, 768, windowName.c_str(), NULL, NULL);
    if (window == NULL){
        fprintf(stderr, "Failed to open GLFW window.\n");
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    return true;
}

