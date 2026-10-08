#include <Servo.h>

// Arduino pin assignment
#define PIN_LED   9
#define PIN_TRIG  12
#define PIN_ECHO  13
#define PIN_SERVO 10

// Ultrasonic sensor
#define SND_VEL 346.0
#define INTERVAL 25
#define PULSE_DURATION 10

#define _DIST_MIN 180.0    // 18 cm
#define _DIST_MAX 360.0    // 36 cm

#define TIMEOUT ((INTERVAL / 2) * 1000.0)
#define SCALE (0.001 * 0.5 * SND_VEL)

// EMA
#define _EMA_ALPHA 0.3

// Servo calibration
#define _DUTY_MIN 530
#define _DUTY_MAX 2400

// global variables
unsigned long last_sampling_time;

float dist_prev = _DIST_MIN;
float dist_ema;
bool ema_first = true;

Servo myservo;


void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  digitalWrite(PIN_TRIG, LOW);
  digitalWrite(PIN_LED, HIGH);   // active-low -> LED OFF

  myservo.attach(PIN_SERVO);

  // 처음에는 0도 위치
  myservo.writeMicroseconds(_DUTY_MIN);

  Serial.begin(57600);
}


void loop() {
  float dist_raw;
  float dist_filtered;
  float servo_duty;

  // sampling interval
  if (millis() < last_sampling_time + INTERVAL)
    return;


  // 1. 초음파 거리 측정
  dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);


  // 2. Range Filter
  if ((dist_raw == 0.0) ||
      (dist_raw < _DIST_MIN) ||
      (dist_raw > _DIST_MAX)) {

    // 측정 실패 또는 범위 밖이면 직전 정상값 사용
    dist_filtered = dist_prev;

    // 범위 밖 -> LED OFF
    digitalWrite(PIN_LED, HIGH);
  }
  else {
    // 정상 범위
    dist_filtered = dist_raw;
    dist_prev = dist_raw;

    // 18~36 cm 안 -> LED ON
    digitalWrite(PIN_LED, LOW);
  }


  // 3. EMA Filter
  if (ema_first) {
    dist_ema = dist_filtered;
    ema_first = false;
  }
  else {
    dist_ema = _EMA_ALPHA * dist_filtered
             + (1.0 - _EMA_ALPHA) * dist_ema;
  }


  // 4. 거리에 따른 서보 각도 제어
  if (dist_ema <= _DIST_MIN) {

    // 18 cm 이하 -> 0도
    servo_duty = _DUTY_MIN;
  }
  else if (dist_ema >= _DIST_MAX) {

    // 36 cm 이상 -> 180도
    servo_duty = _DUTY_MAX;
  }
  else {

    // 18~36 cm -> 0~180도에 비례
    servo_duty =
      _DUTY_MIN
      + (dist_ema - _DIST_MIN)
      * (_DUTY_MAX - _DUTY_MIN)
      / (_DIST_MAX - _DIST_MIN);
  }

  myservo.writeMicroseconds((int)servo_duty);


  // 5. Serial Plotter 출력
  Serial.print("Min:");
  Serial.print(_DIST_MIN);

  Serial.print(",dist:");
  Serial.print(dist_raw);

  Serial.print(",ema:");
  Serial.print(dist_ema);

  Serial.print(",Servo:");
  Serial.print(myservo.read());

  Serial.print(",Max:");
  Serial.print(_DIST_MAX);

  Serial.println("");


  last_sampling_time += INTERVAL;
}


// ultrasonic distance measurement
float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);

  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE;
}
