#include <sstream>
#include <display-panel.h>

// The display/lcd we are working with. Defined in initializePanels().
esphome::display::Display* lcd = NULL;

// For sprintf calls.
char buffer[25];

#define TIME_WIDTH 92
#define TIME_HEIGHT 37

#define ALARM_WIDTH 92
#define ALARM_HEIGHT 13

#define TEMP_WIDTH 36
#define TEMP_HEIGHT 22
#define TEMP_LABEL_WIDTH TEMP_WIDTH
#define TEMP_LABEL_HEIGHT 10
#define DATE_WIDTH TIME_WIDTH
#define DATE_HEIGHT 14
#define TOTAL_WIDTH (TIME_WIDTH + TEMP_WIDTH)

// The Panels used by this app
DisplayPanel timePanel(0, 0, TIME_WIDTH, TIME_HEIGHT);
DisplayPanel alarmPanel(0, TIME_HEIGHT, ALARM_WIDTH, ALARM_HEIGHT);
DisplayPanel datePanel(0, TIME_HEIGHT + ALARM_HEIGHT, DATE_WIDTH, DATE_HEIGHT);

DisplayPanel insideLabelPanel(TIME_WIDTH, 0, TEMP_LABEL_WIDTH, TEMP_LABEL_HEIGHT);
DisplayPanel insideTempPanel(TIME_WIDTH, TEMP_LABEL_HEIGHT, TEMP_WIDTH, TEMP_HEIGHT);
DisplayPanel outdoorLabelPanel(TIME_WIDTH, TEMP_LABEL_HEIGHT + TEMP_HEIGHT, TEMP_LABEL_WIDTH, TEMP_LABEL_HEIGHT);
DisplayPanel outdoorTempPanel(TIME_WIDTH, (TEMP_LABEL_HEIGHT * 2) + TEMP_HEIGHT, TEMP_WIDTH, TEMP_HEIGHT);

std::vector<DisplayPanel*> panels = {
    &timePanel,
    &alarmPanel,
    &datePanel,
    &insideLabelPanel,
    &insideTempPanel,
    &outdoorLabelPanel,
    &outdoorTempPanel
};

std::vector<std::string> insideLabelText = {"room"};
std::vector<std::string> outdoorLabelText = {"outside"};
std::vector<std::string> blankText = {};

void initializePanels(esphome::display::Display &display) {
    lcd = &display;

    timePanel.font = font_time;
    timePanel.color = Color::BLACK;
    timePanel.textColor = Color::WHITE;

    alarmPanel.font = font_alarm;
    alarmPanel.color = Color::BLACK;
    alarmPanel.textColor = Color::WHITE;

    datePanel.font = font_date;
    datePanel.color = Color::BLACK;
    datePanel.textColor = Color::WHITE;

    insideLabelPanel.font = font_temp_label;
    insideLabelPanel.color = Color::BLACK;
    insideLabelPanel.textColor = Color::WHITE;
    insideLabelPanel.text = insideLabelText;

    insideTempPanel.font = font_temp;
    insideTempPanel.color = Color::BLACK;
    insideTempPanel.textColor = Color::WHITE;

    outdoorLabelPanel.font = font_temp_label;
    outdoorLabelPanel.color = Color::BLACK;
    outdoorLabelPanel.textColor = Color::WHITE;
    outdoorLabelPanel.text = outdoorLabelText;

    outdoorTempPanel.font = font_temp;
    outdoorTempPanel.color = Color::BLACK;
    outdoorTempPanel.textColor = Color::WHITE;
}

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

    // Next alarm in room
    if (next_alarm->has_state() && next_alarm->state != "unavailable") {
        sprintf(buffer, "%s", next_alarm->state.c_str());
        std::vector<std::string> alarmText = { buffer };
        alarmPanel.text = alarmText;
    }
    else {
        if (wind_speed->has_state() && wind_speed->state < 200) {
            std::stringstream ss;
            ss << "Wind " << value_accuracy_to_string(wind_speed->state, 0);
            if (wind_gust_speed->has_state()) {
                ss << "-" << value_accuracy_to_string(wind_gust_speed->state, 0);
            }
            if (wind_direction->has_state()) {
                ss << " " << wind_direction->state.c_str();
            }
            alarmPanel.text = { ss.str() };
        }
        else {
            alarmPanel.text = { "" };
        }
    }
    // Date
    std::string monthName = now.strftime("%a/%b");
    sprintf(buffer, "%s %d", monthName.c_str(), now.day_of_month);
    std::vector<std::string> dateText = { buffer };
    datePanel.text = dateText;

    if (back_yard_temperature->has_state() || inside_temperature->has_state()) {
        timePanel.w = TIME_WIDTH;
        datePanel.w = DATE_WIDTH;
        alarmPanel.w = ALARM_WIDTH;
        insideTempPanel.enabled = true;
        outdoorTempPanel.enabled = true;
        insideLabelPanel.enabled = true;
        outdoorLabelPanel.enabled = true;
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
        timePanel.w = TOTAL_WIDTH;
        datePanel.w = TOTAL_WIDTH;
        alarmPanel.w = TOTAL_WIDTH;
        insideTempPanel.enabled = false;
        outdoorTempPanel.enabled = false;
        insideLabelPanel.enabled = false;
        outdoorLabelPanel.enabled = false;
    }
}

// Draw all of the panels
void drawPanels() {
    DisplayPanel::drawAllPanels(*lcd, panels);
}
