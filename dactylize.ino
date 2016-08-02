/*
  dactylize.ino: Arduino sketch for patchbay circuit.
  Based on code borrowed from Listing 10-7 in
  *Digital Electronics for Musicians* by Alexandros Drymonitis.
*/

#include <SPI.h>

const int INPUT_LATCH_PIN = 9;
const int OUTPUT_LATCH_PIN = 10;
const int NUM_OF_INPUT_CHIPS = 2;
const int NUM_OF_OUTPUT_CHIPS = 10;

byte input_bytes[NUM_OF_INPUT_CHIPS] = { 0 };
byte output_bytes[NUM_OF_OUTPUT_CHIPS] = { 0 };

const int NUM_OF_OUTPUT_PINS = NUM_OF_OUTPUT_CHIPS * 8;
byte connection_matrix[NUM_OF_OUTPUT_PINS][NUM_OF_INPUT_CHIPS] = { 0 };
bool change_detected = false;
int hand_chip_id;
int key_pin_id;
long change_count = 0;

void get_input() {
    digitalWrite(INPUT_LATCH_PIN, LOW);
    digitalWrite(INPUT_LATCH_PIN, HIGH);
    for (int i = 0; i < NUM_OF_INPUT_CHIPS; i++) {
        input_bytes[i] = SPI.transfer(0);
    }
}

void set_output() {
    digitalWrite(OUTPUT_LATCH_PIN, LOW);
    for (int i = NUM_OF_OUTPUT_CHIPS - 1; i >= 0; i--) {
      SPI.transfer(output_bytes[i]);
    }
    digitalWrite(OUTPUT_LATCH_PIN, HIGH);
}

byte check_connections(int pin) {
    byte detected_connection = 0;
    for (int i = 0; i < NUM_OF_INPUT_CHIPS; i++) {
        if (input_bytes[i] != connection_matrix[pin][i]) {
            /*
            Serial.print("Input (hand): ");
            Serial.println(i, DEC);
            Serial.println(input_bytes[i], BIN);
            Serial.println(connection_matrix[pin][i], BIN);
            */
            
            detected_connection = input_bytes[i];
            hand_chip_id = i;
            change_detected = true;
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

void print_new_state(byte detected_connection) {
    byte state = 0;
    if (detected_connection) {
        state = 1;
    }
        
    Serial.print(change_count++, DEC);
    Serial.print(": Hand:");
    Serial.print(hand_chip_id, DEC);
    Serial.print(" Finger:");
    Serial.print(detected_connection, DEC);   
    Serial.print(" Key:");
    Serial.print(key_pin_id, DEC);
    Serial.print(" State:");
    Serial.println(state, DEC);
}

void loop() {
    int index = 1;
    byte detected_connection;

    for (int i = 0; i < NUM_OF_OUTPUT_CHIPS; i++) {
        for (int j = 0; j < 8; j++) {
            int pin = j + (i * 8);
            bitSet(output_bytes[i], j);
            set_output();
            delayMicroseconds(10);
            get_input();
            detected_connection = check_connections(pin);
            bitClear(output_bytes[i], j);
            if (change_detected) {
                key_pin_id = pin;
                break;
            }
        }
        if (change_detected) {
            break;
        }
    }

    if (change_detected) {
        print_new_state(detected_connection);
    }
    change_detected = false;
}
