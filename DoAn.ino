#include <Wire.h>
#include <Adafruit_TCS34725.h>
#define TRIG_PIN 18 // Chân Trigger của cảm biến được kết nối với GPIO 18
#define ECHO_PIN 19 // Chân Echo của cảm biến được kết nối với GPIO 19

#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#define API_KEY "ABCDEFGH"
#define DATABASE_URL "ABCDEFGH.COM"

FirebaseData fbdo;
FirebaseData stream;

// Authentication data
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;

const char* color = "green";
const char* pcolor = "none";
const char* state ="forward";
const char* pstate ="forward";
String mode ="auto";
String command ="s";

// int speed = 0;
long ldis, rdis;
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_24MS, TCS34725_GAIN_4X);
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

double Kp = 28.0;
double Ki = 0.0;
double Kd = 6.5;
int baseSpeed = 80;


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

  Serial.print("Start connection");
  WiFi.begin("Sy Minh", "minh140102");
  while((!(WiFi.status()==WL_CONNECTED))){
    delay(200);
    Serial.print("..");
  } 
  Serial.println("Connected");
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  if(Firebase.ready() && signupOK) {   
    Firebase.RTDB.setString(&fbdo, "/color",color);
    Firebase.RTDB.setInt(&fbdo, "/baseSpeed",baseSpeed);
    Firebase.RTDB.setString(&fbdo, "/state",state);
    Firebase.RTDB.setString(&fbdo, "/mode",mode);
    Firebase.RTDB.setString(&fbdo, "/command",command);
  }
  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }

}
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
  if (mode == "auto") {
    uint16_t clear, red, green, blue;
    tcs.getRawData(&red, &green, &blue, &clear);
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
    if (color == "red") { 
      stop();
      state ="stop"; 
      baseSpeed = 0;
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
    if (pcolor == "none") {
      pcolor = color;
    }
    if  (pcolor != color || pstate != state) {
      if(Firebase.ready() && signupOK) {   
      Firebase.RTDB.setString(&fbdo, "/color",color);
      Firebase.RTDB.setInt(&fbdo, "/baseSpeed",baseSpeed);
      Firebase.RTDB.setString(&fbdo, "/state",state);
      }
      pcolor = color;
    }
    if (state == "stop") {
      if (Firebase.RTDB.getString(&fbdo, "/mode")) {
        if (fbdo.dataType() == "string") {
          mode = fbdo.stringData();
          Serial.println(mode);
        }
      }
    }
    Serial.println(mode);
    Serial.print(state);
    Serial.print(" + ");
    Serial.print(pcolor);
    Serial.print(" + ");
    Serial.println(color);

    if (color != "red") {
      long fdis = readDistance();
      // Serial.print("Distance: ");
      // Serial.print(fdis);
      // Serial.println(" cm");
      if (baseSpeed == 55) {
        if (fdis <25) {
          stop();
          delay(2000);
          pstate = state;
          state = "obstacle";
          Firebase.RTDB.setString(&fbdo, "/state",state);
          stop();
          delay(1000);
          turnL(152);
          delay(120); 
          stop();
          ldis = readDistance();
          delay(1000);
          turnR(150);
          delay(120); 
          stop();
          delay(1000);
          turnR(150);
          delay(120); 
          stop();
          rdis = readDistance();
          delay(1000);
          turnL(152);
          delay(120); 
          stop();
          Serial.println(ldis);
          Serial.println(rdis);
          delay(1000);
          if (ldis > 20 && ldis >= rdis ) {
            goLeft();
          }
          else if (rdis > 20 && rdis > ldis) {
            goRight();
          }
          state = pstate; 
          Firebase.RTDB.setString(&fbdo, "/state",pstate);
        }
      }
      if (baseSpeed == 80) {
        if (fdis <35) {
          stop();
          delay(2000);
          pstate = state;
          state = "obstacle";
          Firebase.RTDB.setString(&fbdo, "/state",state);
          stop();
          delay(1000);
          turnL(152);
          delay(120); 
          stop();
          ldis = readDistance();
          delay(1000);
          turnR(150);
          delay(120); 
          stop();
          delay(1000);
          turnR(150);
          delay(120); 
          stop();
          rdis = readDistance();
          delay(1000);
          turnL(152);
          delay(120); 
          stop();
          Serial.println(ldis);
          Serial.println(rdis);
          delay(1000);
          if (ldis > 20 && ldis >= rdis ) {
            goLeft();
          }
          else if (rdis > 20 && rdis > ldis) {
            goRight();
          }
          state = pstate; 
          Firebase.RTDB.setString(&fbdo, "/state",pstate);
        }
      }
      int s1 = digitalRead(ir1);
      int s2 = digitalRead(ir2);
      int s3 = digitalRead(ir3);
      int s4 = digitalRead(ir4);
      int s5 = digitalRead(ir5);

      // // Tính toán lỗi
      int error = calculateError(s1, s2, s3, s4, s5);
      if (error == -5) {
        stop();
        if (Firebase.RTDB.getString(&fbdo, "/mode")) {
          if (fbdo.dataType() == "string") {
            mode = fbdo.stringData();
            Serial.println(mode);
          }
        }
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
        Serial.print(baseSpeed);
        Serial.print(" + ");
        Serial.print(output);
        Serial.print(" + ");
        Serial.print(speedLeft); 
        Serial.print(" + ");
        Serial.println(speedRight); 
      }
    }
  }
  else if (mode == "manual") {
    if (Firebase.RTDB.getString(&fbdo, "/command")) {
      if (fbdo.dataType() == "string") {
        command = fbdo.stringData();
        if (command == "f") {
          forward(100);
        } else if (command == "b") {
          backward(100);
        } else if (command == "l") {
          turnL(100);
        } else if (command == "r") {
          turnR(100);
        } else if (command == "s") {
          stop();
        } else if (command == "auto") {
          mode = "auto";
        }
      }
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
void backward(int s) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
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
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  // Tốc độ có thể điều chỉnh bằng PWM trên ENA và ENB
  analogWrite(ENA, s);
  analogWrite(ENB, s);
}
void turnR(int s) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  // Tốc độ có thể điều chỉnh bằng PWM trên ENA và ENB
  analogWrite(ENA, s);
  analogWrite(ENB, s);
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
void goLeft() {
  turnL(90);
  delay(300);
  forward(100);
  delay(400);
  turnR(90);
  delay(290);
  forward(100);
  delay(200);
  turnR(90);
  delay(300);
  forward(100);
  delay(200);
  turnL(90);
  delay(360);
  stop();
  delay(1000); 
}
void goRight() {
  turnR(90);
  delay(300);
  forward(100);
  delay(350);
  turnL(90);
  delay(300);
  forward(100);
  delay(200);
  turnL(90);
  delay(300);
  forward(100);
  delay(460);
  turnR(90);
  delay(130);
  stop();
  delay(1000);
}
