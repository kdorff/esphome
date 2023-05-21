#ifndef EFFECT_H
#define EFFECT_H

#include "pixel_position.h"

class Effect {
    public:
    bool enabled = false;

    void pre(esphome::light::AddressableLight &lights) {
        // NOP    
    }

    Color recolorPixel(esphome::light::AddressableLight &lights, int ledStripPosition, Color &color) {
        // NOP
        return color;
    }

    std::vector<PixelPosition> post(esphome::light::AddressableLight &lights, std::vector<bool> &pixelsSet) {
        return std::vector<PixelPosition>();
    }

    /**
      * Given the list of pixelPositions on lights, return a Color that is
      * the average of pixels colors.
      */
    Color averageColors(esphome::light::AddressableLight &lights, std::vector<PixelPosition> &pixelPositions) {
        int count = 0;
        int redSum = 0;
        int greenSum = 0;
        int blueSum = 0;
        for (PixelPosition pp : pixelPositions) {
            if (pp.onMatrix) {
                // Only observe PixelPositions that landed on the LED matrix.
                Color color = lights[pp.position].get();
                count++;
                redSum += color.red;
                greenSum += color.green;
                blueSum += color.blue;
                // ESP_LOGD("averageColors", "averaging in p=%d (r=%d,c=%d) (r=%d, g=%d, b=%d)", pp.position, pp.row, pp.column, color.red, color.green, color.blue);
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
