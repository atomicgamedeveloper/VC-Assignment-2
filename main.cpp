#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <stdio.h>
#include <windows.h>
#include <opencv2/core/utils/logger.hpp> 
#include <stdlib.h>
#include <string>

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
using namespace cv;
const char* window_name = "Live Camera Project";

int lowThreshold = 0;
const int max_lowThreshold = 100;
const int _ratio = 3;
const int kernel_size = 3;

// Helper function to initialize the window
bool initWindow(std::string windowName, float aspectRatio);

int main(int argc, char** argv) {
    utils::logging::setLogLevel(utils::logging::LOG_LEVEL_SILENT);

    Mat frame;

    VideoCapture cap;

    int deviceID = 0;
    int apiID = cv::CAP_ANY;

    cap.open(deviceID, apiID);

    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open camera\n";
    }

    // We get one frame from the camera to determine its size.
    cap.read(frame); // cap >> frame
    flip(frame, frame, 0);
    if (frame.empty()) {
        cerr << "Error: couldn't capture an initial frame from camera. Exiting.\n";
        cap.release();
        glfwTerminate();
        return -1;
    }
    float videoAspectRatio = (float)frame.cols / (float)frame.rows;

    // --- Initialize OpenGL context (GLFW & GLAD - already complete) ---
    if (!initWindow("OpenCV to OpenGL Exercise",videoAspectRatio)) return -1;

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }
    // Basic OpenGL setup
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glClearColor(0.1f, 0.1f, 0.2f, 0.0f); // A dark blue background
    glEnable(GL_DEPTH_TEST);

    // Make our vertex array object
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create objects needed for rendering.
    TextureShader* textureShader = new TextureShader("videoTextureShader.vert", "videoTextureShader.frag");
    Scene* myScene = new Scene();
    
    glm::mat4 orthoProjection = glm::ortho(
        -videoAspectRatio, videoAspectRatio,   // left, right  (preserve webcam ratio!)
        -1.0f, 1.0f,                            // bottom, top
        0.1f, 100.0f                            // near, far
    );

    glm::mat4 viewMatrix = glm::lookAt(
        glm::vec3(0, 0, 5.0f),  // position
        glm::vec3(0, 0, 0.0f),  // look at center
        glm::vec3(0, 1, 0.0f)   // up vector
    );

    Camera* renderingCamera = new Camera(orthoProjection, viewMatrix);

    // Calculate aspect ratio and create a quad with the correct dimensions.
    Quad* myQuad = new Quad(videoAspectRatio);
    myQuad->setShader(textureShader);
    myScene->addObject(myQuad);

    // This variable will hold our OpenGL texture.
    Texture* videoTexture = nullptr;

    // --- TODO: CREATE THE TEXTURE ---
    videoTexture = new Texture(frame.data,
        frame.cols,
        frame.rows,
        true);

    textureShader->setTexture(videoTexture);

    cout << "Start grabbing" << endl
        << "Press any key to terminate" << endl;

    vector<string> labels{"None", "Grey", "Gaussian", "Canny", "Median Blur" };
    cout << "\n" << labels.at(0) << endl;

    int mode = -1;
    bool hasChanged = 0;
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Check for ESC key press
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        cap.read(frame);

        bool left = GetAsyncKeyState(37) & 0x8000;
        bool right = GetAsyncKeyState(39) & 0x8000;
        if (left && mode > -1 && hasChanged == 0) {
            mode--;
            hasChanged = 1;
            cout << labels.at(mode + 1) << endl;
        }

        if (right && mode < 3 && hasChanged == 0) {
            mode++;
            hasChanged = 1;
            cout << labels.at(mode + 1) << endl;
        }

        if (mode == 0) {
            cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
            cv::cvtColor(frame, frame, cv::COLOR_GRAY2BGR);
        }
        else if (mode == 1) {
            cv::GaussianBlur(frame, frame, cv::Size(5, 5), 1.5);
        }
        else if (mode == 2) {
            cv::Mat tmp;
            cv::cvtColor(frame, tmp, cv::COLOR_BGR2GRAY);
            cv::Canny(tmp, tmp, 60, 180);
            cv::cvtColor(tmp, frame, cv::COLOR_GRAY2BGR);
        }
        else if (mode == 3) {
            cv::medianBlur(frame, frame, 15);
        }

        if (!frame.empty() && videoTexture != nullptr) {
            flip(frame, frame, 0);
            videoTexture->update(frame.data, frame.cols, frame.rows, true);
        }

        myScene->render(renderingCamera);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();

        if (!left && !right) {
            hasChanged = 0;
        }
        if (waitKey(5) == 113) {
            break;
        }
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
    return 0;
}

/* ------------------------------------------------------------------------- */
/* Helper: initWindow (GLFW) - No changes needed here                        */
/* ------------------------------------------------------------------------- */
bool initWindow(std::string windowName, float aspectRatio) {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return false;
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(720 * aspectRatio, 720, windowName.c_str(), NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window.\n");
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    return true;
}
