/**
 * LED Matrix dimensions.
 * The code expects the matrix is wired row-wise in a serpentine fasion
 * such that pixel 0 is the lower-left (r=0, c=0);
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

/**
 * All possible effects.
 */
EffectBleed effectBleed;
EffectFlicker effectFlicker;

/**
 * Vector of all possible effects.
 * Make sure all of the effects defined above are listed here.
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
void setPixel(esphome::light::AddressableLight &strip, MatrixPixel matrixPixel) {
    strip[matrixPixel.position] = matrixPixel.color;
    pixelsSet[matrixPixel.position] = true;
    // ESP_LOGD("setPixel", "p=%d -> (r=%d, g=%d, b=%d)", matrixPixel.position, matrixPixel.color.red, matrixPixel.color.green, matrixPixel.color.blue);
}

/**
 * Set the color for a vector of zero or more pixels.
 */
void setPixels(esphome::light::AddressableLight &strip, std::vector<MatrixPixel> matrixPixels) {
    for (MatrixPixel matrixPixel : matrixPixels) {
        setPixel(strip, matrixPixel);
    }
}

/**
 * Initialize the BCD LED clock. Should only ever be called once.
 */
void initialize(esphome::light::AddressableLight &strip) {
}

/**
 * Start drawing time.
 */
void startDrawTime(esphome::light::AddressableLight &strip) {
    // Clear the strip and pixelsSet array in preparation
    // for drawing the new time.
    strip.all() = black;
    for (int i = 0; i < NUM_ROWS * NUM_COLUMNS; i++) {
        pixelsSet[i] = false;
    }
    for (auto effect : allEffects) {
        // ESP_LOGD("startDrawTime", "n=%s e=%d", effect->name.c_str(), effect->enabled);
        if (effect->enabled) {
            effect->pre(strip);
        }
    }
}

/**
 * Set the LED color for one column of the clock display.
 */
void setBCDLEDs(esphome::light::AddressableLight &strip, std::bitset<4> &bits, std::vector<int> &ledStripColumn, Color &color) {
    for (int i = 0; i < bits.size(); i++) {
        if (bits[i] != 0) {
            MatrixPixel matrixPixel = MatrixPixel(ledStripColumn[i]);
            matrixPixel.color = color;
            for (auto effect : allEffects) {
                if (effect->enabled) {
                    // ESP_LOGD("setBCDLEDs", "n=%s e=%d", effect->name.c_str(), effect->enabled);
                    matrixPixel.color = effect->recolorPixel(strip, matrixPixel);
                }
            }
            setPixel(strip, matrixPixel);
        }
    }
}

/**
 * Set the LED color for one group (hour, minute, or second).
 */
void setLEDGroup(
        esphome::light::AddressableLight &strip, 
        std::vector<std::bitset<4>> &bcd, 
        std::vector<std::vector<int>> &ledStripColums,
        Color &color) {
    if (bcd.size() == 1) {
        setBCDLEDs(strip, bcd[0], ledStripColums[1], color);
    }
    else if (bcd.size() == 2) {
        setBCDLEDs(strip, bcd[0], ledStripColums[0], color);
        setBCDLEDs(strip, bcd[1], ledStripColums[1], color);
    }
}

/**
 * Complete drawing time.
 */
void endDrawTime(esphome::light::AddressableLight &strip) {
    for (auto effect : allEffects) {
        if (effect->enabled) {
            std::vector<MatrixPixel> changedPixels = effect->post(strip, pixelsSet);
            // ESP_LOGD("endDrawTime", "n=%s e=%d changed=%d", effect->name.c_str(), effect->enabled, changedPixels.size());
            setPixels(strip, changedPixels);
        }
    }
}
