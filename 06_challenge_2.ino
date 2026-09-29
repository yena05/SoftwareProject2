#define LED 7

int period = 1000;   // us
int duty = 0;        // 0 ~ 100
int dir = 1;

void set_period(int p) {
  if (p < 100)
    p = 100;
  else if (p > 10000)
    p = 10000;

  period = p;
}

void set_duty(int d) {
  if (d < 0)
    d = 0;
  else if (d > 100)
    d = 100;

  duty = d;
}

void PWM() {
  int onTime = (long)period * duty / 100;
  int offTime = period - onTime;

  if (onTime > 0) {
    digitalWrite(LED, LOW);   // LED ON
    delayMicroseconds(onTime);
  }

  if (offTime > 0) {
    digitalWrite(LED, HIGH);  // LED OFF
    delayMicroseconds(offTime);
  }
}

void setup() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);   // 처음엔 LED OFF


  // set_period(10000);      // 10 ms
  // set_period(1000);       // 1 ms
   set_period(100);        // 0.1 ms
}

void loop() {
  set_duty(duty);

  PWM();

  duty += dir;

  if (duty >= 100) {
    duty = 100;
    dir = -1;
  }
  else if (duty <= 0) {
    duty = 0;
    dir = 1;
  }
}
