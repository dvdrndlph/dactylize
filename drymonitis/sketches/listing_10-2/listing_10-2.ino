// define a log base 2 function
#define LOG2(x) (log(x) / log(2))

// change this number if using more chips
const int num_of_chips = 1;
// set the number of channels of the multiplexer
const int num_of_channels = 16;
// get the number of control pins according to the number of channels
const int num_ctl_pins = LOG2(num_of_channels);
// set the total number of bytes to be transferred over serial
// * 2 to get 10bit resolution
// + 1 for start character
const int num_of_data = ((num_of_chips * num_of_channels) * 2) + 1;
// buffer to hold data to be transferred over serial
// with the start character set
byte transfer_array[num_of_data] = { 192 };

// create an array for the control pins
int control_pins[num_ctl_pins];
// create an array to hold the masking value to create binary numbers
// these values will be set in the setup function
int control_values[num_ctl_pins];

void setup() {
  // set the value masks, the control pins and their modes
  for(int i = 0; i < num_ctl_pins; i ++){
    control_values[i] = pow(2,(i+1)) - 1;
    control_pins[i] = (num_ctl_pins - i) + 1;
    pinMode(control_pins[i], OUTPUT);
  }
  
  Serial.begin(115200);
}

void loop() {
  int index = 1;
  // loop to read all chips
  for(int i = 0; i < num_of_chips; i++){
    // this loop creates a 4bit binary number that goes through the multiplexer control pins
    for (int j = 0; j < num_of_channels; j++){
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
  Serial.write(transfer_array, num_of_data);
}
