#include <opencv2/highgui.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <iostream>
#include "aruco_samples_utility.hpp"
using namespace cv;

struct CharucoConfig {
    const char* name;
    int squaresX;
    int squaresY;
    int squareLength;
    int markerLength;
    aruco::PredefinedDictionaryType dictType;
    const char* description;
};

int main(int argc, char* argv[]) {
    CharucoConfig configs[] = {
        {"5x7_standard", 5, 7, 200, 100, aruco::DICT_6X6_250, "General purpose, balanced"},
        {"8x11_dense", 8, 11, 150, 112, aruco::DICT_6X6_250, "High accuracy, more points"},
        {"9x6_wide", 9, 6, 180, 120, aruco::DICT_5X5_100, "Wide-angle cameras, landscape"},
        {"4x5_compact", 4, 5, 250, 200, aruco::DICT_4X4_50, "Quick detection, large markers"}
    };

    int margins = 50;
    int borderBits = 1;
    bool showImage = true;

    for (const auto& config : configs) {
        std::cout << "Generating " << config.name << ": " << config.description << std::endl;

        aruco::Dictionary dictionary = aruco::getPredefinedDictionary(config.dictType);

        Size imageSize;
        imageSize.width = config.squaresX * config.squareLength + 2 * margins;
        imageSize.height = config.squaresY * config.squareLength + 2 * margins;

        aruco::CharucoBoard board(Size(config.squaresX, config.squaresY),
            float(config.squareLength),
            float(config.markerLength),
            dictionary);

        Mat boardImage;
        board.generateImage(imageSize, boardImage, margins, borderBits);

        String out = std::string("../../../charuco_board_") + config.name + ".jpg";
        imwrite(out, boardImage);
        std::cout << "  Saved: " << out << std::endl;

        if (showImage) {
            imshow(config.name, boardImage);
            waitKey(1000);  // Show each for 1 second
        }
    }

    if (showImage) {
        std::cout << "\nPress any key to close all windows..." << std::endl;
        waitKey(0);
    }

    return 0;
}