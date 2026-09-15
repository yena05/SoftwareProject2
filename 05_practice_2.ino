const int LED_PIN = 7;

void setup() {
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // 처음 1초 동안 LED 켜기
  digitalWrite(LED_PIN, LOW);
  delay(1000);

  // 다음 1초 동안 5번 깜빡이기
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, HIGH); // 끄기
    delay(100);

    digitalWrite(LED_PIN, LOW);  // 켜기
    delay(100);
  }

  // 마지막에는 LED 끄기
  digitalWrite(LED_PIN, HIGH);

  while (1) {
    // infinite loop
  }
}
