#ifndef EFFECT_FLICKER_H
#define EFFECT_FLICKER_H

#include "effect.h"

/**
 * Flicker effect. Works better with a higher refresh rate, such as 100ms.
 */
class EffectFlicker : public Effect {
    public:
    /**
     * Tunable paratmers to control how much to flicker.
     * A larger value flicker more.
     */
    int flickerSize = 30;

    /**
     * Constructor.
     */
    EffectFlicker() {
        name = "Flicker";
    }

    /**
     * Effect code to execute as a pixel is being drawn. This provides an option to
     * return a new, alternative Color for that pixel, if desired.
     */
    Color recolorPixel(esphome::light::AddressableLight &strip, MatrixPixel &matrixPixel) override {
        // The below code will "shimmer", sort of.
        Color color = matrixPixel.color;
        Color effectColor = color;
        // Vary the color between (255-flickerSize) and 255
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
