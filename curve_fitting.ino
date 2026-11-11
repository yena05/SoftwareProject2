// Arduino pin assignment
#define PIN_IR A0

#define NUM_POINTS 7
#define MAX_DEGREE 6

// 실제 측정 거리 [cm]
float distance_value[NUM_POINTS] = {
  0, 5, 10, 15, 20, 25, 30
};

// 각 거리에서 측정한 ADC 값
float adc_value[NUM_POINTS];


// =====================================================
// 함수 선언
// =====================================================
unsigned int readNumber();
void waitEnter();

int compare(const void *a, const void *b);

unsigned int ir_sensor_filtered(
  unsigned int n,
  float position,
  int verbose
);

bool gaussianElimination(
  float A[MAX_DEGREE + 1][MAX_DEGREE + 2],
  float result[MAX_DEGREE + 1],
  int n
);

bool polynomialFit(
  float x[],
  float y[],
  int count,
  int degree,
  float coeff[]
);

void printEquation(
  float coeff[],
  int degree
);


// =====================================================
// setup
// =====================================================
void setup()
{
  Serial.begin(1000000);

  unsigned int sample_n;
  unsigned int degree;

  Serial.println();
  Serial.println("=== IR Sensor Curve Fitting ===");


  // Median filter n 입력
  Serial.println("Enter median filter n (1 ~ 100):");

  sample_n = readNumber();

  if (sample_n < 1)
    sample_n = 1;

  if (sample_n > 100)
    sample_n = 100;


  // 다항식 차수 입력
  Serial.println("Enter polynomial degree (1 ~ 6):");

  degree = readNumber();

  if (degree < 1)
    degree = 1;

  if (degree > MAX_DEGREE)
    degree = MAX_DEGREE;


  Serial.print("Median n = ");
  Serial.println(sample_n);

  Serial.print("Degree = ");
  Serial.println(degree);


  // ===================================================
  // 0, 5, 10, ..., 30 cm 측정
  // ===================================================
  for (int i = 0; i < NUM_POINTS; i++) {

    Serial.println();

    Serial.print("Place ball at ");
    Serial.print(distance_value[i], 0);
    Serial.println(" cm");

    Serial.println("Press ENTER to measure.");


    // Enter 기다림
    waitEnter();


    // Median filter 적용
    unsigned int filtered =
      ir_sensor_filtered(sample_n, 0.5, 0);


    // ADC 값 저장
    adc_value[i] = filtered;


    Serial.print("FLT: ");
    Serial.println(filtered);
  }


  // ===================================================
  // 측정 결과 출력
  // ===================================================
  Serial.println();
  Serial.println("=== Measured Data ===");

  for (int i = 0; i < NUM_POINTS; i++) {

    Serial.print(distance_value[i], 0);
    Serial.print(" cm -> ADC = ");
    Serial.println(adc_value[i], 0);
  }


  // ===================================================
  // Curve fitting
  // ===================================================
  float coeff[MAX_DEGREE + 1] = {0};


  bool success =
    polynomialFit(
      adc_value,
      distance_value,
      NUM_POINTS,
      degree,
      coeff
    );


  // 결과 출력
  if (success) {

    printEquation(coeff, degree);
  }
  else {

    Serial.println();
    Serial.println("Curve fitting failed.");
  }
}


// setup에서 한 번만 실행
void loop()
{
}


// =====================================================
// 숫자 입력
// =====================================================
unsigned int readNumber()
{
  while (Serial.available() == 0)
    ;

  unsigned int value = Serial.parseInt();

  // 남아 있는 입력 제거
  while (Serial.available() > 0)
    Serial.read();

  delay(100);

  return value;
}


// =====================================================
// Enter 입력 기다리기
// =====================================================
void waitEnter()
{
  // 이전 입력 제거
  while (Serial.available() > 0)
    Serial.read();

  // 새로운 입력 기다림
  while (Serial.available() == 0)
    ;

  // 입력 제거
  while (Serial.available() > 0)
    Serial.read();

  delay(100);
}


// =====================================================
// qsort 비교 함수
// =====================================================
int compare(const void *a, const void *b)
{
  return (*(unsigned int *)a - *(unsigned int *)b);
}


// =====================================================
// IR sensor median filter
// =====================================================
unsigned int ir_sensor_filtered(
  unsigned int n,
  float position,
  int verbose
)
{
  unsigned int *ir_val, ret_val;
  unsigned long start_time;


  if (verbose >= 2)
    start_time = millis();


  if ((n == 0) ||
      (n > 100) ||
      (position < 0.0) ||
      (position > 1.0))
  {
    return 0;
  }


  if (position == 1.0)
    position = 0.999;


  if (verbose == 1) {

    Serial.print("n: ");
    Serial.print(n);

    Serial.print(", position: ");
    Serial.print(position);

    Serial.print(", ret_idx: ");
    Serial.println((unsigned int)(n * position));
  }


  // n개 측정값을 저장할 메모리 생성
  ir_val =
    (unsigned int *)malloc(sizeof(unsigned int) * n);


  if (ir_val == NULL)
    return 0;


  if (verbose == 1)
    Serial.print("IR:");


  // n번 측정
  for (int i = 0; i < n; i++) {

    ir_val[i] = analogRead(PIN_IR);


    if (verbose == 1) {

      Serial.print(" ");
      Serial.print(ir_val[i]);
    }
  }


  if (verbose == 1)
    Serial.print(" => ");


  // 작은 값부터 정렬
  qsort(
    ir_val,
    n,
    sizeof(unsigned int),
    compare
  );


  // position 위치의 값 선택
  // 0.5면 중앙값
  ret_val =
    ir_val[(unsigned int)(n * position)];


  if (verbose == 1) {

    for (int i = 0; i < n; i++) {

      Serial.print(" ");
      Serial.print(ir_val[i]);
    }

    Serial.print(" :: ");
    Serial.println(ret_val);
  }


  free(ir_val);


  if (verbose >= 2) {

    Serial.print("Elapsed time: ");
    Serial.print(millis() - start_time);
    Serial.println("ms");
  }


  return ret_val;
}


// =====================================================
// Gaussian Elimination
//
// polynomialFit()에서 만들어진 연립방정식을 풀어서
// c0, c1, c2 ... 계수를 구함
// =====================================================
bool gaussianElimination(
  float A[MAX_DEGREE + 1][MAX_DEGREE + 2],
  float result[MAX_DEGREE + 1],
  int n
)
{
  // Forward elimination
  for (int i = 0; i < n; i++) {

    // Pivot 선택
    int maxRow = i;

    for (int k = i + 1; k < n; k++) {

      if (fabs(A[k][i]) > fabs(A[maxRow][i])) {

        maxRow = k;
      }
    }


    // 행 교환
    if (maxRow != i) {

      for (int j = 0; j <= n; j++) {

        float temp = A[i][j];

        A[i][j] = A[maxRow][j];
        A[maxRow][j] = temp;
      }
    }


    // 계산 불가능
    if (fabs(A[i][i]) < 0.000001)
      return false;


    // 아래 행 제거
    for (int k = i + 1; k < n; k++) {

      float factor =
        A[k][i] / A[i][i];


      for (int j = i; j <= n; j++) {

        A[k][j] -= factor * A[i][j];
      }
    }
  }


  // Back substitution
  for (int i = n - 1; i >= 0; i--) {

    result[i] = A[i][n];


    for (int j = i + 1; j < n; j++) {

      result[i] -=
        A[i][j] * result[j];
    }


    result[i] /= A[i][i];
  }


  return true;
}


// =====================================================
// Polynomial Curve Fitting
//
// 1차:
// y = c0 + c1*x
//
// 2차:
// y = c0 + c1*x + c2*x^2
//
// ...
//
// 6차:
// y = c0 + c1*x + ... + c6*x^6
//
// x = ADC
// y = 거리(cm)
//
// 최소제곱법으로 c0, c1 ... 계산
// =====================================================
bool polynomialFit(
  float x[],
  float y[],
  int count,
  int degree,
  float coeff[]
)
{
  // 미지수 개수
  int n = degree + 1;


  // 연립방정식 저장
  float A[MAX_DEGREE + 1][MAX_DEGREE + 2] = {0};


  // Normal Equation 생성
  for (int row = 0; row < n; row++) {

    for (int col = 0; col < n; col++) {

      float sum = 0.0;


      for (int i = 0; i < count; i++) {

        sum += pow(x[i], row + col);
      }


      A[row][col] = sum;
    }


    float sum_y = 0.0;


    for (int i = 0; i < count; i++) {

      sum_y +=
        y[i] * pow(x[i], row);
    }


    A[row][n] = sum_y;
  }


  // 연립방정식을 풀어서 계수 계산
  return gaussianElimination(
    A,
    coeff,
    n
  );
}


// =====================================================
// 계산된 식 출력
// =====================================================
void printEquation(
  float coeff[],
  int degree
)
{
  Serial.println();
  Serial.println("=== Curve Fitting Result ===");


  // 각 계수 출력
  for (int i = 0; i <= degree; i++) {

    Serial.print("c");
    Serial.print(i);
    Serial.print(" = ");

    Serial.println(coeff[i], 10);
  }


  Serial.println();
  Serial.println("Arduino executable equation:");

  Serial.println(
    "float volt_to_distance(unsigned int a_value)"
  );

  Serial.println("{");

  Serial.print("  return ");


  // 계산된 계수를 이용해 식 출력
  for (int i = 0; i <= degree; i++) {

    // 부호
    if (i > 0) {

      if (coeff[i] >= 0)
        Serial.print(" + ");
      else
        Serial.print(" - ");
    }

    else {

      if (coeff[i] < 0)
        Serial.print("-");
    }


    // 계수
    Serial.print(fabs(coeff[i]), 10);


    // a_value^1
    if (i == 1) {

      Serial.print(" * a_value");
    }


    // a_value^2 이상
    else if (i >= 2) {

      for (int j = 0; j < i; j++) {

        Serial.print(" * a_value");
      }
    }
  }


  Serial.println(";");

  Serial.println("}");
}
