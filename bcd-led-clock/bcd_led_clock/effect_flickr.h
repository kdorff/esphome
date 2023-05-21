#ifndef EFFECT_FLICKER_H
#define EFFECT_FLICKER_H

#include "effect.h"

/**
 * Flicker effect. Works better with a higher refresh rate, such as 100ms.
 */
class EffectFlicker : public Effect {
    private:

    public:
    int flickerSize = 30;

    /**
     * Flicker effect: Pixel recolor method.
     */
    Color recolorPixel(esphome::light::AddressableLight &lights, int ledStripPosition, Color &color) {
        // The below code will "shimmer", sort of.
        Color effectColor = color;
        // Returns 225-255
        int subcolor = (255 - flickerSize) + (rand() % flickerSize);
        if (color == Color(255, 0, 0)) {
            effectColor = Color(subcolor, 0, 0);
        }
        else if (color == Color(0, 255, 0)) {
            effectColor = Color(0, subcolor, 0);
        }
        else if (color == Color(0, 0, 255)) {
            effectColor = Color(0, 0, subcolor);
        }
        return effectColor;
    }
};

#endif
