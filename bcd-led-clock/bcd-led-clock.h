/**
 * The LED Number (_START) and number of LEDs (_COUNT) for
 * each of the Hours, Minutes, and Seconds columns.
 */
#define HOURS_COL_0_START 0
#define HOURS_COL_0_COUNT 2
#define HOURS_COL_1_START 2
#define HOURS_COL_1_COUNT 4

#define MINUTES_COL_0_START 6
#define MINUTES_COL_0_COUNT 3
#define MINUTES_COL_1_START 9
#define MINUTES_COL_1_COUNT 4

#define SECONDS_COL_0_START 13
#define SECONDS_COL_0_COUNT 3
#define SECONDS_COL_1_START 16
#define SECONDS_COL_1_COUNT 4

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
 * Set the "on" LEDs for one column of the clock display.
 */
void setBCDLEDs(esphome::light::AddressableLight &lights, std::bitset<4> &bits, int startLED, int numLEDs, Color &color) {
    for (int i = 0; i < numLEDs; i++) {
        if (bits[i] != 0) {
            lights[startLED + i] = color;
        }
    }
}

/**
 * Set the LEDs for one group (hour, minute, or second).
 */
void setLEDGroup(
        esphome::light::AddressableLight &lights, 
        std::vector<std::bitset<4>> &bcd, 
        int col0StartLED, int col0NumLEDs, 
        int col1StartLED, int col1NumLEDs, 
        Color &color) {
    if (bcd.size() == 1) {
        setBCDLEDs(lights, bcd[0], col1StartLED, col1NumLEDs, color);
    }
    else if (bcd.size() == 2) {
        setBCDLEDs(lights, bcd[0], col0StartLED, col0NumLEDs, color);
        setBCDLEDs(lights, bcd[1], col1StartLED, col1NumLEDs, color);
    }
}
