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
#define TIME_WIDTH PW(100)
#define TIME_HEIGHT PH(50)
#define ALARM_WIDTH PW(100)
#define ALARM_HEIGHT PH(20)

#define DAY_WIDTH PW(50)
#define DAY_HEIGHT PH(17)
#define DATE_WIDTH PW(50)
#define DATE_HEIGHT PH(18)

#define CONT_WIDTH PW(15)
#define CONT_HEIGHT PH(20)

#define TEMP_LABEL_WIDTH PW(25)
#define TEMP_LABEL_HEIGHT PH(8)
#define TEMP_WIDTH TEMP_LABEL_WIDTH
#define TEMP_HEIGHT PH(22)

#define FLASH_WIDTH PW(90)
#define FLASH_HEIGHT PH(50)
#define FLASH_X PW(5)
#define FLASH_Y PH(20)

#define TOTAL_WIDTH (TIME_WIDTH + TEMP_WIDTH)

//
// TODO: Anything we can to to simplify touch?
// TODO: Move day of week below time, full width
//

// The Panels used by this app
DisplayPanel timePanel(0, 0, TIME_WIDTH, TIME_HEIGHT);
DisplayPanel alarmPanel(0, TIME_HEIGHT, ALARM_WIDTH, ALARM_HEIGHT);

DisplayTouchPanel contDownPanel(0, 0, CONT_WIDTH, CONT_HEIGHT);
DisplayTouchPanel contUpPanel(PW(85), 0, CONT_WIDTH, CONT_HEIGHT);

DisplayPanel dayPanel(TEMP_WIDTH, TIME_HEIGHT + ALARM_HEIGHT, DAY_WIDTH, DAY_HEIGHT);
DisplayPanel datePanel(TEMP_WIDTH, TIME_HEIGHT + ALARM_HEIGHT + DAY_HEIGHT, DATE_WIDTH, DATE_HEIGHT);

DisplayPanel insideTempPanel(0, TIME_HEIGHT + ALARM_HEIGHT, TEMP_WIDTH, TEMP_HEIGHT);
DisplayPanel insideLabelPanel(0, PH(92), TEMP_LABEL_WIDTH, TEMP_LABEL_HEIGHT);
DisplayPanel outdoorTempPanel(TEMP_WIDTH + DATE_WIDTH, TIME_HEIGHT + ALARM_HEIGHT, TEMP_WIDTH, TEMP_HEIGHT);
DisplayPanel outdoorLabelPanel(TEMP_WIDTH + DATE_WIDTH, PH(92), TEMP_LABEL_WIDTH, TEMP_LABEL_HEIGHT);

DisplayPanel flashPanel(FLASH_X, FLASH_Y, FLASH_WIDTH, FLASH_HEIGHT);

// Static text used by some Panels
std::vector<std::string> contDownText = {"󰃞"};
std::vector<std::string> contUpText = {"󰃠"};

std::vector<std::string> insideLabelText = {"room"};
std::vector<std::string> outdoorLabelText = {"outside"};

std::vector<std::string> blankText = {};

// One time, initialize the Panels
void initializePanels(esphome::display::DisplayBuffer &display) {
    timePanel.font = font_time;
    timePanel.color = Color::BLACK;
    timePanel.textColor = color_text_white;
    timePanel.fontVertOffset = 45;

    alarmPanel.font = font_alarm;
    alarmPanel.color = Color::BLACK;
    alarmPanel.textColor = color_text_white;

    dayPanel.font = font_day;
    dayPanel.color = Color::BLACK;
    dayPanel.textColor = color_text_white;

    datePanel.font = font_date;
    datePanel.color = Color::BLACK;
    datePanel.textColor = color_text_white;
    datePanel.fontVertOffset = -7;

    contUpPanel.font = icon_font_45;
    contUpPanel.color = Color::BLACK;
    contUpPanel.textColor = color_text_white;
    contUpPanel.text = contUpText;

    contDownPanel.font = icon_font_45;
    contDownPanel.color = Color::BLACK;
    contDownPanel.textColor = color_text_white;
    contDownPanel.text = contDownText;

    insideLabelPanel.font = font_temp_label;
    insideLabelPanel.color = Color::BLACK;
    insideLabelPanel.textColor = color_text_white;
    insideLabelPanel.text = insideLabelText;

    insideTempPanel.font = font_temp;
    insideTempPanel.color = Color::BLACK;
    insideTempPanel.textColor = color_text_white;

    outdoorLabelPanel.font = font_temp_label;
    outdoorLabelPanel.color = Color::BLACK;
    outdoorLabelPanel.textColor = color_text_white;
    outdoorLabelPanel.text = outdoorLabelText;

    outdoorTempPanel.font = font_temp;
    outdoorTempPanel.color = Color::BLACK;
    outdoorTempPanel.textColor = color_text_white;

    flashPanel.font = font_flash;
    flashPanel.color = Color::BLACK;
    flashPanel.textColor = color_text_white;
    flashPanel.drawPanelOutline = true;
    flashPanel.enabled = false;

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
void updatePanelStates(esphome::display::DisplayBuffer &display) {
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

    // Next alarm in room
    // if (next_alarm->has_state() && next_alarm->state != "unavailable") {
    //     sprintf(buffer, "%s", next_alarm->state.c_str());
    //     std::vector<std::string> alarmText = { buffer };
    //     alarmPanel.text = alarmText;
    //     // ESP_LOGD("update", "next_alarm=%s", next_alarm->state.c_str());
    // }
    // else {
        alarmPanel.text = blankText;
    // }

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
void drawPanels(esphome::display::DisplayBuffer &display) {
    // drawAllPanels is generally preferred
    DisplayPanel::drawAllPanels(display, {
        timePanel,
        alarmPanel,
        datePanel,
        dayPanel,
        insideLabelPanel,
        insideTempPanel,
        outdoorLabelPanel,
        outdoorTempPanel,
        contDownPanel,
        contUpPanel
    });
    // But draw flashPanel separately so it over-draws
    // what is below it.
    flashPanel.draw(display);
}
