#include "esphome.h"
#include <vector>

// A rectangular panel to be displayed on the LCD that we can 
// write one or more lines of centered text to.
// One can also determine if a DisplayPanel is in the 
// range of a touch coordinates.
class DisplayPanel {
    public:
        // Position of the panel
        unsigned int x;
        unsigned int y;

        // Size of the panel
        unsigned int w;
        unsigned int h;

        // Calculated maximum x and y values. Use for touch in isTouchOnPanel()
        unsigned int max_x;
        unsigned int max_y;

        bool enabled = true;

        // Reduction of font height when calculating positioning
        // in printMulti(). 
        // If the lines are too far apart, increase this value.
        int fontHeightOffset = 0;

        // Change in vertical position when calculating
        // positioning in printMiddle() or printMulti().
        // If the lines start too low in the panel,
        // set this to a negative value.
        int fontVertOffset = 0;

        // Draw an outline around the panel
        // using textColor
        bool drawPanelOutline = false;

        // Color of the panel
        esphome::Color color;
        // Color of the text printed to the panel
        esphome::Color textColor;
        // Font of the text printed to the panel
        esphome::display::Font *font;

        // Text lines to print on the panel.
        std::vector<std::string> text = { };

        // Constructor
        DisplayPanel(unsigned int _x, unsigned int _y, unsigned int _w, unsigned int _h) {
            x = _x;
            y = _y;
            w = _w;
            h = _h;
            max_x = _x + _w;
            max_y = _y + _h;
        }

        void draw(esphome::display::DisplayBuffer &display) {
            drawRect(display);
            drawText(display);
        }

        static void drawAllPanels(esphome::display::DisplayBuffer &display, std::vector<DisplayPanel> panels) {
            for (std::vector<DisplayPanel>::iterator panel = panels.begin(); panel != panels.end(); panel++) {
                (*panel).drawRect(display);
            }
            for (std::vector<DisplayPanel>::iterator panel = panels.begin(); panel != panels.end(); panel++) {
                (*panel).drawText(display);
            }
        }

        // See the touched x,y location is in the range of the Panel.
        bool isTouchOnPanel(int tpX, int tpY) {
            bool found =
                (tpX >= x && tpX <= max_x) && 
                (tpY >= y && tpY <= max_y);
            return found;
        }
    protected:
        // Draw the Panel in the specified location
        // at the specified color.
        void drawRect(esphome::display::DisplayBuffer &display) {
            if (!enabled || w == 0 || h == 0) {
                // Noththing to draw.
                return;
            }

            // ESP_LOGD("panel", "Drawing panel x=%d, y=%d, w=%d, h=%d", x, y, w, h);

            display.filled_rectangle(x, y, w, h, color);
            if (drawPanelOutline) {
                display.rectangle(x, y, w, h, textColor);
            }
        }

        void drawText(esphome::display::DisplayBuffer &display) {
            if (!enabled || w == 0 || h == 0) {
                // Noththing to draw.
                return;
            }

            if (text.size() == 1) {
                printMiddle(display, text[0].c_str());
            }
            else if (text.size() > 1) {
                printMulti(display, text);
            }
        }

        // Print centered text with padding from the top.
        // Useful if printing multiple lines of text within a Panel.
        void printMulti(esphome::display::DisplayBuffer &display, 
                std::vector<std::string> &text) {

            // Determine the height of a line
            int width, x_offset, textHeight, height;
            font->measure("M", &width, &x_offset, &textHeight, &height);
            textHeight += fontHeightOffset;
            int topPadding = (h - (text.size() * (textHeight))) / 2;
            topPadding = topPadding >= 0 ? topPadding : 0;

            int lineNum = 0;
            for (std::vector<std::string>::iterator t = text.begin(); t != text.end(); t++) {
                std::string text = *t;
                int printY = y + topPadding + (lineNum * textHeight);
                display.print(
                    x + ((int) (w/2)), 
                    y + topPadding + (lineNum * textHeight) + fontVertOffset, 
                    font, textColor, TextAlign::TOP_CENTER, text.c_str());
                lineNum++;
            }
        }

        // Print text in the middle of the panel.
        // Useful if printing a single line of text within a Panel.
        void printMiddle(esphome::display::DisplayBuffer &display,
                const char *text) {
            display.print(
                x + ((int) (w/2)), 
                y + ((int) (h/2) + fontVertOffset), 
                font, textColor, TextAlign::CENTER, text);
        }
};
