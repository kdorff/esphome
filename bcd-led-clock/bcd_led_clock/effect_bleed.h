#ifndef EFFECT_BLEED_H
#define EFFECT_BLEED_H

#include "effect.h"

/**
 * Bleed effect. Bleeds the color from pixels that are enabled
 * to adjacent colors (with a lighter color).
 * Works fine with a 1s referesh rate (or anything faster).
 */
class EffectBleed : public Effect {
    public:
    /**
     * Tunable paratmers to control how bright the bleed pixels will be.
     */
    double bleedRedFactor = 0.3;
    double bleedGreenFactor = 0.3;
    double bleedBlueFactor = 0.4;

    /**
     * Constructor.
     */
    EffectBleed() {
        name = "Bleed";
    }

    /**
     * Given a vector of MatrixPixel, find one to use as a Bleed pixel
     * and return it's color scaled with `bleed*Factor`.
     */
    Color selectBleedColor(std::vector<MatrixPixel> consider) {
        for (MatrixPixel mp : consider) {
            if (mp.onMatrix) {
                Color color = mp.color;
                if ((color.red + color.green + color.blue) > 0) {
                    // We found a non-black color. Let's use it.
                    return Color(
                        round(color.red * bleedRedFactor) ,
                        round(color.green * bleedGreenFactor),
                        round(color.blue * bleedBlueFactor));
                }
            }
        }
        // No adjacent colors. Return black.
        return Color(0, 0, 0);
    }

    /**
     * Effect code to execute after the matrix has been drawn,
     * in case the Effect wants to make more changes.
     */
    std::vector<MatrixPixel> post(esphome::light::AddressableLight &strip, std::vector<bool> &pixelsSet) override {
        std::vector<MatrixPixel> bleedPixels;
        for (int c = 0; c < NUM_COLUMNS; c++) {
            for (int r = 0; r < NUM_ROWS; r++) {
                MatrixPixel base = MatrixPixel(r, c);
                if (!pixelsSet[base.position]) {
                    // Find the bleed color considering the adjacent pixels.
                    std::vector<MatrixPixel> pixelPositionsToConsider = {
                        MatrixPixel(strip, base.row + 1, base.column),
                        MatrixPixel(strip, base.row - 1, base.column),
                        MatrixPixel(strip, base.row, base.column - 1),
                        MatrixPixel(strip, base.row, base.column + 1),
                    };
                    base.color = selectBleedColor(pixelPositionsToConsider);
                    if ((base.color.red + base.color.green + base.color.blue) != 0) {
                        bleedPixels.push_back(base);
                    }
                }
            }
        }

        return bleedPixels;
    }
};

#endif
