#include <display-panel.h>
// #include "display-panel-dev.h"

// For sprintf calls.
char buffer[25];

// Size of the actual display
#define WIDTH 320
#define HEIGHT 240
// Convert percentage width or height (0-100) to pixels
#define PW(PCT_WIDTH) (PCT_WIDTH * 0.01 * WIDTH)
#define PH(PCT_HEIGHT) (PCT_HEIGHT * 0.01 * HEIGHT)

// Size information about the panels
// Size for PW(x), PH(x) are percentages, 0-100.
#define CONT_WIDTH PW(15)
#define CONT_HEIGHT PH(25)

#define DAY_WIDTH PW(70)
#define DAY_HEIGHT PH(13)
#define DATE_WIDTH PW(70)
#define DATE_HEIGHT PH(12)

#define TIME_WIDTH PW(100)
#define TIME_HEIGHT PH(40)

#define TEMP_WIDTH PW(50)
#define TEMP_HEIGHT PH(27)
#define TEMP_LABEL_WIDTH PW(50)
#define TEMP_LABEL_HEIGHT PH(8)

#define FLASH_WIDTH PW(90)
#define FLASH_HEIGHT PH(50)
#define FLASH_X PW(5)
#define FLASH_Y PH(20)

//
// TODO: Anything we can to to simplify touch?
// TODO: Move day of week below time, full width
//

// The Panels used by this app
// X, Y, W, H
DisplayPanel contDownPanel(0, 0, CONT_WIDTH, CONT_HEIGHT);
DisplayPanel datePanel(CONT_WIDTH, 0, DATE_WIDTH, DATE_HEIGHT);
DisplayPanel dayPanel(CONT_WIDTH, DATE_HEIGHT, DAY_WIDTH, DAY_HEIGHT);
DisplayPanel contUpPanel(PW(85), 0, CONT_WIDTH, CONT_HEIGHT);

DisplayPanel timePanel(0, CONT_HEIGHT, TIME_WIDTH, TIME_HEIGHT);

DisplayPanel insideTempPanel(0, CONT_HEIGHT + TIME_HEIGHT, TEMP_WIDTH, TEMP_HEIGHT);
DisplayPanel insideLabelPanel(0, CONT_HEIGHT + TIME_HEIGHT +  TEMP_HEIGHT, TEMP_LABEL_WIDTH, TEMP_LABEL_HEIGHT);
DisplayPanel outdoorTempPanel(TEMP_WIDTH, CONT_HEIGHT + TIME_HEIGHT, TEMP_WIDTH, TEMP_HEIGHT);
DisplayPanel outdoorLabelPanel(TEMP_WIDTH, CONT_HEIGHT + TIME_HEIGHT +  TEMP_HEIGHT, TEMP_LABEL_WIDTH, TEMP_LABEL_HEIGHT);

DisplayPanel butterflyPanel(0, 0, 200, 200);

DisplayPanel flashPanel(FLASH_X, FLASH_Y, FLASH_WIDTH, FLASH_HEIGHT);

// The current page number. This device only has one page.
int pageNumber = 0;

std::vector<std::vector<DisplayPanel*>> pages = {
    {
        // Page 0. The only page at the moment.
        &contDownPanel,
        &datePanel,
        &dayPanel,
        &contUpPanel,
        &timePanel,
        &insideTempPanel,
        &insideLabelPanel,
        &outdoorLabelPanel,
        &outdoorTempPanel,
        &butterflyPanel
    }
};

// Static text used by some Panels
std::vector<std::string> contDownText = { "\U000F00DE" };
std::vector<std::string> contUpText = { "\U000F00E0" };

std::vector<std::string> insideLabelText = {"room"};
std::vector<std::string> outdoorLabelText = {"outside"};

std::vector<std::string> blankText = {};

// Remember the display we are writing to so we
// can use it whenever we need. Defined in initializePanels().
esphome::display::DisplayBuffer* lcd = NULL;

// One time, initialize the Panels
void initializePanels(esphome::display::DisplayBuffer &display) {
    lcd = &display;

    timePanel.font = font_time;
    timePanel.color = Color::BLACK;
    timePanel.textColor = color_text_white;
    timePanel.fontVertOffset = 8;
    timePanel.touchable = false;

    dayPanel.font = font_day;
    dayPanel.color = Color::BLACK;
    dayPanel.textColor = color_text_white;
    dayPanel.touchable = false;

    datePanel.font = font_date;
    datePanel.color = Color::BLACK;
    datePanel.textColor = color_text_white;
    datePanel.touchable = false;

    contUpPanel.font = icon_font_45;
    contUpPanel.color = Color::BLACK;
    contUpPanel.textColor = color_text_white;
    contUpPanel.text = contUpText;
    contUpPanel.name = "contUpPanel";
    contUpPanel.tag = contUpPanel.name + " tag";

    contDownPanel.font = icon_font_45;
    contDownPanel.color = Color::BLACK;
    contDownPanel.textColor = color_text_white;
    contDownPanel.text = contDownText;
    contDownPanel.name = "contDownPanel";
    contDownPanel.tag = contDownPanel.name + " tag";

    insideTempPanel.font = font_temp;
    insideTempPanel.color = Color::BLACK;
    insideTempPanel.textColor = color_text_white;
    insideTempPanel.fontVertOffset = 6;
    insideTempPanel.touchable = false;

    insideLabelPanel.font = font_temp_label;
    insideLabelPanel.color = Color::BLACK;
    insideLabelPanel.textColor = color_text_white;
    insideLabelPanel.text = insideLabelText;
    insideLabelPanel.touchable = false;

    outdoorTempPanel.font = font_temp;
    outdoorTempPanel.color = Color::BLACK;
    outdoorTempPanel.textColor = color_text_white;
    outdoorTempPanel.fontVertOffset = 6;
    outdoorTempPanel.touchable = false;

    outdoorLabelPanel.font = font_temp_label;
    outdoorLabelPanel.color = Color::BLACK;
    outdoorLabelPanel.textColor = color_text_white;
    outdoorLabelPanel.text = outdoorLabelText;
    outdoorLabelPanel.touchable = false;

    butterflyPanel.font = font_flash;
    butterflyPanel.color = color_text_white;;
    butterflyPanel.textColor = color_green;
    butterflyPanel.image = butterfly_image;
    butterflyPanel.enabled = false;   // Was just for testing.
    butterflyPanel.tag = "hello, baby";
    butterflyPanel.touchable = false;

    flashPanel.font = font_flash;
    flashPanel.color = Color::BLACK;
    flashPanel.textColor = color_text_white;
    flashPanel.drawPanelOutline = true;
    flashPanel.enabled = false;  // By default not display.
    flashPanel.touchable = false;

    // Restore brightness from the global
    backlight->set_level(brightness->value());
}

// The time until which to display flash
esphome::time::ESPTime flashUntil;

// Enable the Flash message with some specific text
void enableFlash(std::vector<std::string> flashText) {
    flashUntil = esptime->now();
    flashUntil.increment_second();
    flashPanel.enabled = true;
    flashPanel.text = flashText;
}

// Once per "frame", update the state of all of the panels.
// This does NOT draw.
void updatePanelStates() {
    // Time      
    auto now = esptime->now();
    int hour = now.hour;
    int minute = now.minute;
    bool is_pm = hour >= 12;
    hour = hour == 0 ? 12 : 
        hour > 12 ? hour - 12 : hour;
    sprintf(buffer, "%d:%02d", hour, minute);
    std::vector<std::string> timeText = { buffer };
    timePanel.text = timeText;

    // Day of the week
    std::string dayName = now.strftime("%A");
    std::vector<std::string> dayText = { dayName };
    dayPanel.text = dayText;

    // Date
    std::string monthName = now.strftime("%b");
    sprintf(buffer, "%s. %d", monthName.c_str(), now.day_of_month);
    std::vector<std::string> dateText = { buffer };
    datePanel.text = dateText;

    if (back_yard_temperature->has_state() || inside_temperature->has_state()) {
        insideLabelPanel.enabled = true;
        insideTempPanel.enabled = true;
        outdoorLabelPanel.enabled = true;
        outdoorTempPanel.enabled = true;
        if (inside_temperature->has_state()) {
            // Temperature
            sprintf(buffer, "%.0f°", inside_temperature->state);
            std::vector<std::string> tempText = { buffer };
            insideTempPanel.text = tempText;
        }
        if (back_yard_temperature->has_state()) {
            // Temperature
            sprintf(buffer, "%.0f°", back_yard_temperature->state);
            std::vector<std::string> tempText = { buffer };
            outdoorTempPanel.text = tempText;
        }
    }
    else {
        // If we don't have a temperature state, just show time.
        insideLabelPanel.enabled = false;
        insideTempPanel.enabled = false;
        outdoorLabelPanel.enabled = false;
        outdoorTempPanel.enabled = false;
    }

    if (flashPanel.enabled && now > flashUntil) {
        flashPanel.enabled = false;
    }
}

// Draw all of the panels
void drawPanels() {
    // drawAllPanels is generally preferred
    DisplayPanel::drawAllPanels(*(lcd), pages[pageNumber]);
    // But draw flashPanel separately so it over-draws
    // what is below it.
    flashPanel.draw(*(lcd));
}


/* 
 * ALTERNATIVE method for handling touch in one go.
 * If touch is detected on one of the enabled, touchable
 * DisplayPanels on the current page, this will set 
 * lastTouchedPanel a pointer to the touched DisplayPanel.
 */

// DisplayPanel* lastTouchedPanel = NULL;

// Draw all of the panels
// boolean panelTouched(int tpX, int tpY) {
//     lastTouchedPanel = NULL;
//     for (std::vector<DisplayPanel*>::iterator panel = pages[pageNumber].begin(); panel != pages[pageNumber].end(); panel++) {
//         if ((*(*panel)).isTouchOnPanel(tpX, tpY)) {
//             ESP_LOGD("touched", "touched %s x=%d, y=%d", (*(*panel)).text[0], tpX, tpX);
//             lastTouchedPanel = &(*(*panel));
//             return true;
//         }
//     }
//     return false;
// }
