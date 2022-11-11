#include "esphome.h"
#include <vector>

// Extends DisplayPanel to add touch supports.
class DisplayTouchPanel: public DisplayPanel {
    public:
        // Calculated maximum x and y values. Use for touch in isTouchOnPanel()
        unsigned int max_x;
        unsigned int max_y;

        // Constructor, uses DisplayPanel's constructor.
        DisplayTouchPanel(unsigned int _x, unsigned int _y, unsigned int _w, unsigned int _h): DisplayPanel(_x, _y, _w, _h) {
            max_x = _x + _w;
            max_y = _y + _h;
        }

        // See the point touched is in the range of the Panel.
        bool isTouchOnPanel(esphome::touchscreen::TouchPoint touchPoint) {
            bool found =
                (touchPoint.x >= x && touchPoint.x <= max_x) && 
                (touchPoint.y >= y && touchPoint.y <= max_y);
            return found;
        }
};
