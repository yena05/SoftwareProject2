#include <Servo.h>

#define PIN_SERVO 10

// Servo calibration
#define _DUTY_MIN 530
#define _DUTY_NEU 1465
#define _DUTY_MAX 2400

#define INTERVAL 20

Servo myservo;

unsigned long last_update = 0;
unsigned long last_print = 0;
unsigned long phase_start_time = 0;

float duty_curr;

// mode 0 : TEST 1, 3 deg/s, 0 -> 180
// mode 1 : TEST 2, 0.3 deg/s, 180 -> 90
// mode 2 : finished
int mode = 0;


// 테스트 시작을 눈으로 구분하기 위한 빠른 움직임
void signalMove(int count)
{
  for (int i = 0; i < count; i++) {
    myservo.writeMicroseconds(_DUTY_NEU - 150);
    delay(150);

    myservo.writeMicroseconds(_DUTY_NEU + 150);
    delay(150);
  }

  myservo.writeMicroseconds(_DUTY_NEU);
  delay(300);
}


void setup()
{
  myservo.attach(PIN_SERVO);

  Serial.begin(57600);

  // =========================
  // TEST 1 시작 신호
  // 빠르게 좌우 2번
  // =========================
  signalMove(2);

  // TEST 1 시작 위치 : 0도
  duty_curr = _DUTY_MIN;
  myservo.writeMicroseconds((int)duty_curr);
  delay(500);

  // 실제 측정 시작 시간
  phase_start_time = millis();
  last_update = millis();
  last_print = millis();

  Serial.println("==============================");
  Serial.println("TEST 1 START");
  Serial.println("Speed : 3 deg/s");
  Serial.println("0 deg -> 180 deg");
  Serial.println("Expected time : 60 sec");
  Serial.println("==============================");
}


void loop()
{
  unsigned long now = millis();

  // 20ms마다 갱신
  if (now < last_update + INTERVAL)
    return;

  last_update += INTERVAL;


  // =====================================
  // TEST 1
  // 3 deg/s : 0 -> 180 degree
  // =====================================
  if (mode == 0) {

    float duty_change =
      (_DUTY_MAX - _DUTY_MIN)
      * (3.0 / 180.0)
      * (INTERVAL / 1000.0);

    duty_curr += duty_change;


    // 180도 도착
    if (duty_curr >= _DUTY_MAX) {

      duty_curr = _DUTY_MAX;
      myservo.writeMicroseconds((int)duty_curr);

      Serial.println();
      Serial.println("===== TEST 1 FINISHED =====");
      Serial.print("Elapsed time : ");
      Serial.print((now - phase_start_time) / 1000.0);
      Serial.println(" sec");


      // =========================
      // TEST 2 시작 신호
      // 빠르게 좌우 3번
      // =========================
      signalMove(3);


      // TEST 2 시작 위치 : 180도
      duty_curr = _DUTY_MAX;
      myservo.writeMicroseconds((int)duty_curr);
      delay(500);


      // TEST 2 시작
      mode = 1;

      phase_start_time = millis();
      last_update = millis();
      last_print = millis();

      Serial.println();
      Serial.println("==============================");
      Serial.println("TEST 2 START");
      Serial.println("Speed : 0.3 deg/s");
      Serial.println("180 deg -> 90 deg");
      Serial.println("Expected time : 300 sec");
      Serial.println("==============================");

      return;
    }
  }


  // =====================================
  // TEST 2
  // 0.3 deg/s : 180 -> 90 degree
  // =====================================
  else if (mode == 1) {

    float duty_change =
      (_DUTY_MAX - _DUTY_MIN)
      * (0.3 / 180.0)
      * (INTERVAL / 1000.0);

    duty_curr -= duty_change;


    // 90도 도착
    if (duty_curr <= _DUTY_NEU) {

      duty_curr = _DUTY_NEU;
      mode = 2;

      Serial.println();
      Serial.println("===== TEST 2 FINISHED =====");
      Serial.print("Elapsed time : ");
      Serial.print((now - phase_start_time) / 1000.0);
      Serial.println(" sec");

      Serial.println("===== ALL TEST FINISHED =====");
    }
  }


  // 실제 서보에 정수 duty 전달
  myservo.writeMicroseconds((int)duty_curr);


  // =====================================
  // 1초마다 Serial Monitor 출력
  // =====================================
  if ((mode != 2) && (now >= last_print + 1000)) {

    last_print += 1000;

    float angle =
      (duty_curr - _DUTY_MIN)
      * 180.0
      / (_DUTY_MAX - _DUTY_MIN);


    Serial.print("TEST:");

    if (mode == 0)
      Serial.print("1 (3 deg/s)");
    else
      Serial.print("2 (0.3 deg/s)");


    Serial.print(", elapsed:");
    Serial.print((now - phase_start_time) / 1000.0);
    Serial.print(" sec");

    Serial.print(", angle:");
    Serial.print(angle, 2);

    Serial.print(", duty:");
    Serial.println(duty_curr, 3);
  }
} 
