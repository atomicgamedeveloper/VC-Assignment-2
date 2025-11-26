/*
* AppUtilities.cpp
* Common utility functions for OpenGL and OpenCV applications
*/
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

GLFWwindow* window = nullptr;
int BASE_WIDTH = 720;

bool initWindow(const std::string& windowName, float aspectRatio) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return false;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(
        BASE_WIDTH * aspectRatio,
        BASE_WIDTH,
        windowName.c_str(),
        NULL,
        NULL
    );

    if (window == NULL) {
        std::cerr << "Failed to open GLFW window.\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    return true;
}