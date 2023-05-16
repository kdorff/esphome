
/**
 * The matrix positions for the hours, minutes, and seconds columns.
 */
std::vector<int> matrixHoursCol0 = {0, 11};
std::vector<int> matrixHoursCol1 = {1, 10, 13, 22};

std::vector<int> matrixMinsCol0 = {2, 9, 14};
std::vector<int> matrixMinsCol1 = {3, 8, 15, 20};

std::vector<int> matrixSecsCol0 = {4, 7, 16};
std::vector<int> matrixSecsCol1 = {5, 6, 17, 18};

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
void setBCDLEDs(esphome::light::AddressableLight &lights, std::bitset<4> &bits, std::vector<int> &col, Color &color) {
    for (int i = 0; i < col.size(); i++) {
        if (bits[i] != 0) {
            lights[col[i]] = color;
        }
    }
}

/**
 * Set the LEDs for one group (hour, minute, or second).
 */
void setLEDGroup(
        esphome::light::AddressableLight &lights, 
        std::vector<std::bitset<4>> &bcd, 
        std::vector<int> &matrixCol0, 
        std::vector<int> &matrixCol1, 
        Color &color) {
    if (bcd.size() == 1) {
        setBCDLEDs(lights, bcd[0], matrixCol1, color);
    }
    else if (bcd.size() == 2) {
        setBCDLEDs(lights, bcd[0], matrixCol0, color);
        setBCDLEDs(lights, bcd[1], matrixCol1, color);
    }
}
