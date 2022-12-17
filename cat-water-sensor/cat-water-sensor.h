#include <display-panel.h>

// For sprintf calls.
char buffer[25];

#define TIME_WIDTH 92
#define TIME_HEIGHT 37

#define ALARM_WIDTH 92
#define ALARM_HEIGHT 13

#define DATE_WIDTH TIME_WIDTH
#define DATE_HEIGHT 14

#define COL_2_WIDTH 36
#define COL_2_ROW_HEIGHT 32

#define TOTAL_WIDTH (TIME_WIDTH + COL_2_WIDTH)

// The Panels used by this app
DisplayPanel timePanel(0, 0, TIME_WIDTH, TIME_HEIGHT);
DisplayPanel alarmPanel(0, TIME_HEIGHT, ALARM_WIDTH, ALARM_HEIGHT);
DisplayPanel datePanel(0, TIME_HEIGHT + ALARM_HEIGHT, DATE_WIDTH, DATE_HEIGHT);

DisplayPanel waterSymbolPanel(TIME_WIDTH, 0, COL_2_WIDTH, COL_2_ROW_HEIGHT);
DisplayPanel waterPercentPanel(TIME_WIDTH, COL_2_ROW_HEIGHT, COL_2_WIDTH, COL_2_ROW_HEIGHT);

std::vector<std::string> waterSymbolText = { "Cat", "H2O" };
std::vector<std::string> blankText = {};

void initializePanels(esphome::display::DisplayBuffer &display) {
    timePanel.font = font_time;
    timePanel.color = Color::BLACK;
    timePanel.textColor = Color::WHITE;

    alarmPanel.font = font_alarm;
    alarmPanel.color = Color::BLACK;
    alarmPanel.textColor = Color::WHITE;

    datePanel.font = font_date;
    datePanel.color = Color::BLACK;
    datePanel.textColor = Color::WHITE;

    waterSymbolPanel.font = font_water;
    waterSymbolPanel.color = Color::BLACK;
    waterSymbolPanel.textColor = Color::WHITE;
    waterSymbolPanel.text = waterSymbolText;

    waterPercentPanel.font = font_water;
    waterPercentPanel.color = Color::BLACK;
    waterPercentPanel.textColor = Color::WHITE;
    waterPercentPanel.text = blankText;
}

void drawPanels(esphome::display::DisplayBuffer &display) {
    timePanel.draw(display);
    alarmPanel.draw(display);
    datePanel.draw(display);
    waterSymbolPanel.draw(display);
    waterPercentPanel.draw(display);
}

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
    if (next_alarm->has_state() && next_alarm->state != "unavailable") {
        sprintf(buffer, "%s", next_alarm->state.c_str());
        std::vector<std::string> alarmText = { buffer };
        alarmPanel.text = alarmText;
        // ESP_LOGD("update", "next_alarm=%s", next_alarm->state.c_str());
    }
    else {
        std::string dayName = now.strftime("%A");
        std::vector<std::string> dayText = { dayName };
        alarmPanel.text = dayText;
    }

    // Date
    std::string monthName = now.strftime("%b");
    sprintf(buffer, "%s %d", monthName.c_str(), now.day_of_month);
    std::vector<std::string> dateText = { buffer };
    datePanel.text = dateText;

    if (cat_water_percentage->has_state()) {
        timePanel.w = TIME_WIDTH;
        datePanel.w = DATE_WIDTH;
        alarmPanel.w = ALARM_WIDTH;
        waterSymbolPanel.enabled = true;
        waterPercentPanel.enabled = true;
        if (cat_water_percentage->has_state()) {
            // Cat water percent
            sprintf(buffer, "%.0f%%", cat_water_percentage->state);
            std::vector<std::string> tempText = { buffer };
            waterPercentPanel.text = tempText;
        }
    }
    else {
        // If we don't have a temperature state, just show time.
        timePanel.w = TOTAL_WIDTH;
        datePanel.w = TOTAL_WIDTH;
        alarmPanel.w = TOTAL_WIDTH;
        waterSymbolPanel.enabled = false;
        waterPercentPanel.enabled = false;
    }
}

