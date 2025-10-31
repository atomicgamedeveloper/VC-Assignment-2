/*
* Experiments.cpp
* Runs trials of different configurations and logs performance data to CSV files.
*/

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../hpp/Filters.hpp"
#include "../hpp/InputHandler.hpp"
#include "../hpp/AppUtilities.hpp"

#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utils/logger.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <chrono>

#include <common/Shader.hpp>
#include <common/Camera.hpp>
#include <common/Scene.hpp>
#include <common/Object.hpp>
#include <common/TextureShader.hpp>
#include <common/Quad.hpp>
#include <common/Texture.hpp>
#include <random>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>

using namespace std;
using namespace cv;
using namespace glm;
namespace fs = std::filesystem;

using namespace std;
using namespace cv;
using namespace glm;


struct TestConfig {
    GLFWwindow* window{};
    GLuint VertexArrayID{};
    VideoCapture* cap{};
    float videoAspectRatio{};

    map<string, TextureShader*> shaders;
    TextureShader* currentShader{};

    Scene* myScene{};
    Camera* renderingCamera{};
    Quad* myQuad{};

    Texture* videoTexture{};
    Texture* fboTextureObj{};
    Texture* fboSmallTextureObj{};

    GLuint fbo{};
    GLuint fboTexture{};
    GLuint fboSmall{};
    GLuint fboSmallTexture{};

    string filter;
    int resolution{};
    bool useGPU{};
    bool useTransforms{};
};

void controlAppFromConfig(int& mode, int& renderMode,
    bool& resolutionChanged,
    FrameStats& stats,
    const TestConfig& config) {
    if (config.filter == "greyscale") mode = 0;
    else if (config.filter == "pixelated") mode = 1;
    else if (config.filter == "blur") mode = 2;
    else if (config.filter == "sincity") mode = 3;
    else mode = -1;

    renderMode = config.useGPU ? 1 : 0;

    if (BASE_WIDTH != config.resolution) {
        BASE_WIDTH = config.resolution;
        resolutionChanged = true;
        stats.reset();
    }
}


struct ExperimentResult {
    string filter;
    int resolution;
    bool useGPU;
    bool useTransforms;
    int totalFrames;
    double totalTime;
    double averageFrameTime;
    double averageFPS;
    double minFrameTime;
    double maxFrameTime;
    double stdDevFrameTime;
};

ExperimentResult makeResult(const TestConfig& cfg,
    const FrameStats& st,
    const std::vector<double>& times)
{
    ExperimentResult r;
    r.filter = cfg.filter;
    r.resolution = cfg.resolution;
    r.useGPU = cfg.useGPU;
    r.useTransforms = cfg.useTransforms;
    r.totalFrames = static_cast<int>(times.size());
    r.totalTime = st.totalTime;
    r.averageFrameTime = st.totalTime / st.totalFrames;
    r.averageFPS = 1000.0 / r.averageFrameTime;
    r.minFrameTime = *std::min_element(times.begin(), times.end());
    r.maxFrameTime = *std::max_element(times.begin(), times.end());

    // standard deviation
    double squaredSum = 0.0;
    for (double t : times) squaredSum += (t - r.averageFrameTime) * (t - r.averageFrameTime);
    r.stdDevFrameTime = std::sqrt(squaredSum / times.size());
    return r;
}

// Exports to out folder
string createExperimentFolderStructure() {
    string baseDir = "experimentResults";

    // Create base directory if it doesn't exist
    if (!fs::exists(baseDir)) {
        fs::create_directory(baseDir);
        cout << "Created directory: " << baseDir << endl;
    }

    // Create timestamped subfolder for this experiment session
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now;
    localtime_s(&tm_now, &time_t_now);

    std::ostringstream oss;
    oss << baseDir << "/experiment_"
        << std::put_time(&tm_now, "%Y%m%d_%H%M%S");
    string sessionDir = oss.str();

    fs::create_directory(sessionDir);
    cout << "Created session directory: " << sessionDir << endl;

    return sessionDir;
}

string getConfigFilename(const TestConfig& config, const string& sessionDir) {
    std::ostringstream oss;
    oss << sessionDir << "/"
        << config.filter << "_"
        << config.resolution << "p_"
        << (config.useGPU ? "GPU" : "CPU") << "_"
        << (config.useTransforms ? "WithTransforms" : "NoTransforms")
        << ".csv";
    return oss.str();
}

void saveFrameTimingsToCSV(const string& filename,
    const TestConfig& config,
    const vector<double>& frameTimes,
    const ExperimentResult& result) {
    std::ofstream file(filename);

    if (!file.is_open()) {
        cerr << "Error: Could not open file for writing: " << filename << endl;
        return;
    }

    file << "# Configuration Summary\n";
    file << "# Filter," << config.filter << "\n";
    file << "# Resolution," << config.resolution << "\n";
    file << "# Processing," << (config.useGPU ? "GPU" : "CPU") << "\n";
    file << "# Transforms," << (config.useTransforms ? "Yes" : "No") << "\n";
    file << "# Total Frames," << result.totalFrames << "\n";
    file << "# Total Time (ms)," << std::fixed << std::setprecision(3) << result.totalTime << "\n";
    file << "# Avg Frame Time (ms)," << std::fixed << std::setprecision(3) << result.averageFrameTime << "\n";
    file << "# Avg FPS," << std::fixed << std::setprecision(2) << result.averageFPS << "\n";
    file << "# Min Frame Time (ms)," << std::fixed << std::setprecision(3) << result.minFrameTime << "\n";
    file << "# Max Frame Time (ms)," << std::fixed << std::setprecision(3) << result.maxFrameTime << "\n";
    file << "# Std Dev Frame Time (ms)," << std::fixed << std::setprecision(3) << result.stdDevFrameTime << "\n";
    file << "#\n";

    file << "Frame Number,Frame Time (ms)\n";

    for (size_t i = 0; i < frameTimes.size(); ++i) {
        file << i << "," << std::fixed << std::setprecision(3) << frameTimes[i] << "\n";
    }

    file.close();
    cout << "Saved results to: " << filename << endl;
}

void saveSummaryCSV(const string& sessionDir, const vector<ExperimentResult>& allResults) {
    string summaryFile = sessionDir + "/summary.csv";
    std::ofstream file(summaryFile);

    if (!file.is_open()) {
        cerr << "Error: Could not open summary file for writing: " << summaryFile << endl;
        return;
    }

    file << "Filter,Resolution,Processing,Transforms,Total Frames,Total Time (ms),"
        << "Avg Frame Time (ms),Avg FPS,Min Frame Time (ms),Max Frame Time (ms),"
        << "Std Dev Frame Time (ms)\n";

    for (const auto& result : allResults) {
        file << result.filter << ","
            << result.resolution << ","
            << (result.useGPU ? "GPU" : "CPU") << ","
            << (result.useTransforms ? "Yes" : "No") << ","
            << result.totalFrames << ","
            << std::fixed << std::setprecision(3) << result.totalTime << ","
            << std::fixed << std::setprecision(3) << result.averageFrameTime << ","
            << std::fixed << std::setprecision(2) << result.averageFPS << ","
            << std::fixed << std::setprecision(3) << result.minFrameTime << ","
            << std::fixed << std::setprecision(3) << result.maxFrameTime << ","
            << std::fixed << std::setprecision(3) << result.stdDevFrameTime << "\n";
    }

    file.close();
    cout << "\nSaved summary to: " << summaryFile << endl;
}

float getRandomRange(float min, float max, std::default_random_engine& rng) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

int testConfiguration(TestConfig& config,
    const string& sessionDir,
    vector<ExperimentResult>& allResults) {
    utils::logging::setLogLevel(utils::logging::LOG_LEVEL_SILENT);
    const int numFrames = 2;
    FrameStats stats;
    vector<double> frameTimes;

    int mode = -1;
    if (config.filter == "greyframeScale") mode = 0;
    else if (config.filter == "pixelated") mode = 1;
    else if (config.filter == "blur") mode = 2;
    else if (config.filter == "sincity") mode = 3;

    int renderMode = config.useGPU ? 1 : 0;
    bool resolutionChanged = false;
    bool isMultiPass = false;

    if (BASE_WIDTH != config.resolution) {
        BASE_WIDTH = config.resolution;
        resolutionChanged = true;
    }

    Mat frame;

    std::default_random_engine rng(static_cast<unsigned>(
        std::chrono::system_clock::now().time_since_epoch().count()));

    float frameTranslationX = 0.0f;
    float frameTranslationY = 0.0f;
    float frameRotation = 0.0f;
    float frameScale = 1.0f;

    for (int frameNum = 0; frameNum < numFrames; ++frameNum) {

        double frameTime = timeFunction([&]() {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            config.cap->read(frame);

            if (frame.empty()) {
                std::cerr << "Warning: Empty frame captured at frame "
                    << frameNum << std::endl;
                return;
            }

            controlAppFromConfig(mode, renderMode, resolutionChanged, stats, config);

            bool needsTransform = false;
            cv::Mat transformationMatrix = cv::Mat::eye(3, 3, CV_32F);

            if (config.useTransforms) {
                frameTranslationX = getRandomRange(-0.1f, 0.1f, rng) * frame.cols;
                frameTranslationY = getRandomRange(-0.1f, 0.1f, rng) * frame.rows;
                frameRotation = getRandomRange(-15.0f, 15.0f, rng);
                frameScale = getRandomRange(0.9f, 1.1f, rng);

                transformationMatrix = getTransformationMatrix(
                    frame, frameRotation,
                    frameTranslationX, frameTranslationY,
                    frameScale, needsTransform
                );

                if (transformationMatrix.empty())
                    transformationMatrix = cv::Mat::eye(3, 3, CV_32F);

                if (needsTransform && !renderMode)
                    frame = TransformImageBackwards(frame, transformationMatrix);
            }

            if (resolutionChanged) {
                glfwSetWindowSize(config.window,
                    BASE_WIDTH * config.videoAspectRatio,
                    BASE_WIDTH);
                glViewport(0, 0,
                    BASE_WIDTH * config.videoAspectRatio,
                    BASE_WIDTH);
                resizeFBO(config.fbo, config.fboTexture,
                    BASE_WIDTH * config.videoAspectRatio,
                    BASE_WIDTH);
                config.fboTextureObj->m_textureID = config.fboTexture;
                resizeFBO(config.fboSmall, config.fboSmallTexture,
                    int((BASE_WIDTH * config.videoAspectRatio) / 10),
                    int(BASE_WIDTH / 10));
                config.fboSmallTextureObj->m_textureID = config.fboSmallTexture;
                resolutionChanged = false;
            }

            bool shaderRendering = renderMode;
            isMultiPass = false;
            if (shaderRendering) {
                glm::mat4 model = glm::mat4(1.0f);
                if (config.useTransforms) {
                    model = getModelMatrix(frameTranslationX, frameTranslationY,
                        frameRotation, frameScale,
                        BASE_WIDTH, config.videoAspectRatio);
                }
                config.myQuad->addTransform(model);

                applyShaderFilters(mode, config.shaders, config.myQuad,
                    config.renderingCamera, config.videoTexture,
                    config.fboSmallTextureObj,
                    config.fboSmall, BASE_WIDTH,
                    config.videoAspectRatio);
            }
            else {
                config.myQuad->addTransform(glm::mat4(1.0f));
                applyShader("none", config.currentShader, config.shaders,
                    config.myQuad, config.videoTexture);
                applyCVFilter(mode, frame);
            }

            if (!frame.empty() && config.videoTexture != nullptr) {
                flip(frame, frame, 0);
                config.videoTexture->update(frame.data, frame.cols, frame.rows, true);
            }

            GLuint programID = config.currentShader->getProgramID();
            if (programID != 0) {
                GLuint ratioLoc = glGetUniformLocation(programID, "aspectRatio");
                glUseProgram(programID);
                glUniform1f(ratioLoc, config.videoAspectRatio);
            }

            if (!isMultiPass)
                config.myScene->render(config.renderingCamera);

            glfwSwapBuffers(config.window);
            glfwPollEvents();
            });

        stats.addFrame(frameTime);
        frameTimes.push_back(frameTime);
    }

    ExperimentResult result = makeResult(config, stats, frameTimes);

    string csvFile = getConfigFilename(config, sessionDir);
    saveFrameTimingsToCSV(csvFile, config, frameTimes, result);

    allResults.push_back(result);

    std::cout << "\n=== Final Results ===" << std::endl;
    std::cout << "Filter: " << config.filter << std::endl;
    std::cout << "Resolution: " << config.resolution << std::endl;
    std::cout << "GPU: " << (config.useGPU ? "Yes" : "No") << std::endl;
    std::cout << "Transforms: " << (config.useTransforms ? "Yes" : "No") << std::endl;
    std::cout << "Average FPS: " << result.averageFPS << std::endl;

    return 0;
}


int main() {
    string sessionDir = createExperimentFolderStructure();
    vector<ExperimentResult> allResults;

    vector<string> shaderLabels = {
        "none",
        "greyframeScale",
        "pixelated",
        "sincity"
    };
    vector<int> resolutions = { 240, 720, 1080, 1600 };
    vector<bool> useGPUOptions = { true, false };
    vector<bool> useTransformsOptions = { true, false };

    Mat frame;
    VideoCapture cap;

    int deviceID = 0;
    int apiID = CAP_ANY;
    cap.open(deviceID, apiID);

    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }

    cap.read(frame);
    flip(frame, frame, 0);
    if (frame.empty()) {
        cerr << "Error: couldn't capture an initial frame from camera. Exiting.\n";
        cap.release();
        return -1;
    }
    float videoAspectRatio = (float)frame.cols / (float)frame.rows;

    if (!initWindow("OpenCV to OpenGL Exercise", videoAspectRatio)) return -1;

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glClearColor(0.1f, 0.1f, 0.2f, 0.0f);
    glEnable(GL_DEPTH_TEST);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    map<string, TextureShader*> shaders;
    addShader("none", "videoTextureShader.frag", shaders);
    addShader("greyframeScale", "greyframeScaleShader.frag", shaders);
    addShader("pixelated", "pixelatedShader.frag", shaders);
    addShader("blur", "blurShader.frag", shaders);
    addShader("sincity", "sinCityShader.frag", shaders);

    TextureShader* currentShader = shaders["none"];
    Scene* myScene = new Scene();

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
        glm::vec3(0, 0, 5.0f),
        glm::vec3(0, 0, 0.0f),
        glm::vec3(0, 1, 0.0f)
    );

    Camera* renderingCamera = new Camera(orthoProjection, viewMatrix);
    Quad* myQuad = new Quad(videoAspectRatio);
    myQuad->setShader(currentShader);
    myScene->addObject(myQuad);

    Texture* videoTexture = new Texture(frame.data, frame.cols, frame.rows, true);

    GLuint fboTexture;
    GLuint fbo = createFBO(BASE_WIDTH * videoAspectRatio, BASE_WIDTH, fboTexture);
    if (fbo == 0) {
        std::cerr << "Failed to create FBO!" << std::endl;
        return -1;
    }

    Texture* fboTextureObj = new Texture();
    fboTextureObj->m_textureID = fboTexture;

    GLuint fboSmallTexture;
    GLuint fboSmall = createFBO(int((BASE_WIDTH * videoAspectRatio) / 10),
        int((BASE_WIDTH) / 10), fboSmallTexture);
    if (fboSmall == 0) {
        std::cerr << "Failed to create small FBO!" << std::endl;
        return -1;
    }

    Texture* fboSmallTextureObj = new Texture();
    fboSmallTextureObj->m_textureID = fboSmallTexture;

    currentShader->setTexture(videoTexture);

    // Run all test configurations
    for (const auto& filter : shaderLabels) {
        for (const auto& resolution : resolutions) {
            for (const auto& gpu : useGPUOptions) {
                for (const auto& transforms : useTransformsOptions) {
                    cout << "\n========================================" << endl;
                    cout << "Testing configuration: "
                        << "Filter: " << filter << ", "
                        << "Resolution: " << resolution << ", "
                        << "Use GPU: " << (gpu ? "Yes" : "No") << ", "
                        << "Use Transforms: " << (transforms ? "Yes" : "No") << endl;
                    cout << "========================================\n" << endl;

                    TestConfig config;
                    config.window = window;
                    config.VertexArrayID = VertexArrayID;
                    config.cap = &cap;
                    config.videoAspectRatio = videoAspectRatio;
                    config.shaders = shaders;
                    config.currentShader = currentShader;
                    config.myScene = myScene;
                    config.renderingCamera = renderingCamera;
                    config.myQuad = myQuad;
                    config.videoTexture = videoTexture;
                    config.fboTextureObj = fboTextureObj;
                    config.fboSmallTextureObj = fboSmallTextureObj;
                    config.fbo = fbo;
                    config.fboTexture = fboTexture;
                    config.fboSmall = fboSmall;
                    config.fboSmallTexture = fboSmallTexture;
                    config.filter = filter;
                    config.resolution = resolution;
                    config.useGPU = gpu;
                    config.useTransforms = transforms;

                    testConfiguration(config, sessionDir, allResults);
                }
            }
        }
    }

    saveSummaryCSV(sessionDir, allResults);

    // Cleanup
    std::cout << "\nClosing application..." << endl;
    cap.release();
    delete myScene;
    delete renderingCamera;
    delete videoTexture;
    delete fboTextureObj;
    delete fboSmallTextureObj;

    glDeleteFramebuffers(1, &fbo);
    glDeleteFramebuffers(1, &fboSmall);
    glDeleteVertexArrays(1, &VertexArrayID);

    glfwTerminate();
    return 0;
}