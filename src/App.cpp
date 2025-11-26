#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/aruco/charuco.hpp>
#include <iostream>
#include <stdio.h>
#include <windows.h>
#include <opencv2/core/utils/logger.hpp> 
#include <stdlib.h>
#include <string>
#include <map>

// Import utility functions
#include "../hpp/Filters.hpp"
#include "../hpp/InputHandler.hpp"
#include "../hpp/AppUtilities.hpp"

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

void calibrateCamera(VideoCapture& cap,
    GLFWwindow* window,
    Scene* myScene,
    Camera* renderingCamera,
    Texture* videoTexture) {
    cout << "Starting camera calibration..." << endl;
    cout << "Press 'C' key to capture frames for calibration" << endl;

    int capturedImages = 0;
    int capturedImagesGoal = 3;

    // Create charuco board object and CharucoDetector
    int squaresX = 5;
    int squaresY = 7;
    float squareLength = 0.038f;
    float markerLength = 0.019f;
    float aspectRatio = (float)squaresX / (float)squaresY;
    int calibrationFlags = 0;
    cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
    cv::aruco::DetectorParameters detectorParams = cv::aruco::DetectorParameters();
    cv::aruco::CharucoParameters charucoParams = cv::aruco::CharucoParameters();

    cv::aruco::CharucoBoard board(Size(squaresX, squaresY), squareLength, markerLength, dictionary);
    cv::aruco::CharucoDetector detector(board, charucoParams, detectorParams);

    vector<Mat> allCharucoCorners, allCharucoIds;
    vector<vector<Point2f>> allImagePoints;
    vector<vector<Point3f>> allObjectPoints;
    vector<Mat> allImages;
    Size imageSize;

    bool cKeyWasPressed = false;

    while (capturedImages < capturedImagesGoal && cap.grab()) {
        Mat image;
        cap.retrieve(image);
        flip(image, image, 0);
        flip(image, image, 1);

        vector<int> markerIds;
        vector<vector<Point2f>> markerCorners;
        Mat currentCharucoCorners, currentCharucoIds;
        vector<Point3f> currentObjectPoints;
        vector<Point2f> currentImagePoints;

        // Detect ChArUco board
        detector.detectBoard(image, currentCharucoCorners, currentCharucoIds);

        // Draw detected corners on the image for visual feedback
        if (currentCharucoCorners.total() > 0) {
            cv::aruco::drawDetectedCornersCharuco(image, currentCharucoCorners, currentCharucoIds);
        }

        // === RENDER FRAME ===
        videoTexture->update(image.data, image.cols, image.rows);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        myScene->draw(renderingCamera);
        glfwSwapBuffers(window);
        glfwPollEvents();
        // ====================

        // Check for 'C' key press using GLFW
        bool cKeyIsPressed = (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS);

        // Detect rising edge (key was just pressed)
        if (cKeyIsPressed && !cKeyWasPressed) {
            if (currentCharucoCorners.total() > 3) {
                // Match image points
                board.matchImagePoints(currentCharucoCorners, currentCharucoIds, currentObjectPoints, currentImagePoints);

                if (currentImagePoints.empty() || currentObjectPoints.empty()) {
                    cout << "Point matching failed, try again." << endl;
                }
                else {
                    cout << "Frame captured (" << capturedImages + 1 << "/" << capturedImagesGoal << ")" << endl;

                    allCharucoCorners.push_back(currentCharucoCorners);
                    allCharucoIds.push_back(currentCharucoIds);
                    allImagePoints.push_back(currentImagePoints);
                    allObjectPoints.push_back(currentObjectPoints);
                    allImages.push_back(image);

                    imageSize = image.size();
                    capturedImages++;
                }
            }
            else {
                cout << "Not enough corners detected (" << currentCharucoCorners.total() << " found, need >3)" << endl;
            }
        }

        cKeyWasPressed = cKeyIsPressed;

        // Check for ESC key to cancel
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            cout << "Calibration cancelled by user" << endl;
            return;
        }
    }

    if (capturedImages < capturedImagesGoal) {
        cout << "Calibration cancelled - insufficient images captured" << endl;
        return;
    }

    cout << "Running calibration with " << capturedImages << " images..." << endl;

    Mat cameraMatrix, distCoeffs;

    if (calibrationFlags & CALIB_FIX_ASPECT_RATIO) {
        cameraMatrix = Mat::eye(3, 3, CV_64F);
        cameraMatrix.at<double>(0, 0) = aspectRatio;
    }

    // Calibrate camera using ChArUco
    double repError = cv::calibrateCamera(allObjectPoints, allImagePoints, imageSize, cameraMatrix, distCoeffs,
        noArray(), noArray(), noArray(), noArray(), noArray(), calibrationFlags);

    cout << "\n=== Calibration Complete ===" << endl;
    cout << "Reprojection error: " << repError << endl;
    cout << "Camera matrix:\n" << cameraMatrix << endl;
    cout << "Distortion coefficients:\n" << distCoeffs << endl;
}

int main(int argc, char** argv) {
    utils::logging::setLogLevel(utils::logging::LOG_LEVEL_SILENT);

    Mat frame;
    VideoCapture cap;

    int deviceID = 0;
    int apiID = CAP_ANY;
    cap.open(deviceID, apiID);

    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }

    // Get one frame from the camera to determine its size
    cap.read(frame);
    flip(frame, frame, 0);
    if (frame.empty()) {
        cerr << "Error: couldn't capture an initial frame from camera. Exiting.\n";
        cap.release();
        return -1;
    }
    float videoAspectRatio = (float)frame.cols / (float)frame.rows;

    // Initialize OpenGL context
    if (!initWindow("OpenCV to OpenGL Exercise", videoAspectRatio)) return -1;

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Basic OpenGL setup
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glClearColor(0.1f, 0.1f, 0.2f, 0.0f);
    glEnable(GL_DEPTH_TEST);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create shader
    TextureShader* shader = new TextureShader(
        "videoTextureShader.vert",
        "videoTextureShader.frag"
    );

    // Setup camera and scene
    glm::mat4 orthoProjection = glm::ortho(
        -videoAspectRatio, videoAspectRatio,
        -1.0f, 1.0f,
        0.1f, 100.0f
    );

    glm::mat4 viewMatrix = glm::lookAt(
        glm::vec3(0, 0, 5.0f),
        glm::vec3(0, 0, 0.0f),
        glm::vec3(0, 1, 0.0f)
    );

    Camera* renderingCamera = new Camera(orthoProjection, viewMatrix);
    Scene* myScene = new Scene();

    // Create quad and add to scene
    Quad* myQuad = new Quad(videoAspectRatio);
    myQuad->setShader(shader);
    myScene->addObject(myQuad);

    // Create texture from video frame
    Texture* videoTexture = new Texture(frame.data, frame.cols, frame.rows, true);
    shader->setTexture(videoTexture);

    // Pass aspect ratio to shader
    GLuint programID = shader->getProgramID();
    GLuint ratioLoc = glGetUniformLocation(programID, "aspectRatio");
    glUseProgram(programID);
    glUniform1f(ratioLoc, videoAspectRatio);

    // FIXED: Pass the actual arguments, not a lambda
    calibrateCamera(cap, window, myScene, renderingCamera, videoTexture);

    // Main rendering loop
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        cap.read(frame);

        if (!frame.empty()) {
            flip(frame, frame, 0);
            videoTexture->update(frame.data, frame.cols, frame.rows, true);
        }

        myScene->render(renderingCamera);

        glfwSwapBuffers(window);
        glfwPollEvents();

        if (waitKey(1) == 113) { // 'q' key
            break;
        }
    }

    // Cleanup
    std::cout << "Closing application..." << endl;
    cap.release();
    delete myScene;
    delete renderingCamera;
    delete videoTexture;
    delete shader;

    glDeleteVertexArrays(1, &VertexArrayID);
    glfwTerminate();
    return 0;
}