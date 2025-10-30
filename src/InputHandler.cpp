#include "../hpp/InputHandler.hpp"
#include <iostream>
using namespace std;

void FrameStats::reset() {
    totalTime = 0;
    totalFrames = 0;
}

void handleMouseInput() {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    cout << "\rCursor Position: (" << xpos << ", " << ypos << ")   " << flush;
}

void handleFilterInput(int& mode, InputState& input, FrameStats& stats) {
    const int MAX_MODE = 3, MIN_MODE = -1;
    vector<string> filterLabels{ "None", "Grey", "Pixelated", "SinCity", "Median Blur", "Gaussian" };

    bool left = glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS;
    bool right = glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS;

    if ((left || right) && !input.filterChanged) {
        if (right && mode < MAX_MODE) mode++;
        if (left && mode > MIN_MODE) mode--;

        input.filterChanged = true;
        cout << "\nInput detected.\nfilter: " << filterLabels.at(mode + 1) << endl;
        stats.reset();
    }

    if (!left && !right)
        input.filterChanged = false;
}

void handleRenderModeInput(int& renderMode, InputState& input, FrameStats& stats) {
    const int MAX_RENDER_MODE = 1, MIN_RENDER_MODE = 0;
    vector<string> renderLabels{ "OpenCV", "GLSL" };

    bool up = glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS;
    bool down = glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS;

    if ((up || down) && !input.renderChanged) {
        if (up && renderMode < MAX_RENDER_MODE) renderMode++;
        if (down && renderMode > MIN_RENDER_MODE) renderMode--;

        input.renderChanged = true;
        cout << "\nInput detected.\nrender mode: " << renderLabels.at(renderMode) << "\n" << endl;
        stats.reset();
    }

    if (!up && !down)
        input.renderChanged = false;
}

void handleResolutionInput(bool& resolutionChanged, InputState& input, FrameStats& stats) {
    bool one = glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS;
    bool two = glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS;
    bool three = glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS;
    bool four = glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS;

    if ((one || two || three || four) && !input.resolutionChanged) {
        cout << "\nInput detected.\n";

        if (one) {
            BASE_WIDTH = 480;
            cout << "Resolution set to 480p.\n" << endl;
        }
        else if (two) {
            BASE_WIDTH = 720;
            cout << "Resolution set to 720p.\n" << endl;
        }
        else if (three) {
            BASE_WIDTH = 1080;
            cout << "Resolution set to 1080p.\n" << endl;
        }
        else if (four) {
            BASE_WIDTH = 1600;
            cout << "Resolution set to 1600p.\n" << endl;
        }

        input.resolutionChanged = true;
        resolutionChanged = true;
        stats.reset();
    }

    if (!one && !two && !three && !four)
        input.resolutionChanged = false;
}

void controlApp(int& mode, int& renderMode, bool& resolutionChanged, FrameStats& stats, InputState& input) {
    handleMouseInput();
    handleFilterInput(mode, input, stats);
    handleRenderModeInput(renderMode, input, stats);
    handleResolutionInput(resolutionChanged, input, stats);
}
