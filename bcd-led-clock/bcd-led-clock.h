/**
 * Panel dimensions.
 * Code expects the matrix is wired row-wise in a serpentine fashion. 
 * Pixel 0 is is lower left, (r=0, c=0). Pixel 11 is (r=1, c=0).
 */ 
#define NUM_ROWS 4
#define NUM_COLUMNS 6

#include "bcd_led_clock/matrix_pixel.h"
#include "bcd_led_clock/effect_bleed.h"
#include "bcd_led_clock/effect_flicker.h"

/**
 * Colors to display.
 */
Color red = Color(255, 0, 0);
Color green = Color(0, 255, 0);
Color blue = Color(0, 0, 255);
Color black = Color(0, 0, 0);

/**
 * Array to know what pixels have been set during a draw iteration.
 * Can be used by effects.
 */
std::vector<bool> pixelsSet(NUM_ROWS*NUM_COLUMNS);

EffectBleed effectBleed;
EffectFlicker effectFlicker;

/**
 * List of all possible effects.
 */
std::vector<Effect*> allEffects = {
    &effectFlicker,
    &effectBleed
};

/**
 * Helper to convert a bit to an int (useful for printf, etc.).
 */
#define BIT_TO_INT(x) (x == 0) ? ((int) 0) : ((int) 1)

/**
 * Convert an integer to BCD, in four bit groups.
 *    <10         will return one four bit group.
 *    =>10, <=99  will return two four bit groups.
 *
 * I got this algorithm from:
 *   https://stackoverflow.com/questions/48291111/converting-from-decimal-to-bcd
 */
std::vector<std::bitset<4>> dec_to_bin(int n) {
    std::vector<std::bitset<4>> repr;
    while(n > 0){
        repr.push_back(std::bitset<4>(n % 10));
        n /= 10;
    }
    std::reverse(repr.begin(), repr.end());
    return repr;
}

/**
 * The led strip positions for the hours, minutes, and seconds columns.
 */
std::vector<std::vector<int>> ledStripHoursColumns = { {0, 11}, {1, 10, 13, 22} };
std::vector<std::vector<int>> ledStripMinutesColumns = { {2, 9, 14}, {3, 8, 15, 20} };
std::vector<std::vector<int>> ledStripSecondsColumns = { {4, 7, 16}, {5, 6, 17, 18} };

/**
 * Set the color for one pixel.
 */
void setPixel(esphome::light::AddressableLight &lights, MatrixPixel matrixPixel) {
    lights[matrixPixel.position] = matrixPixel.color;
    pixelsSet[matrixPixel.position] = true;
    // ESP_LOGD("setPixel", "p=%d -> (r=%d, g=%d, b=%d)", matrixPixel.position, matrixPixel.color.red, matrixPixel.color.green, matrixPixel.color.blue);
}

void setPixels(esphome::light::AddressableLight &lights, std::vector<MatrixPixel> matrixPixels) {
    for (MatrixPixel matrixPixel : matrixPixels) {
        setPixel(lights, matrixPixel);
    }
}

/**
 * Initialize the BCD LED clock. Should only be called. once.
 */
void initialize(esphome::light::AddressableLight &lights) {
}

/**
 * Start drawing time.
 */
void startDrawTime(esphome::light::AddressableLight &lights) {
    // Clear the strip and pixelsSet array in preparation
    // for drawing the new time.
    lights.all() = black;
    for (int i = 0; i < NUM_ROWS * NUM_COLUMNS; i++) {
        pixelsSet[i] = false;
    }
    for (auto effect : allEffects) {
        // ESP_LOGD("startDrawTime", "n=%s e=%d", effect->name.c_str(), effect->enabled);
        if (effect->enabled) {
            effect->pre(lights);
        }
    }
}

/**
 * Set the LED color for one column of the clock display.
 */
void setBCDLEDs(esphome::light::AddressableLight &lights, std::bitset<4> &bits, std::vector<int> &ledStripColumn, Color &color) {
    for (int i = 0; i < bits.size(); i++) {
        if (bits[i] != 0) {
            MatrixPixel matrixPixel = MatrixPixel(ledStripColumn[i]);
            matrixPixel.color = color;
            for (auto effect : allEffects) {
                if (effect->enabled) {
                    // ESP_LOGD("setBCDLEDs", "n=%s e=%d", effect->name.c_str(), effect->enabled);
                    matrixPixel.color = effect->recolorPixel(lights, matrixPixel);
                }
            }
            setPixel(lights, matrixPixel);
        }
    }
}

/**
 * Set the LED color for one group (hour, minute, or second).
 */
void setLEDGroup(
        esphome::light::AddressableLight &lights, 
        std::vector<std::bitset<4>> &bcd, 
        std::vector<std::vector<int>> &ledStripColums,
        Color &color) {
    if (bcd.size() == 1) {
        setBCDLEDs(lights, bcd[0], ledStripColums[1], color);
    }
    else if (bcd.size() == 2) {
        setBCDLEDs(lights, bcd[0], ledStripColums[0], color);
        setBCDLEDs(lights, bcd[1], ledStripColums[1], color);
    }
}

/**
 * Complete drawing time.
 */
void endDrawTime(esphome::light::AddressableLight &lights) {
    for (auto effect : allEffects) {
        if (effect->enabled) {
            std::vector<MatrixPixel> changedPixels = effect->post(lights, pixelsSet);
            // ESP_LOGD("endDrawTime", "n=%s e=%d changed=%d", effect->name.c_str(), effect->enabled, changedPixels.size());
            setPixels(lights, changedPixels);
        }
    }
}
