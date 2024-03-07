#include <sstream>
#include <display-panel.h>
// #include "display-panel-dev.h"

// The current page number. This device only has one page.
int pageNumber = 0;

// Last touched page
DisplayPanel* lastTouchedPanel = NULL;

// The display/lcd we are working with. Defined in initializePanels().
esphome::display::Display* lcd = NULL;

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

// One time, initialize the Panels
void initializePanels(esphome::display::Display &display) {
    lcd = &display;

    timePanel.font = font_time;
    timePanel.color = Color::BLACK;
    timePanel.textColor = color_text_white;
    timePanel.fontVertOffset = 8;
    timePanel.touchable = true;
    timePanel.name = "timePanel";

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
esphome::ESPTime flashUntil;

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

    // Time
    timePanel.text = { str_sprintf("%d:%02d", hour, minute) };

    // Day of the week
    dayPanel.text = { now.strftime("%a") + "/" + now.strftime("%b") + " " + std::to_string(now.day_of_month) };

    // Wind speed, gust, and direction
    if (wind_speed->has_state() && wind_speed->state < 200) {
        std::stringstream ss;
        ss << "Wind " << value_accuracy_to_string(wind_speed->state, 0);
        if (wind_gust_speed->has_state()) {
            ss << "+";
        }
        if (wind_direction->has_state()) {
            ss << " " << wind_direction->state.c_str();
        }
        datePanel.text = { ss.str() };
    }

    if (back_yard_temperature->has_state() || inside_temperature->has_state()) {
        insideLabelPanel.enabled = true;
        insideTempPanel.enabled = true;
        outdoorLabelPanel.enabled = true;
        outdoorTempPanel.enabled = true;
        if (inside_temperature->has_state()) {
            // Temperature
            insideTempPanel.text = { str_sprintf("%.0f°", inside_temperature->state) };
        }
        if (back_yard_temperature->has_state()) {
            // Temperature
            outdoorTempPanel.text = { str_sprintf("%.0f°", back_yard_temperature->state) };
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
    DisplayPanel::drawAllPanels(*lcd, pages[pageNumber]);
    // But draw flashPanel separately so it over-draws
    // what is below it.
    flashPanel.draw(*lcd);
}

// See if one of the enabled, touchable panels on the
// current page has been touched.
// lastTouchedPanel will be set to a pointer to the
// touched panel (or NULL of no panel was found for the coordinates).
boolean isPanelTouched(int tpX, int tpY) {
    lastTouchedPanel = DisplayPanel::touchedPanel(pages[pageNumber], tpX, tpY);
    return lastTouchedPanel != NULL;
}
