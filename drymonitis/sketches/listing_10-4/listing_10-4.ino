#include <SPI.h>

const int latch_pin = 9;
const int num_of_chips = 2;
const int num_of_data = (num_of_chips * 2) + 1;
byte transfer_array[num_of_data] = { 192 };

void setup() {
  SPI.begin();

  pinMode(latch_pin, OUTPUT);
  
  Serial.begin(115200);
}

void loop() {
  int index = 1;
  byte in_byte;

  digitalWrite(latch_pin, LOW);
  digitalWrite(latch_pin, HIGH);
  for(int i = 0; i < num_of_chips; i++){
    in_byte = SPI.transfer(0);
    transfer_array[index++] = in_byte & 0x7f;
    transfer_array[index++] = in_byte >> 7;
  }

  Serial.write(transfer_array, num_of_data);
}
