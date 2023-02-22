#include <display-panel.h>

// For sprintf calls.
char buffer[25];

#define WIDTH 320
#define HEIGHT 240
// Convert percentage width or height (0-100) to pixels
// for easier panel layout.
#define PW(PCT_WIDTH) (PCT_WIDTH * 0.01 * WIDTH)
#define PH(PCT_HEIGHT) (PCT_HEIGHT * 0.01 * HEIGHT)

// The Panels used by this app. Params are (X, Y, W, H)
DisplayPanel backDoorPanel(0, 0, PW(44), PH(50));
DisplayPanel frontDoorPanel(PW(44), 0, PW(44), PH(50));
DisplayPanel brightnessUpPanel(PW(88), 0, PW(12), PH(25));
DisplayPanel brightnessDownPanel(PW(88), PH(25), PW(12), PH(25));
DisplayPanel timePanel(0, PH(50), PW(60), PH(50));
DisplayPanel tempPanel(PW(60), PH(50), PW(40), PH(50));

// Predefined Panel text options
std::vector<std::string> backDoorOpenText = {"Back", "Door", "Open"};
std::vector<std::string> backDoorText = {"Back", "Door"};
std::vector<std::string> frontDoorOpenText = {"Front", "Door", "Open"};
std::vector<std::string> frontDoorText = {"Front", "Door"};
std::vector<std::string> brightnessUpText = {"+"};
std::vector<std::string> brightnessDownText = {"-"};

void initializePanels(esphome::display::DisplayBuffer &display) {
    backDoorPanel.font = font_door;
    backDoorPanel.fontVertOffset = -5;
    backDoorPanel.fontHeightOffset = -5;

    frontDoorPanel.font = font_door;
    frontDoorPanel.fontVertOffset = -5;
    frontDoorPanel.fontHeightOffset = -5;

    timePanel.color = color_blue;
    timePanel.font = font_time;
    timePanel.textColor = Color::WHITE;

    tempPanel.color = color_blue;
    tempPanel.font = font_temp;
    tempPanel.textColor = Color::WHITE;;

    brightnessUpPanel.text = brightnessUpText;
    brightnessUpPanel.color = color_yellow;
    brightnessUpPanel.font = font_brightness;
    brightnessUpPanel.textColor = Color::BLACK;

    brightnessDownPanel.text = brightnessDownText;
    brightnessDownPanel.color = color_yellow;
    brightnessDownPanel.font = font_brightness;
    brightnessDownPanel.textColor = Color::BLACK;

    // Backlight initial level
    backlight->set_level(brightness->value());

    // Fill the screen the first time to have BLACK in any gaps in Panels.
    display.fill(Color::BLACK);
}

void updatePanelStates(esphome::display::DisplayBuffer &display) {
    //
    // Update configuration for Panels that might have changed.
    //
    backDoorPanel.color = 
        back_door_sensor->state && back_door_autolock->state ? color_yellow :
        back_door_sensor->state && !back_door_autolock->state ? color_orange :
        back_door_autolock->state ? color_green : color_red;
    backDoorPanel.textColor = 
        back_door_sensor->state ? Color::BLACK : 
        back_door_autolock->state ? Color::BLACK : Color::WHITE;
    backDoorPanel.text = back_door_sensor->state ? backDoorOpenText : backDoorText;

    frontDoorPanel.color = 
        front_door_sensor->state && back_door_autolock->state ? color_yellow :
        front_door_sensor->state && !front_door_autolock->state ? color_orange :
        front_door_autolock->state ? color_green : color_red;
    frontDoorPanel.textColor = 
        front_door_sensor->state ? Color::BLACK : 
        front_door_autolock->state ? Color::BLACK : Color::WHITE;
    frontDoorPanel.text = front_door_sensor->state ? frontDoorOpenText : frontDoorText;

    // Time      
    int hour = esptime->now().hour;
    int minute = esptime->now().minute;
    hour = hour == 0 ? 12 : 
        hour > 12 ? hour - 12 : hour;
    sprintf(buffer, "%d:%02d", hour, minute);
    std::vector<std::string> timeText = { buffer };
    timePanel.text = timeText;

    if (back_yard_temperature->has_state()) {
        // Temperature
        timePanel.w = PW(60);
        tempPanel.w = PW(40);
        sprintf(buffer, "%.0f°", back_yard_temperature->state);
        std::vector<std::string> tempText = { buffer };
        tempPanel.text = tempText;
    }
    else {
        // If we don't have a temperature state, just show time.
        timePanel.w = 320;
        tempPanel.w = 0;
    }
}

void drawPanels(esphome::display::DisplayBuffer &display) {
    DisplayPanel::drawAllPanels(display, {
        backDoorPanel,
        frontDoorPanel,
        brightnessUpPanel,
        brightnessDownPanel,
        timePanel,
        tempPanel
    });
}
