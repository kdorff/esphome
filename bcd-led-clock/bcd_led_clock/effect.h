#ifndef EFFECT_H
#define EFFECT_H

#include "matrix_pixel.h"

/**
 * Base Effect and including utility methods.
 */
class Effect {
    public:
    bool enabled = false;
    std::string name = "Effect";

    virtual void pre(esphome::light::AddressableLight &lights) {
        // NOP    
    }

    virtual Color recolorPixel(esphome::light::AddressableLight &lights, MatrixPixel &matrixPixel) {
        // NOP
        return matrixPixel.color;
    }

    virtual std::vector<MatrixPixel> post(esphome::light::AddressableLight &lights, std::vector<bool> &pixelsSet) {
        return std::vector<MatrixPixel>();
    }

    /**
      * Given the list of matrixPixels on lights, return a Color that is
      * the average of pixels colors.
      */
    static Color averageColors(esphome::light::AddressableLight &lights, std::vector<MatrixPixel> &matrixPixels) {
        int count = 0;
        int redSum = 0;
        int greenSum = 0;
        int blueSum = 0;
        for (MatrixPixel mp : matrixPixels) {
            if (mp.onMatrix) {
                // Only observe MatrixPixels that landed on the LED matrix.
                Color color = lights[mp.position].get();
                count++;
                redSum += color.red;
                greenSum += color.green;
                blueSum += color.blue;
                // ESP_LOGD("averageColors", "averaging in p=%d (r=%d,c=%d) (r=%d, g=%d, b=%d)", mp.position, mp.row, mp.column, color.red, color.green, color.blue);
            }
        }
        Color averageColor = count == 0 ? 
            Color(0, 0, 0) : 
            Color(
                (int) round(redSum / count), 
                (int) round(greenSum / count), 
                (int) round(blueSum / count));
        // ESP_LOGD("averageColors", "average = (%d, %d, %d)", averageColor.red, averageColor.green, averageColor.blue);
        return averageColor;
    }
};

#endif
