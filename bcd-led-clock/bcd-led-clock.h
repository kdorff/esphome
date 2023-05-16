/*
 * Colors to display
 */
Color red = Color(255, 0, 0);
Color green = Color(0, 255, 0);
Color blue = Color(0, 0, 255);
Color offColor = Color::BLACK;

/**
 * The led strip positions for the hours, minutes, and seconds columns.
 */
std::vector<int> ledStripHoursCol0 = {0, 11};
std::vector<int> ledStripHoursCol1 = {1, 10, 13, 22};

std::vector<int> ledStripMinsCol0 = {2, 9, 14};
std::vector<int> ledStripMinsCol1 = {3, 8, 15, 20};

std::vector<int> ledStripSecsCol0 = {4, 7, 16};
std::vector<int> ledStripSecsCol1 = {5, 6, 17, 18};

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
            // The below code will "shimmer", sort of.
            // This looked nice paired with 100ms update.
            // Returns 128-255
            int subcolor = 225 + (rand() % 30);
            Color drawColor = red;
            if (color == red) {
                drawColor = Color(subcolor, 0, 0);
            }
            else if (color == green) {
                drawColor = Color(0, subcolor, 0);
            }
            else if (color == blue) {
                drawColor = Color(0, 0, subcolor);
            }

            lights[col[i]] = drawColor;
        }
    }
}

/**
 * Set the LEDs for one group (hour, minute, or second).
 */
void setLEDGroup(
        esphome::light::AddressableLight &lights, 
        std::vector<std::bitset<4>> &bcd, 
        std::vector<int> &ledStripCol0, 
        std::vector<int> &ledStripCol1, 
        Color &color) {
    if (bcd.size() == 1) {
        setBCDLEDs(lights, bcd[0], ledStripCol1, color);
    }
    else if (bcd.size() == 2) {
        setBCDLEDs(lights, bcd[0], ledStripCol0, color);
        setBCDLEDs(lights, bcd[1], ledStripCol1, color);
    }
}
