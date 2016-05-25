#include <SPI.h>

const int latch_pin = 10;
const int num_of_chips = 2;
byte out_bytes[num_of_chips] = { 0 };

void set_output(){
  digitalWrite(latch_pin, LOW);
  for(int i = num_of_chips - 1; i >= 0; i--)
    SPI.transfer(out_bytes[i]);
  digitalWrite(latch_pin, HIGH);
}

void setup() {
  SPI.begin();

  pinMode(latch_pin, OUTPUT);
}

void loop() {
  for(int i = 0; i < num_of_chips; i++){
    // zero previous chip
    if(i) out_bytes[i - 1] = 0;
    for(int j = 0; j < 8; j++){
      // clear the previous bit
      if(j) bitClear(out_bytes[i], (j - 1));
      // set the current bit
      bitSet(out_bytes[i], j);
      set_output();
      delay(100);
    }
  }

  for(int i = num_of_chips - 1; i >= 0; i--){
    // zero previous chip
    if(i < num_of_chips - 1) out_bytes[i + 1] = 0;
    for(int j = 7; j >= 0; j--){
      // clear previous bit
      if(j < 7) bitClear(out_bytes[i], (j + 1));
      // set current bit
      bitSet(out_bytes[i], j);
      set_output();
      delay(100);
    }
  }
}
