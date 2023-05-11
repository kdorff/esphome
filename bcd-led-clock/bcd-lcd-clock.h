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

/**
 * Convert an integer to BCD, in four bit groups.
 *    <10         will return one four bit group.
 *    =>10, <=99  will return two four bit groups.
 *
 * I got this algorithm from:
 *   https://stackoverflow.com/questions/48291111/converting-from-decimal-to-bcd
 */
std::vector<std::bitset<4>> dec_to_bin(int n) {
    std::vector<std::bitset<4>> repr;;
    while(n > 0){
        repr.push_back(std::bitset<4>(n % 10));
        n /= 10;
    }
    std::reverse(repr.begin(), repr.end());
    return repr;
}

/**
 * Set the "on" LEDS for one column of the clock display.
 */
void turnOnLEDs(esphome::light::AddressableLight &lights, std::bitset<4> &bits, int startLED, int numLEDs, Color &onColor) {
    for (int i = 0; i < numLEDs; i++) {
        if (bits[i] != 0) {
            lights[startLED + i] = onColor;
        }
    }
}
