#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  Serial.println("This script will help to find the on board LED or PIN connections in general.");
  for(int i = 12; i < 30; i++){
      if(i == 7 || i == 8 || i == 9 || i == 10 || i == 11 || i == 12) continue;
      Serial.print("PIN set High: ");
      Serial.println(i);
      pinMode(i, OUTPUT);
      digitalWrite(i, HIGH);
      delay(1000);
  }
}
void loop() {
}