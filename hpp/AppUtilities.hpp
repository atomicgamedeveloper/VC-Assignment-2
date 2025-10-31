#ifndef APP_UTILITIES_HPP
#define APP_UTILITIES_HPP

#include <string>
#include <map>
#include <functional>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <opencv2/core.hpp>
#include <glm/glm.hpp>

#include <common/TextureShader.hpp>
#include <common/Quad.hpp>
#include <common/Camera.hpp>
#include <common/Texture.hpp>

extern const char* window_name;
extern int BASE_WIDTH;
extern GLFWwindow* window;

bool initWindow(const std::string& windowName, float aspectRatio);

cv::Mat TransformImageBackwards(const cv::Mat& img, const cv::Mat& M);

double timeFunction(std::function<void()> codeBlock);

GLuint createFBO(int width, int height, GLuint& outTexture);

void resizeFBO(GLuint& fbo, GLuint& texture, int newWidth, int newHeight);

void addShader(const std::string& label,
    const std::string& fragmentShaderName,
    std::map<std::string, TextureShader*>& shaders);

cv::Mat getTranslationMatrix(float xoff, float yoff);
cv::Mat getRotationMatrix(float angleDegrees, cv::Point2f center);
cv::Mat getScaleMatrix(float xscale, float yscale, cv::Point2f center);

void setShaderMVP(TextureShader* shader, const glm::mat4& MVP);

cv::Mat getTransformationMatrix(cv::Mat sourceMat,
    float rotation,
    float translationX,
    float translationY,
    float scale,
    bool& needsTransform);

void applyShader(const std::string& shaderName,
    TextureShader*& currentShader,
    std::map<std::string, TextureShader*>& shaders,
    Quad* quad,
    Texture* videoTexture);

bool applyShaderFilters(int mode,
    std::map<std::string, TextureShader*>& shaders,
    Quad* quad,
    Camera* camera,
    Texture* videoTexture,
    Texture* fboSmallTexture,
    GLuint fboSmall,
    int baseWidth,
    float videoAspectRatio);

glm::mat4 getModelMatrix(float frameTranslationX,
    float frameTranslationY,
    float frameRotation,
    float frameScale,
    int   baseWidth,
    float videoAspectRatio);

#endif
