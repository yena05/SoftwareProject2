// Arduino pin assignment
#define PIN_LED  9
#define PIN_TRIG 12
#define PIN_ECHO 13

// configurable parameters
#define SND_VEL 346.0
#define INTERVAL 25
#define PULSE_DURATION 10
#define _DIST_MIN 100
#define _DIST_MAX 300

#define TIMEOUT ((INTERVAL / 2) * 1000.0)
#define SCALE (0.001 * 0.5 * SND_VEL)

#define _EMA_ALPHA 0.5

// Median filter size
#define MEDIAN_N 30
// N = 3, 10, 30 으로 변경하면서 실험

// global variables
unsigned long last_sampling_time;

float dist_ema;
bool ema_first = true;

// 최근 N개 샘플 저장
float samples[MEDIAN_N];
int sample_count = 0;
int sample_index = 0;


void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  digitalWrite(PIN_TRIG, LOW);

  Serial.begin(57600);
}


void loop() {
  float dist_raw;
  float dist_median;

  // wait until next sampling time
  if (millis() < last_sampling_time + INTERVAL)
    return;

  // raw distance
  dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);


  // EMA
  if (ema_first) {
    dist_ema = dist_raw;
    ema_first = false;
  }
  else {
    dist_ema = _EMA_ALPHA * dist_raw
             + (1.0 - _EMA_ALPHA) * dist_ema;
  }


  // 최근 N개 샘플 저장
  samples[sample_index] = dist_raw;

  sample_index++;
  if (sample_index >= MEDIAN_N)
    sample_index = 0;

  if (sample_count < MEDIAN_N)
    sample_count++;


  // Median 계산
  dist_median = getMedian(samples, sample_count);


  // Serial Plotter output
  Serial.print("Min:");
  Serial.print(_DIST_MIN);

  Serial.print(",raw:");
  Serial.print(dist_raw);

  Serial.print(",ema:");
  Serial.print(dist_ema);

  Serial.print(",median:");
  Serial.print(dist_median);

  Serial.print(",Max:");
  Serial.print(_DIST_MAX);

  Serial.println("");


  // LED control
  if ((dist_raw < _DIST_MIN) || (dist_raw > _DIST_MAX))
    digitalWrite(PIN_LED, 1);
  else
    digitalWrite(PIN_LED, 0);


  last_sampling_time += INTERVAL;
}


// Median 계산
float getMedian(float data[], int count)
{
  float temp[MEDIAN_N];

  // 원본 배열을 건드리지 않도록 복사
  for (int i = 0; i < count; i++) {
    temp[i] = data[i];
  }

  // 오름차순 정렬
  for (int i = 0; i < count - 1; i++) {
    for (int j = i + 1; j < count; j++) {
      if (temp[i] > temp[j]) {
        float t = temp[i];
        temp[i] = temp[j];
        temp[j] = t;
      }
    }
  }

  // 홀수 개
  if (count % 2 == 1) {
    return temp[count / 2];
  }

  // 짝수 개
  else {
    return (temp[count / 2 - 1] + temp[count / 2]) / 2.0;
  }
}


// ultrasonic distance measurement
float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);

  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE;
}
