#ifndef EFFECT_BLEED_H
#define EFFECT_BLEED_H

#include "effect.h"

/**
 * Bleed effect. Works fine with a 1s referesh rate (or anything faster).
 */
class EffectBleed : public Effect {
    public:
    double bleedRedFactor = 0.3;
    double bleedGreenFactor = 0.3;
    double bleedBlueFactor = 0.4;

    EffectBleed() {
        name = "Bleed";
    }

    Color selectBleedColor(std::vector<MatrixPixel> consider) {
        for (MatrixPixel mp : consider) {
            if (mp.onMatrix) {
                Color color = mp.color;
                if ((color.red + color.green + color.blue) > 0) {
                    return Color(
                        round(color.red * bleedRedFactor) ,
                        round(color.green * bleedGreenFactor),
                        round(color.blue * bleedBlueFactor));
                }
            }
        }
        // No adjacent colors
        return Color(0, 0, 0);
    }

    /**
     * Bleed effect: Post method.
     */
    std::vector<MatrixPixel> post(esphome::light::AddressableLight &lights, std::vector<bool> &pixelsSet) override {
        std::vector<MatrixPixel> bleedPixels;
        for (int c = 0; c < NUM_COLUMNS; c++) {
            for (int r = 0; r < NUM_ROWS; r++) {
                MatrixPixel base = MatrixPixel(r, c);
                if (!pixelsSet[base.position]) {
                    // Find the bleed color considering the adjacent pixels.
                    std::vector<MatrixPixel> pixelPositionsToConsider = {
                        MatrixPixel(lights, base.row + 1, base.column),
                        MatrixPixel(lights, base.row - 1, base.column),
                        MatrixPixel(lights, base.row, base.column - 1),
                        MatrixPixel(lights, base.row, base.column + 1),
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
