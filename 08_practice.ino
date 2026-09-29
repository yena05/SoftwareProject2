// Arduino pin assignment
#define PIN_LED  9
#define PIN_TRIG 12   // sonar sensor TRIGGER
#define PIN_ECHO 13   // sonar sensor ECHO

// configurable parameters
#define SND_VEL 346.0     // sound velocity at 24 celsius degree (unit: m/sec)
#define INTERVAL 25      // sampling interval (unit: msec)
#define PULSE_DURATION 10 // ultra-sound Pulse Duration (unit: usec)
#define _DIST_MIN 100.0   // minimum distance to be measured (unit: mm)
#define _DIST_MID 200.0   // maximum brightness distance (unit: mm)
#define _DIST_MAX 300.0   // maximum distance to be measured (unit: mm)

#define TIMEOUT ((INTERVAL / 2) * 1000.0)
#define SCALE (0.001 * 0.5 * SND_VEL)

unsigned long last_sampling_time;   // unit: msec

void setup() {
  // initialize GPIO pins
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);  // sonar TRIGGER
  pinMode(PIN_ECHO, INPUT);   // sonar ECHO
  digitalWrite(PIN_TRIG, LOW);  // turn-off Sonar

  // initialize serial port
  Serial.begin(57600);
}

void loop() {
  float distance;
  float pwm_value;

  // wait until next sampling time. // polling
  if (millis() < (last_sampling_time + INTERVAL))
    return;

  distance = USS_measure(PIN_TRIG, PIN_ECHO); // read distance


  // 300mm 초과 또는 측정 실패
  if ((distance == 0.0) || (distance > _DIST_MAX)) {

    distance = _DIST_MAX + 10.0;    // 기존과 동일: Plotter에 310 표시
    pwm_value = 255.0;              // LED OFF

  }

  // 100mm 미만
  else if (distance < _DIST_MIN) {

    distance = _DIST_MIN - 10.0;    // 기존과 동일: Plotter에 90 표시
    pwm_value = 255.0;              // LED OFF

  }

  // 100mm ~ 200mm
  else if (distance <= _DIST_MID) {

    // 100mm -> 255 : 최소 밝기
    // 150mm -> 약 128 : 50% 밝기
    // 200mm -> 0 : 최대 밝기
    pwm_value = (_DIST_MID - distance)
                * 255.0
                / (_DIST_MID - _DIST_MIN);

  }

  // 200mm ~ 300mm
  else {

    // 200mm -> 0 : 최대 밝기
    // 250mm -> 약 128 : 50% 밝기
    // 300mm -> 255 : 최소 밝기
    pwm_value = (distance - _DIST_MID)
                * 255.0
                / (_DIST_MAX - _DIST_MID);
  }


  // LED brightness control (active low)
  analogWrite(PIN_LED, (int)pwm_value);


  // 기존 Serial Plotter 출력 그대로
  Serial.print("Min:");
  Serial.print(_DIST_MIN);

  Serial.print(",distance:");
  Serial.print(distance);

  Serial.print(",Max:");
  Serial.print(_DIST_MAX);

  Serial.println("");

  // update last sampling time
  last_sampling_time += INTERVAL;
}


// get a distance reading from USS. return value is in millimeter.
float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);

  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE; // unit: mm

  // Pulse duration to distance conversion example (target distance = 17.3m)
  // - pulseIn(ECHO, HIGH, timeout) returns microseconds (음파의 왕복 시간)
  // - 편도 거리 = (pulseIn() / 1,000,000) * SND_VEL / 2 (미터 단위)
  //   mm 단위로 하려면 * 1,000이 필요 ==> SCALE = 0.001 * 0.5 * SND_VEL
}
