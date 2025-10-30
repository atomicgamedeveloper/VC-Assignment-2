#include <glad/glad.h>
#include <GLFW/glfw3.h>

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
#include <map>

#include "../hpp/Filters.hpp"
#include "../hpp/InputHandler.hpp"

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

int BASE_WIDTH = 720;

// Helper function to initialize the window
bool initWindow(std::string windowName, float aspectRatio);

Mat TransformImageBackwards(Mat img, Mat M) {
    Mat invM = M.inv();
    Mat newImg = Mat::zeros(img.size(), img.type());
    for (int row = 0; row < img.rows; row++) {
        for (int col = 0; col < img.cols; col++) {
            // Destination position
            Mat pos = (cv::Mat_<float>(3, 1) << col, row, 1);

            // Source position
            Mat srcPos = invM * pos;
            int r = (int)round(srcPos.at<float>(1, 0));
            int c = (int)round(srcPos.at<float>(0, 0));

            if (r < img.rows && c < img.cols && r >= 0 && c >= 0)
            {
                // Pixel
                Vec3b srcPixel = img.at<cv::Vec3b>(r, c);
                newImg.at<cv::Vec3b>(row, col) = srcPixel;
            }
        }
    }
    return newImg;
}

double timeFunction(function<void()> codeBlock) {
    auto start = std::chrono::high_resolution_clock::now();

    codeBlock();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    double milliseconds = duration.count() / 1000.0;
    return milliseconds;
}

// Helper function to create an FBO with a color attachment texture
GLuint createFBO(int width, int height, GLuint& outTexture) {
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create texture to render to
    glGenTextures(1, &outTexture);
    glBindTexture(GL_TEXTURE_2D, outTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Attach texture to FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outTexture, 0);

    // Check FBO is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR: Framebuffer not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return 0; // Return 0 to indicate failure
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return fbo;
}

void resizeFBO(GLuint& fbo, GLuint& texture, int newWidth, int newHeight) {
    // Delete old resources
    glDeleteTextures(1, &texture);
    glDeleteFramebuffers(1, &fbo);

    // Create new FBO with new size
    fbo = createFBO(newWidth, newHeight, texture);

    if (fbo == 0) {
        std::cerr << "Failed to recreate FBO!" << std::endl;
    }
}

void addShader(string label, string fragmentShaderName, map<string, TextureShader*>& shaders) {
    string vertexShader = "videoTextureShader.vert";
    shaders[label] = new TextureShader(vertexShader, fragmentShaderName);
    cout << "Texture shader program ID: " << shaders[label]->getProgramID() << endl;
    if (shaders[label]->getProgramID() == 0) {
        cerr << "ERROR: Texture shader failed to compile!" << endl;
    }
}

Mat getTranslationMatrix(float xoff, float yoff) {
    Mat trans = Mat::eye(3, 3, CV_32F);
    trans.at<float>(0, 2) = xoff;
    trans.at<float>(1, 2) = yoff;
    return trans;
}

Mat getRotationMatrix(float angleDegrees, Point2f center) {
    float angleRadians = angleDegrees * static_cast<float>(CV_PI) / 180.0f;
    Mat rot = Mat::eye(3, 3, CV_32F);
    rot.at<float>(0, 0) = cos(angleRadians);
    rot.at<float>(0, 1) = -sin(angleRadians);
    rot.at<float>(1, 0) = sin(angleRadians);
    rot.at<float>(1, 1) = cos(angleRadians);
    Mat translateToOrigin = getTranslationMatrix(-center.x, -center.y);
    Mat translateBack = getTranslationMatrix(center.x, center.y);
    return translateBack * rot * translateToOrigin;
}

Mat getScaleMatrix(float xscale, float yscale, Point2f center) {
    Mat scale = Mat::eye(3, 3, CV_32F);
    scale.at<float>(0, 0) = xscale;
    scale.at<float>(1, 1) = yscale;
    Mat translateToOrigin = getTranslationMatrix(-center.x, -center.y);
    Mat translateBack = getTranslationMatrix(center.x, center.y);
    return translateBack * scale * translateToOrigin;
}

int main(int argc, char** argv) {
    utils::logging::setLogLevel(utils::logging::LOG_LEVEL_SILENT);

    Mat frame;

    VideoCapture cap;

    double fps = cap.get(CAP_PROP_FPS);
    cout << "Frames per second using video.get(CAP_PROP_FPS) : " << fps << endl;

    int deviceID = 0;
    int apiID = CAP_ANY;

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
    if (!initWindow("OpenCV to OpenGL Exercise", videoAspectRatio)) return -1;

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

    // Create shader map
    map<string, TextureShader*> shaders;

    addShader("none", "videoTextureShader.frag", shaders);
    addShader("greyscale", "greyscaleShader.frag", shaders);
    addShader("pixelated", "pixelatedShader.frag", shaders);
    addShader("blur", "blurShader.frag", shaders);
    addShader("sincity", "sinCityShader.frag", shaders);

    TextureShader* currentShader = shaders["none"];

    Scene* myScene = new Scene();

    // Pass aspectRatio to all shaders
    for (auto& pair : shaders) {
        GLuint programID = pair.second->getProgramID();
        GLuint ratioLoc = glGetUniformLocation(programID, "aspectRatio");
        glUseProgram(programID);
        glUniform1f(ratioLoc, videoAspectRatio);
    }

    glm::mat4 orthoProjection = glm::ortho(
        -videoAspectRatio, videoAspectRatio,
        -1.0f, 1.0f,
        0.1f, 100.0f
    );

    glm::mat4 viewMatrix = glm::lookAt(
        glm::vec3(0, 0, 5.0f),  // position
        glm::vec3(0, 0, 0.0f),  // look at center
        glm::vec3(0, 1, 0.0f)   // up vector
    );

    Camera* renderingCamera = new Camera(orthoProjection, viewMatrix);

    // Calculate aspect ratio and create a quad with the correct dimensions.
    Quad* myQuad = new Quad(videoAspectRatio);
    myQuad->setShader(currentShader);
    myScene->addObject(myQuad);

    // This variable will hold our OpenGL texture.
    Texture* videoTexture = nullptr;

    // --- CREATE THE TEXTURE ---
    videoTexture = new Texture(frame.data,
        frame.cols,
        frame.rows,
        true);

    // Create FBO
    GLuint fboTexture;
    GLuint fbo = createFBO(BASE_WIDTH * videoAspectRatio, BASE_WIDTH, fboTexture);

    if (fbo == 0) {
        std::cerr << "Failed to create FBO!" << std::endl;
    }

    // Wrap the FBO texture so you can use it in your shaders
    Texture* fboTextureObj = new Texture();
    fboTextureObj->m_textureID = fboTexture;


    // Create small FBO for pixelation filter
    GLuint fboSmallTexture;
    GLuint fboSmall = createFBO(int((BASE_WIDTH * videoAspectRatio) / 10), int((BASE_WIDTH) / 10), fboSmallTexture);

    if (fboSmall == 0) {
        std::cerr << "Failed to create FBO!" << std::endl;
    }

    // Wrap the FBO texture so you can use it in your shaders
    Texture* fboSmallTextureObj = new Texture();
    fboSmallTextureObj->m_textureID = fboSmallTexture;

    currentShader->setTexture(videoTexture);

    int mode = -1;
    int renderMode = 0;
    bool hasChanged = 0;
    bool resolutionChanged = false;

	FrameStats stats = FrameStats();
    InputState input = InputState();


    glfwSetScrollCallback(window, [](GLFWwindow* w, double xoffset, double yoffset) {
        InputState* input = static_cast<InputState*>(glfwGetWindowUserPointer(w));
        if (input) {
            input->mouse.scroll = static_cast<int>(yoffset);
        }
        });

    glfwSetWindowUserPointer(window, &input);


    GLuint programID;
    GLuint ratioLoc;
    bool isMultiPass = false;

    // Affine transformation
    float prevFrameRotation = 0.0f;
    float frameRotation = prevFrameRotation;
	float frameTranslationX = 0.0f;
	float frameTranslationY = 0.0f;
	float frameScale = 1.0f;
    while (!glfwWindowShouldClose(window)) {
        auto toBeTimed = [&]() {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                glfwSetWindowShouldClose(window, true);

            cap.read(frame);

            controlApp(mode, renderMode, resolutionChanged, stats, input);

            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
                frameRotation = 0;
				frameTranslationX = 0;
				frameTranslationY = 0;
				frameScale = 1.0f;
            }

            if (input.mouse.mbright) {
				double dx = input.mouse.xpos - input.mouse.rightClickX;
				double dy = input.mouse.ypos - input.mouse.rightClickY;
                double dragDistance = sqrt(dx * dx + dy * dy);
                frameRotation = static_cast<float>(dragDistance);
            }

            if (input.mouse.mbleft) {
                double dx = input.mouse.xpos - input.mouse.leftClickX;
                double dy = input.mouse.ypos - input.mouse.leftClickY;
                frameTranslationX = static_cast<float>(dx);
                frameTranslationY = static_cast<float>(dy);
            }

            if (input.mouse.scroll != 0) {
                frameScale *= std::pow(1.1f, static_cast<float>(input.mouse.scroll));
                frameScale = std::max(0.1f, frameScale);

                input.mouse.scroll = 0;
            }
            Mat compositeTransform = Mat::eye(3, 3, CV_32F);
            bool needsTransform = false;

            Point2f center(frame.cols / 2.0f + frameTranslationX,
                frame.rows / 2.0f + frameTranslationY);

            if (fabs(frameScale - 1.0f) > 0.0001f) {
                Mat scaleMatrix = getScaleMatrix(frameScale, frameScale, center);
                compositeTransform = compositeTransform * scaleMatrix;
                needsTransform = true;
            }

            if (fabs(frameRotation) > 0.1f) {
                Mat rotationMatrix = getRotationMatrix(frameRotation, center);
                compositeTransform = compositeTransform * rotationMatrix;
                needsTransform = true;
            }

            if (fabs(frameTranslationX) > 0.1f || fabs(frameTranslationY) > 0.1f) {
                Mat translationMatrix = getTranslationMatrix(frameTranslationX, frameTranslationY);
                compositeTransform = compositeTransform * translationMatrix;
                needsTransform = true;
            }

            if (needsTransform) {
                frame = TransformImageBackwards(frame, compositeTransform);
            }

            if (resolutionChanged) {
                // Resize window
                glfwSetWindowSize(window, BASE_WIDTH * videoAspectRatio, BASE_WIDTH);
                glViewport(0, 0, BASE_WIDTH * videoAspectRatio, BASE_WIDTH);

                // Resize FBOs
                resizeFBO(fbo, fboTexture, BASE_WIDTH * videoAspectRatio, BASE_WIDTH);
                fboTextureObj->m_textureID = fboTexture;

                resizeFBO(fboSmall, fboSmallTexture,
                    int((BASE_WIDTH * videoAspectRatio) / 10),
                    int(BASE_WIDTH / 10));
                fboSmallTextureObj->m_textureID = fboSmallTexture;

                resolutionChanged = false;
            }

            bool shaderRendering = renderMode;
            isMultiPass = false;
            if (shaderRendering) {
                switch (mode)
                {
                case -1: // None
                    currentShader = shaders["none"];
                    myQuad->setShader(currentShader);
                    currentShader->setTexture(videoTexture);
                    break;
                case 0: // B&W
                    currentShader = shaders["greyscale"];
                    myQuad->setShader(currentShader);
                    currentShader->setTexture(videoTexture);
                    break;
                case 1: // Pixelated shader
                    isMultiPass = true;

                    // PASS 1: Render to small FBO
                    glBindFramebuffer(GL_FRAMEBUFFER, fboSmall);
                    glViewport(0, 0, int((BASE_WIDTH * videoAspectRatio) / 10), int(BASE_WIDTH / 10));
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    myQuad->setShader(shaders["none"]);
                    shaders["none"]->setTexture(videoTexture);
                    myQuad->render(renderingCamera);

                    //// PASS 2: Upscale
                    glBindFramebuffer(GL_FRAMEBUFFER, 0);
                    glViewport(0, 0, BASE_WIDTH * videoAspectRatio, BASE_WIDTH);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    myQuad->setShader(shaders["none"]);
                    shaders["none"]->setTexture(fboSmallTextureObj);
                    myQuad->render(renderingCamera);

                    currentShader = shaders["none"];
                    break;
                case 2: // SinCity shader
                    currentShader = shaders["sincity"];
                    myQuad->setShader(currentShader);
                    currentShader->setTexture(videoTexture);
                    break;
                }
            }
            else {
                // Reset shader once
                if (input.filterChanged && input.renderChanged == 0) {
                    currentShader = shaders["none"];
                    myQuad->setShader(currentShader);
                    currentShader->setTexture(videoTexture);
                }
                applyCVFilter(mode, frame);
            }

            if (!frame.empty() && videoTexture != nullptr) {
                flip(frame, frame, 0);
                videoTexture->update(frame.data, frame.cols, frame.rows, true);
            }

            // Keep passing videoAspectRatio uniform
            GLuint programID = currentShader->getProgramID();
            if (programID != 0) {
                GLuint ratioLoc = glGetUniformLocation(programID, "aspectRatio");
                glUseProgram(programID);
                glUniform1f(ratioLoc, videoAspectRatio);
            }

            if (!isMultiPass) {
                myScene->render(renderingCamera);
            }
            glfwSwapBuffers(window);
            glfwPollEvents();
            };

        double frameTime = timeFunction(toBeTimed);
        stats.totalTime += frameTime;
        stats.totalFrames++;

        double averageFrameTime = stats.totalTime / stats.totalFrames;
        double averageFPS = 1000.0 / averageFrameTime;

        if (stats.totalFrames % 60 == 0) {
            cout << "Average FPS: " << averageFPS
                << " (Avg frame time: " << averageFrameTime << " ms)" << endl;
        }

        if (waitKey(1) == 113) {
            break;
        }
    }

    // --- Cleanup -----------------------------------------------------------
    std::cout << "Closing application..." << endl;
    cap.release();
    delete myScene;
    delete renderingCamera;

    // Clean up all shaders in map
    //for (auto& pair : shaders) {
    //    delete pair;
    //}

    delete videoTexture;

    glDeleteFramebuffers(1, &fbo);
    delete fboTextureObj;

    glfwTerminate();
    return 0;
}

// Helper function to initialize the window
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
    window = glfwCreateWindow(BASE_WIDTH * aspectRatio, BASE_WIDTH, windowName.c_str(), NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window.\n");
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    return true;
}