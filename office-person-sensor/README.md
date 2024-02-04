ESPHome project that pairs an ESP8266 with a Person Sensor (sen21231).

This **does not** use ESPHome's `sen21231`. I had problems with the code and detection stability. I'm currently accessing the sensor directly which gives far fewer false positives and false negatives.

The `template` `sensor.person_sensor_loop` controls `template` `binary_sensor.office_person_sensor`.

See https://www.thingiverse.com/thing:6004207 for a housing / case and additional details.
