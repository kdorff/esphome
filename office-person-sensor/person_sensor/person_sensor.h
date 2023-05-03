#ifndef INCLUDE_PERSON_SENSOR_H
#define INCLUDE_PERSON_SENSOR_H

// Definitions for the Useful Sensors Person Sensor module.
// Includes the standard I2C address of the sensor, constants for the
// configuration commands, and the data structures used to communicate results
// to the main system.
// See the full developer guide at https://usfl.ink/ps_dev for more information.

#include <Wire.h>
#include <stdint.h>

// The I2C address of the person sensor board.
#define PERSON_SENSOR_I2C_ADDRESS (0x62)

// Configuration commands for the sensor. Write this as a byte to the I2C bus
// followed by a second byte as an argument value.
#define PERSON_SENSOR_REG_MODE             (0x01)
#define PERSON_SENSOR_REG_ENABLE_ID        (0x02)
#define PERSON_SENSOR_REG_SINGLE_SHOT      (0x03)
#define PERSON_SENSOR_REG_CALIBRATE_ID     (0x04)
#define PERSON_SENSOR_REG_PERSIST_IDS      (0x05)
#define PERSON_SENSOR_REG_ERASE_IDS        (0x06)
#define PERSON_SENSOR_REG_DEBUG_MODE       (0x07)

// The person sensor will never output more than four faces.
#define PERSON_SENSOR_MAX_FACES_COUNT (4)

// How many different faces the sensor can recognize.
#define PERSON_SENSOR_MAX_IDS_COUNT (7)

// The following structures represent the data format returned from the person
// sensor over the I2C communication protocol. The C standard doesn't
// guarantee the byte-wise layout of a regular struct across different
// platforms, so we add the non-standard (but widely supported) __packed__
// attribute to ensure the layouts are the same as the wire representation.

// The results returned from the sensor have a short header providing
// information about the length of the data packet:
//   reserved: Currently unused bytes.
//   data_size: Length of the entire packet, excluding the header and checksum.
//     For version 1.0 of the sensor, this should be 40.
typedef struct __attribute__ ((__packed__)) {
    uint8_t reserved[2];  // Bytes 0-1.
    uint16_t data_size;   // Bytes 2-3.
} person_sensor_results_header_t;

// Each face found has a set of information associated with it:
//   box_confidence: How certain we are we have found a face, from 0 to 255.
//   box_left: X coordinate of the left side of the box, from 0 to 255.
//   box_top: Y coordinate of the top edge of the box, from 0 to 255.
//   box_width: Width of the box, where 255 is the full view port size.
//   box_height: Height of the box, where 255 is the full view port size.
//   id_confidence: How sure the sensor is about the recognition result.
//   id: Numerical ID assigned to this face.
//   is_looking_at: Whether the person is facing the camera, 0 or 1.
typedef struct __attribute__ ((__packed__)) {
    uint8_t box_confidence;   // Byte 1.
    uint8_t box_left;         // Byte 2.
    uint8_t box_top;          // Byte 3.
    uint8_t box_right;        // Byte 4.
    uint8_t box_bottom;       // Byte 5.
    int8_t id_confidence;     // Byte 6.
    int8_t id;                // Byte 7
    uint8_t is_facing;        // Byte 8.
} person_sensor_face_t;

// This is the full structure of the packet returned over the wire from the
// sensor when we do an I2C read from the peripheral address.
// The checksum should be the CRC16 of bytes 0 to 38. You shouldn't need to
// verify this in practice, but we found it useful during our own debugging.
typedef struct __attribute__ ((__packed__)) {
    person_sensor_results_header_t header;                     // Bytes 0-4.
    int8_t num_faces;                                          // Byte 5.
    person_sensor_face_t faces[PERSON_SENSOR_MAX_FACES_COUNT]; // Bytes 6-37.
    uint16_t checksum;                                         // Bytes 38-39.
} person_sensor_results_t;

// Fetch the latest results from the sensor. Returns false if the read didn't
// succeed.
inline bool person_sensor_read(person_sensor_results_t* results) {
    // For an explanation of why we're doing the read in chunks see 
    // https://www.arduino.cc/reference/en/language/functions/communication/wire/
    // In particular: "The Wire library implementation uses a 32 byte buffer, 
    // therefore any communication should be within this limit. Exceeding 
    // bytes in a single transmission will just be dropped."
    // I missed this warning in my initial implementatiom, which caused 
    // https://github.com/usefulsensors/person_sensor_arduino/issues/2 on
    // older boards like the Uno.
    const int maxBytesPerChunk = 32;
    const int totalBytes = sizeof(person_sensor_results_t);
    int8_t* results_bytes = (int8_t*)(results);
    int index = 0;
    while (index < totalBytes) {
        const int bytesRemaining = totalBytes - index;
        const int bytesThisChunk = min(bytesRemaining, maxBytesPerChunk);
        const int endIndex = index + bytesThisChunk;
        const bool isLastChunk = (bytesRemaining <= maxBytesPerChunk);
        Wire.requestFrom(PERSON_SENSOR_I2C_ADDRESS, bytesThisChunk, isLastChunk);
        for (; index < endIndex; ++index) {
            if (Wire.available() < 1) {
                ESP_LOGE("person_sensor.h", "Only %d bytes available on I2C, but we need %d", index, bytesThisChunk);
                return false;
            }
            results_bytes[index] = Wire.read();
        }
    }
    return true;
}

// Writes the value to the sensor register over the I2C bus.
inline void person_sensor_write_reg(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(PERSON_SENSOR_I2C_ADDRESS);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}


// void test() {
//     person_sensor_results_t results = {};

//       // Perform a read action on the I2C address of the sensor to get the
//       // current face information detected.
//     if (!person_sensor_read(&results)) {
//         ESP_LOGD("office_person_sensor", "No person sensor results found on the i2c bus");
//         return {};
//     }
//     else {
//         ESP_LOGD("office_person_sensor", "********");
//         ESP_LOGD("office_person_sensor", "%d faces found", results.num_faces);
//         for (int i = 0; i < results.num_faces; ++i) {
//             const person_sensor_face_t* face = &results.faces[i];
//             ESP_LOGD("office_person_sensor", "Face # %d: %d confidence (l=%d, t=%d, r=%d, b=%d) %s", 
//                 i, 
//                 face->box_confidence,
//                 face->box_left,
//                 face->box_top,
//                 face->box_right,
//                 face->box_bottom,
//                 face->is_facing ? "facing" : "not facing");
//         }
//         return {};
//       }    
// }

#endif  // INCLUDE_PERSON_SENSOR_H
