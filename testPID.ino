#include <BluetoothSerial.h>

// Định nghĩa các chân kết nối với cảm biến
#define ir1 14
#define ir2 27
#define ir3 26
#define ir4 25
#define ir5 33

// Định nghĩa các chân điều khiển động cơ
#define IN1 17  // Right Motor MA1
#define IN2 16  // Right Motor MA2
#define IN3 4   // Left Motor MB1
#define IN4 2   // Left Motor MB2
#define ENA 5   // Right Motor Enable Pin EA
#define ENB 15  // Left Motor Enable Pin EB

BluetoothSerial SerialBT;

double Kp = 28.0;
double Ki = 0.0;
double Kd = 5.0;
int baseSpeed = 55;

double previousError = 0;
double integral = 0;

void setup() {
  pinMode(ir1, INPUT);
  pinMode(ir2, INPUT);
  pinMode(ir3, INPUT);
  pinMode(ir4, INPUT);
  pinMode(ir5, INPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  Serial.begin(9600); // Bắt đầu giao tiếp Serial với máy tính
  SerialBT.begin("ESP32_LF_Car"); // Tên thiết bị Bluetooth
  Serial.println("The device started, now you can pair it with Bluetooth!");
}
void sharpLeftTurn() {
  /*The pin numbers and high, low values might be different depending on your connections */
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}
void sharpRightTurn() {
  /*The pin numbers and high, low values might be different depending on your connections */
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

// Hàm tính toán lỗi (error)
int calculateError(int s1, int s2, int s3, int s4, int s5) {
  // 
  if (s1 == 0 && s2 == 1 && s3 == 1 && s4 == 1 && s5 == 1) return 4;
  if (s1 == 0 && s2 == 0 && s3 == 1 && s4 == 1 && s5 == 1) return 3;
  if (s1 == 1 && s2 == 0 && s3 == 1 && s4 == 1 && s5 == 1) return 2;
  if (s1 == 1 && s2 == 0 && s3 == 0 && s4 == 1 && s5 == 1) return 1;
  if (s1 == 1 && s2 == 1 && s3 == 0 && s4 == 1 && s5 == 1) return 0;
  if (s1 == 1 && s2 == 1 && s3 == 0 && s4 == 0 && s5 == 1) return -1;
  if (s1 == 1 && s2 == 1 && s3 == 1 && s4 == 0 && s5 == 1) return -2;
  if (s1 == 1 && s2 == 1 && s3 == 1 && s4 == 0 && s5 == 0) return -3;
  if (s1 == 1 && s2 == 1 && s3 == 1 && s4 == 1 && s5 == 0) return -4;
  if (s1 == 0 && s2 == 0 && s3 == 0 && s4 == 0 && s5 == 0) return -5;
  return 0;
}

double PIDCal(int error) {
  integral += error;
  double derivative = error - previousError;
  double output = Kp * error + Ki * integral + Kd * derivative;
  previousError = error;
  return output;
}

void loop() {
  // Nhận giá trị PID và tốc độ từ Bluetooth
  if (SerialBT.available()) {
    String receivedData = SerialBT.readStringUntil('\n');
    char identifier = receivedData.charAt(0);
    double value = receivedData.substring(1).toFloat();

    switch (identifier) {
      case 'P':
        Kp = value;
        break;
      case 'I':
        Ki = value;
        break;
      case 'D':
        Kd = value;
        break;
      case 'S':
        baseSpeed = value;
        break;
    }

    Serial.print("Kp: ");
    Serial.print(Kp);
    Serial.print(" Ki: ");
    Serial.print(Ki);
    Serial.print(" Kd: ");
    Serial.print(Kd);
    Serial.print(" Base Speed: ");
    Serial.println(baseSpeed);
  }

  // Đọc giá trị từ cảm biến
  int s1 = digitalRead(ir1);
  int s2 = digitalRead(ir2);
  int s3 = digitalRead(ir3);
  int s4 = digitalRead(ir4);
  int s5 = digitalRead(ir5);

  // Tính toán lỗi
  int error = calculateError(s1, s2, s3, s4, s5);
  if (error == -5) {
    stop();
  }

  else {
  double output = PIDCal(error);

    // Tính toán tốc độ của các động cơ
    int speedLeft = baseSpeed - output;
    int speedRight = baseSpeed + output;

    speedLeft = constrain(speedLeft, 0, 255);
    speedRight = constrain(speedRight, 0, 255);

    // Điều khiển động cơ
    analogWrite(ENA, speedLeft);
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

    analogWrite(ENB, speedRight);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    Serial.print(output);
    Serial.print(" + ");
    Serial.print(speedLeft); 
    Serial.print(" + ");
    Serial.println(speedRight); 
  }
  delay(20);
}
void forward(int s) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  // Tốc độ có thể điều chỉnh bằng PWM trên ENA và ENB
  analogWrite(ENA, s);
  analogWrite(ENB, s);
}

// Hàm điều khiển lùi
void stop() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  // Tốc độ có thể điều chỉnh bằng PWM trên ENA và ENB
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}
void turnL(int s) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  // Tốc độ có thể điều chỉnh bằng PWM trên ENA và ENB
  analogWrite(ENB, s);
}
void turnR(int s) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  // Tốc độ có thể điều chỉnh bằng PWM trên ENA và ENB
  analogWrite(ENA, s);
}