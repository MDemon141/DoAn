#include <BluetoothSerial.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>


#define TRIG_PIN 18 // Chân Trigger của cảm biến được kết nối với GPIO 18
#define ECHO_PIN 19 // Chân Echo của cảm biến được kết nối với GPIO 19
const char* color = "green";
const char* icolor = "none";
const char* state ="none";
// int speed = 0;
long ldis, rdis;
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

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
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(ir1, INPUT);
  pinMode(ir2, INPUT);
  pinMode(ir3, INPUT);
  pinMode(ir4, INPUT);
  pinMode(ir5, INPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, LOW);
  pinMode(ENB, LOW);

  Serial.begin(9600); // Bắt đầu giao tiếp Serial với máy tính
  SerialBT.begin("ESP32_LF_Car"); // Tên thiết bị Bluetooth
  Serial.println("The device started, now you can pair it with Bluetooth!");

  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
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
  // Setup màu từ cảm biến màu
  uint16_t clear, red, green, blue;

  tcs.getRawData(&red, &green, &blue, &clear);

  Serial.print("C: "); Serial.print(clear);
  Serial.print(" R: "); Serial.print(red);
  Serial.print(" G: "); Serial.print(green);
  Serial.print(" B: "); Serial.println(blue);

  if((0.7*red > blue)&& (0.7*red > green)) {
    color = "red";
  }
  if((0.7*green > blue)&& (0.7*green > red)) {
    color = "green";
  }
  if((0.7*blue > red)&& (0.7*blue > green)) {
    color = "blue";
  }
  if((0.7*red > blue)&& (0.7*green > blue)) {
    color = "yellow";
  }

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
  
  if (color == "red") { 
    stop();
    state ="stop"; 
    speed = 0;
  }
  if (color == "green") { 
    state = "forward";
    baseSpeed = 80;
    // Kp = 0.0;
    // Ki = 0.0;
    // Kd = 5.0;
  }
  if (color == "yellow") { 
    state = "forward";
    baseSpeed = 55;
    // Kp = 28.0;
    // Ki = 0.0;
    // Kd = 5.0;

  }

  if (color != "red") {
    long fdis = readDistance();
    Serial.print("Distance: ");
    Serial.print(fdis);
    Serial.println(" cm");
    if (baseSpeed == 55) {
        if (fdis <20) {
        stop();
        state = "stop";
        delay(1000);
        turnL(80, 80);
        delay(250); 
        stop();
        ldis = readDistance();
        delay(1000);
        turnR(80, 80);
        delay(250); 
        stop();
        delay(1000);
        turnR(80, 80);
        delay(250); 
        stop();
        rdis = readDistance();
        delay(1000);
        turnL(80, 80);
        delay(260); 
        stop();
        Serial.println(ldis);
        Serial.println(rdis);
        delay(1000);

        if (ldis >10 && ldis >= rdis ) {
          goLeft();
        }
        else if (rdis > 10 && rdis > ldis) {
          goRight();
        }
      }
    }


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
  }
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
long readDistance() {
  digitalWrite(TRIG_PIN, LOW); // Tắt chân Trigger
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); // Gửi xung siêu âm
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH); 
  long distance = (duration * 0.0343) / 2;

  return distance;
}