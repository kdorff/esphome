//
// This is part of 4 files. Make sure you have the whole set
//    tft-office.yaml        https://gist.github.com/kdorff/363cc20a26fddf7a3dea6fabbcd04805
//    display-panel.h        https://gist.github.com/kdorff/5c26fb21c573e4309da2587aa6e9b5d3
//    display-touch-panel.h  https://gist.github.com/kdorff/78d45057ee7a1aaf92f839f576c99e0b
//    tft-room-time-temp.h   https://gist.github.com/kdorff/811f86b33bf8b63050dce7e91d70cac8
//

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
