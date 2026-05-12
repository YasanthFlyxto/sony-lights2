// Blink built-in LED — NO libraries needed
// Board: ESP32 Dev Module

void setup() {
  pinMode(2, OUTPUT); // GPIO2 = built-in blue LED on most ESP32 boards
  Serial.begin(115200);
  Serial.println("Board is alive!");
}

void loop() {
  digitalWrite(2, HIGH);
  delay(500);
  digitalWrite(2, LOW);
  delay(500);
}
