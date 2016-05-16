#include <SPI.h>
// define a log base 2 function
#define LOG2(x) (log(x) / log(2))

// multiplexer variables

// change this number if using more chips
const int num_of_mux_chips = 1;
const int num_of_pots[num_of_mux_chips] = { 15 };
const int num_of_mux_channels = 16;
const int num_ctl_pins = LOG2(num_of_mux_channels);
int control_pins[num_ctl_pins];
// create an array to hold the masking value to create binary numbers
// these values will be set in the setup function
int control_values[num_ctl_pins];

// shift register variables

const int input_latch_pin = 9;
const int output_latch_pin = 10;
const int num_of_input_chips = 2;
const int num_of_output_chips = 2;
byte input_bytes[num_of_input_chips] = { 0 };
byte output_bytes[num_of_output_chips] = { 0 };
int num_of_output_pins[num_of_output_chips] = { 8, 5 };
// the 2D array must have as many rows as the total output pins
byte connection_matrix[13][num_of_input_chips] = { 0 };
bool connection_detected = false;
int connected_chip, connected_pin;
// set the pin number of the first switch on the last input chip, starting from 0
int first_switch = 5;
// set position of the DSP LED
int dsp_led = 7;
// set the pin number of the first LED on the last output chip, starting from 0
int first_led = 5;
// set the number of switch that control LEDs without receiving data from the serial port
int num_of_ctl_switches = 2;
// set a mask for the last input byte to properly detect connections
byte switch_mask = B11100000;

// set the total number of bytes to be transferred over serial
// num_of_pots * 2 to get 10bit resolution
// two bytes for the connection byte
// one for the input chip that is connected
// one for the output pin that is connected
// one for the three switches
// and one for the start character
const int num_of_data = 36;
byte transfer_array[num_of_data] = { 255 };


void get_input(){
  digitalWrite(input_latch_pin, LOW);
  digitalWrite(input_latch_pin, HIGH);
  for(int i = 0; i < num_of_input_chips; i++)
    input_bytes[i] = SPI.transfer(0);
}

void set_output(){
  digitalWrite(output_latch_pin, LOW);
  for(int i = num_of_output_chips - 1; i >= 0; i--)
    SPI.transfer(output_bytes[i]);
  digitalWrite(output_latch_pin, HIGH);
}

byte check_connections(int pin){
  byte detected_connection;
  for(int i = 0; i < num_of_input_chips; i++){
    // copy input byte because we need to mask the last one here
    // but we also need it unmasked in the main loop function
    byte copy_byte = input_bytes[i];
    // mask the last byte to exclude switches
    if(i == num_of_input_chips - 1) copy_byte &= (~switch_mask);
    if(copy_byte != connection_matrix[pin][i]){
      detected_connection = copy_byte;
      connected_chip = i;
      connection_detected = true;
      connection_matrix[pin][i] = copy_byte;
      break;
    }
  }
  return detected_connection;
}

void setup() {
  SPI.begin();

  pinMode(input_latch_pin, OUTPUT);
  pinMode(output_latch_pin, OUTPUT);

  set_output();

  // set the value masks, the control pins and their modes
  for(int i = 0; i < num_ctl_pins; i ++){
    control_values[i] = pow(2,(i+1)) - 1;
    control_pins[i] = (num_ctl_pins - i) + 1;
    pinMode(control_pins[i], OUTPUT);
  }

  Serial.begin(57600);
}

void loop() {
  int index = 1;
  byte detected_connection;

  if(Serial.available()){
    byte in_byte = Serial.read();
    bitWrite(output_bytes[num_of_output_chips - 1], dsp_led, in_byte);
  }

  // first go through the potentiometers
  for(int i = 0; i < num_of_mux_chips; i++){
    // this loop creates a 4bit binary number that goes through the multiplexer control pins
    for (int j = 0; j < num_of_pots[i]; j++){
      for(int k = 0; k < num_ctl_pins; k++){
        // this line translates the decimal j to binary
        // and sets the corresponding control pin
        digitalWrite(control_pins[k], (j & control_values[k]) >> k);
      }
      int val = analogRead(i);
      transfer_array[index++] = val & 0x007f;
      transfer_array[index++] = val >> 7;
    }
  }
  
  // then go through the shift registers
  for(int i = 0; i < num_of_output_chips; i++){
    for(int j = 0; j < num_of_output_pins[i]; j++){
      int pin = j + (i * 8);
      bitSet(output_bytes[i], j);
      set_output();
      delayMicroseconds(1);
      get_input();
      detected_connection = check_connections(pin);
      bitClear(output_bytes[i], j);
      if(connection_detected){
        connected_pin = pin;
        break;
      }
    }
    if(connection_detected) break;
  }
  
  if(connection_detected){
    transfer_array[index++] = detected_connection & 0x7f;
    transfer_array[index++] = detected_connection >> 7;
    transfer_array[index++] = connected_chip;
    transfer_array[index++] = connected_pin;
    connection_detected = false;
  }

  // write the masked byte with the three switches
  transfer_array[num_of_data - 1] = input_bytes[num_of_input_chips - 1] & switch_mask;

  // control the two LEDs according the two switches
  // copy the pin variables so we can post-increment them
  int first_switch_pin = first_switch;
  int first_led_pin = first_led;
  for(int i = 0; i < num_of_ctl_switches; i++){
    int switch_val = bitRead(input_bytes[num_of_input_chips - 1], first_switch_pin++);
    bitWrite(output_bytes[num_of_output_chips - 1], first_led_pin++, switch_val);
  }

  Serial.write(transfer_array, num_of_data);
}
