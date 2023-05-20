/**
 * Colors to display.
 */
Color red = Color(255, 0, 0);
Color green = Color(0, 255, 0);
Color blue = Color(0, 0, 255);
Color black = Color(0, 0, 0);

/**
 * Panel dimensions.
 * Code expects the matrix is wired row-wise in a serpentine fasion. 
 * Pixel 0 is is lower left, (r=0, c=0). Pixel 11 is (r=1, c=0).
 */ 
#define NUM_ROWS 4
#define NUM_COLUMNS 6

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
 * Class expects the matrix is wired row-wise in a serpentine fasion. 
 * Class to represent a pixel position, either defined by
 * (led position (index)) or (row, column). Once defined, you can use either 
 * the (position) or (row, column) interchangably.
 * Can optionally can hold a Color.
 */
class PixelPosition {
    public:
    int row = -1;
    int column = -1;
    int position = -1;
    bool onMatrix = false;
    Color color;

    /**
     * Find the column for a position.
     */
    static int rowForPosition(int position) {
        return (int) floor(position / NUM_COLUMNS);
    }

    /**
     * Find the column for a position.
     * We must know the row it's on to
     * compensate for the serpentine LED path.
     */
    static int columnForPosition(int row, int position) {
        int column = position % NUM_COLUMNS;
        if (row % 2 == 1) {
            // Compoensate for serpentine matrix
            column = (NUM_COLUMNS - 1) - column;
        }
        return column;
    }

    /**
     * Find the position for a known row and column.
     * Compensate for the serpentine LED path.
     */
    static int positionForRowAndColumn(int row, int column) {
        int actualColumn = (row % 2 == 1) ? (NUM_COLUMNS - 1 - column) : column;
        return NUM_COLUMNS * row + actualColumn;
    }

    /**
     * Constructor.
     * Create a PixelPosition based on a known led position (linear).
     * The row and column will be calculated.
     */
    PixelPosition(int position_) {     // Constructor
        position = position_;
        onMatrix = (position >= 0) && (position < (NUM_ROWS * NUM_COLUMNS));
        if (onMatrix) {
            row = rowForPosition(position);
            column = columnForPosition(row, position);
            // ESP_LOGD("PixelPosition for position", "p=%d -> (r=%d, c=%d)", position, row, column);
        }
    }

    /**
     * Constructor.
     * Create a PixelPosition based on a known row and column.
     * The (linear) position will be calculated.
     */
    PixelPosition(int row_, int column_) {     // Constructor
        row = row_;
        column = column_;
        onMatrix = (row >= 0 && row < NUM_ROWS && column >= 0 && column < NUM_COLUMNS);
        if (onMatrix) {
            position = positionForRowAndColumn(row, column);
            // ESP_LOGD("PixelPosition for (r,c)", "(r=%d, c=%d) -> p=%d", row, column, position);
        }
    }
};

/**
 * Enable / Disable Effects.
 * --------------------------
 */
// Flicker (candle-esque). This works best with a faster refresh rate such as 100ms.
bool effectFlickrEnabled = false;
// Bleed. Works fine with a 1s referesh rate.
bool effectBleedEnabled = false;

/**
 * The led strip positions for the hours, minutes, and seconds columns.
 */
std::vector<std::vector<int>> ledStripHoursColumns = { {0, 11}, {1, 10, 13, 22} };
std::vector<std::vector<int>> ledStripMinutesColumns = { {2, 9, 14}, {3, 8, 15, 20} };
std::vector<std::vector<int>> ledStripSecondsColumns = { {4, 7, 16}, {5, 6, 17, 18} };

/**
 * Array to know what pixels have been set during a draw iteration.
 * Can be used by effects.
 */
bool pixelSet[NUM_ROWS*NUM_COLUMNS];

/**
 * Used by the Bleed effect to store pixels to draw
 * in a later phase.
 */
std::vector<PixelPosition> bleedPixels;

/**
 * Set the color for one pixel.
 */
void setPixel(esphome::light::AddressableLight &lights, int ledStripPosition, Color &color) {
    lights[ledStripPosition] = color;
    pixelSet[ledStripPosition] = true;
    // ESP_LOGD("setPixel", "p=%d -> (r=%d, g=%d, b=%d)", ledStripPosition, color.red, color.green, color.blue);
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
        black : 
        Color(
            (int) round(redSum / count), 
            (int) round(greenSum / count), 
            (int) round(blueSum / count));
    // ESP_LOGD("averageColors", "average = (%d, %d, %d)", averageColor.red, averageColor.green, averageColor.blue);
    return averageColor;
}

/**
 * Flickr effect: Pixel recolor method.
 * Works best at higher referesh rates, such as 100ms.
 */
Color effectFlickr_recolorPixel(esphome::light::AddressableLight &lights, int ledStripPosition, Color &color) {
    // The below code will "shimmer", sort of.
    Color effectColor = color;
    // Returns 225-255
    int subcolor = 225 + (rand() % 30);
    if (color == red) {
        effectColor = Color(subcolor, 0, 0);
    }
    else if (color == green) {
        effectColor = Color(0, subcolor, 0);
    }
    else if (color == blue) {
        effectColor = Color(0, 0, subcolor);
    }
    return effectColor;
}

/**
 * Flickr effect: Post method.
 * Works best at higher referesh rates, such as 100ms.
 */
void effectFlickr_post(esphome::light::AddressableLight &lights) {
    // NOP
}

/**
 * Bleed effect: Pixel recolor method.
 * Works well at slow refresh rates, such as 1s.
 */
Color effectBleed_recolorPixel(esphome::light::AddressableLight &lights, int ledStripPosition, Color &color) {
    // NOP
    return color;
}

/**
 * Bleed effect: Post method.
 * Works well at slow refresh rates, such as 1s.
 */
void effectBleed_post(esphome::light::AddressableLight &lights) {
    bleedPixels.clear();
    for (int i = 0; i < NUM_ROWS * NUM_COLUMNS; i++) {
        if (!pixelSet[i]) {
            // Step through all pixels that we haven't yet set.
            PixelPosition base = PixelPosition(i);
            // Average the colors of the current and four surrounding pixels
            // (up, down, left, right).
            std::vector<PixelPosition> pixelPositionsToAverage = {
                base,
                PixelPosition(base.row + 1, base.column),
                PixelPosition(base.row - 1, base.column),
                PixelPosition(base.row, base.column - 1),
                PixelPosition(base.row, base.column + 1),
            };
            base.color = averageColors(lights, pixelPositionsToAverage);
            if ((base.color.red + base.color.green + base.color.blue) != 0) {
                // If the average isn't black, we'll it later because
                // we want to calculate all of the bleeds without
                // using the bleed pixels in the calculations.
                bleedPixels.push_back(base);
            }
        }
    }

    // Draw all of the non-black bleed pixels.
    for (PixelPosition bleedPixel : bleedPixels) {
        setPixel(lights, bleedPixel.position, bleedPixel.color);
    }
}

/**
 * Set the LED color for one column of the clock display.
 */
void setBCDLEDs(esphome::light::AddressableLight &lights, std::bitset<4> &bits, std::vector<int> &ledStripColumn, Color &color) {
    for (int i = 0; i < bits.size(); i++) {
        if (bits[i] != 0) {
            int ledStripPosition = ledStripColumn[i];
            Color drawColor = color;
            if (effectFlickrEnabled) {
                drawColor = effectFlickr_recolorPixel(lights, ledStripPosition, color);
            }
            if (effectBleedEnabled) {
                drawColor = effectBleed_recolorPixel(lights, ledStripPosition, color);
            }
            setPixel(lights, ledStripPosition, drawColor);
        }
    }
}

/**
 * Start drawing time.
 */
void startDrawTime(esphome::light::AddressableLight &lights) {
    // Clear the strip and pixelSet array in preparation
    // for drawing the new time.
    lights.all() = black;
    for (int i = 0; i < NUM_ROWS * NUM_COLUMNS; i++) {
        pixelSet[i] = false;
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
    if (effectFlickrEnabled) {
        effectFlickr_post(lights);
    }
    if (effectBleedEnabled) {
        effectBleed_post(lights);
    }
}
