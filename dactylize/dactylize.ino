/*
  dactylize.ino: Arduino sketch for patchbay circuit.
  Based on code borrowed from *Digityal Electronics for Musicians*
  by Alexandros Drymonitis.
*/

#include <SPI.h>
const int INPUT_LATCH_PIN = 9;
const int OUTPUT_LATCH_PIN = 10;
const int NUM_OF_INPUT_CHIPS = 2;
const int NUM_OF_OUTPUT_CHIPS = 2;

byte input_bytes[NUM_OF_INPUT_CHIPS] = { 0 };
byte output_bytes[NUM_OF_OUTPUT_CHIPS] = { 0 };
// we need two bytes for the connection byte
// one for the input chip that is connected
// one for the output pin that is connected
// and one for the start character
const int NUM_OF_DATA = 5;
byte transfer_array[NUM_OF_DATA] = { 192 };

const int NUM_OF_OUTPUT_PINS = NUM_OF_OUTPUT_CHIPS * 8;
byte connection_matrix[NUM_OF_OUTPUT_PINS][NUM_OF_INPUT_CHIPS] = { 0 };
bool connection_detected = false;
int connected_chip;
int connected_pin;

void get_input() {
    digitalWrite(INPUT_LATCH_PIN, LOW);
    digitalWrite(INPUT_LATCH_PIN, HIGH);
    for (int i = 0; i < NUM_OF_INPUT_CHIPS; i++) {
        input_bytes[i] = SPI.transfer(0);
    }
}

void set_output() {
    digitalWrite(OUTPUT_LATCH_PIN, LOW);
    for (int i = NUM_OF_OUTPUT_CHIPS - 1; i >= 0; i--)
    SPI.transfer(output_bytes[i]);
    digitalWrite(OUTPUT_LATCH_PIN, HIGH);
}

byte check_connections(int pin) {
    byte detected_connection;
    for (int i = 0; i < NUM_OF_INPUT_CHIPS; i++) {
        if (input_bytes[i] != connection_matrix[pin][i]) {
            detected_connection = input_bytes[i];
            connected_chip = i;
            connection_detected = true;
            connection_matrix[pin][i] = input_bytes[i];
            break;
        }
    }
    return detected_connection;
}

void setup() {
    SPI.begin();
    pinMode(INPUT_LATCH_PIN, OUTPUT);
    pinMode(OUTPUT_LATCH_PIN, OUTPUT);

    set_output();

    Serial.begin(115200);
}

void loop() {
    int index = 1;
    byte detected_connection;

    for (int i = 0; i < NUM_OF_OUTPUT_CHIPS; i++) {
        for (int j = 0; j < 8; j++) {
            int pin = j + (i * 8);
            bitSet(output_bytes[i], j);
            set_output();
            delayMicroseconds(1);
            get_input();
            detected_connection = check_connections(pin);
            bitClear(output_bytes[i], j);
            if (connection_detected) {
                connected_pin = pin;
                break;
            }
        }
        if (connection_detected) {
            break;
        }
    }

    if (connection_detected) {
        transfer_array[index++] = detected_connection & 0x7f;
        transfer_array[index++] = detected_connection >> 7;
        transfer_array[index++] = connected_chip;
        transfer_array[index++] = connected_pin;
        connection_detected = false;

        Serial.write(transfer_array, NUM_OF_DATA);
    }
}
