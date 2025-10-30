#ifndef INPUT_HANDLER_HPP
#define INPUT_HANDLER_HPP

#include <iostream>
#include <vector>
#include <GLFW/glfw3.h>
#include <opencv2/opencv.hpp>

extern GLFWwindow* window;
extern int BASE_WIDTH;

// Frame timing structure
struct FrameStats {
    double totalTime = 0.0;
    int totalFrames = 0;
    void reset();
};

// Input state for key handling
struct InputState {
    bool filterChanged = false;
    bool renderChanged = false;
    bool resolutionChanged = false;
};

// Function declarations
void handleMouseInput();
void handleFilterInput(int& mode, InputState& input, FrameStats& stats);
void handleRenderModeInput(int& renderMode, InputState& input, FrameStats& stats);
void handleResolutionInput(bool& resolutionChanged, InputState& input, FrameStats& stats);
void controlApp(int& mode, int& renderMode, bool& resolutionChanged, FrameStats& stats, InputState& input);

#endif
