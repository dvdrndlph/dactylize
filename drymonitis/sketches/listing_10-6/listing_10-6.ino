#include <SPI.h>

const int input_latch = 9;
const int output_latch = 10;
const int num_of_chips = 2;
byte in_bytes[num_of_chips];
byte out_bytes[num_of_chips];
const int num_of_data = (num_of_chips * 2) + 1;
byte transfer_array[num_of_data] = { 192 };

void get_input(){
  digitalWrite(input_latch, LOW);
  digitalWrite(input_latch, HIGH);
  for(int i = 0; i < num_of_chips; i++)
    in_bytes[i] = SPI.transfer(0);
}

void set_output(){
  digitalWrite(output_latch, LOW);
  for(int i = num_of_chips - 1; i >= 0; i--)
    SPI.transfer(out_bytes[i]);
  digitalWrite(output_latch, HIGH);
}

void setup() {
  SPI.begin();

  pinMode(input_latch, OUTPUT);
  pinMode(output_latch, OUTPUT);

  Serial.begin(115200);
}

void loop() {
  int index = 1;

  get_input();
  for(int i = 0; i < num_of_chips; i++){
    transfer_array[index++] = in_bytes[i] & 0x7f;
    transfer_array[index++] = in_bytes[i] >> 7;
    out_bytes[i] = in_bytes[i];
  }
  set_output();

  Serial.write(transfer_array, num_of_data);
}
