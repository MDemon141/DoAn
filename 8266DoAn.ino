#include <SoftwareSerial.h>
#define RX_PIN 4 // Chân RX của EspSoftwareSerial
#define TX_PIN 5
#include <Firebase_ESP_Client.h>
#include <ESP8266WiFi.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define API_KEY "AIzaSyDMaokD3SIFf-FZ--pAu8bhgNLmELBeFR4"
#define DATABASE_URL "https://doan-141-default-rtdb.firebaseio.com"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;
String pmode = "auto";
String mode = "auto";

SoftwareSerial mySerial(RX_PIN, TX_PIN);

void setup() {
  Serial.begin(115200); // Khởi tạo cổng nối tiếp cứng với tốc độ baud là 9600
  mySerial.begin(9600); 
  WiFi.begin("Sy Minh", "minh140102");
  while((!(WiFi.status()==WL_CONNECTED))){
    delay(200);
    Serial.print("..");
  } 
  Serial.println("Connected");
  
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("Firebase sign up successful");
    signupOK = true;
  } else {
    Serial.println("Firebase sign up failed");
  }
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Firebase.RTDB.setString(&fbdo, "/color","green");
  Firebase.RTDB.setInt(&fbdo, "/baseSpeed",80);
  Firebase.RTDB.setString(&fbdo, "/state","forward");
}

void loop() {
  if (Firebase.RTDB.getString(&fbdo, "/mode")) {
    if (fbdo.dataType() == "string") {
      mode = fbdo.stringData();
      // Serial.println(mode);
    }
  }
  if (mode != pmode) {
    mySerial.print(mode);
    pmode = mode;
  }
   if (mySerial.available() > 0) {
    String data = mySerial.readStringUntil('\n'); // Đọc chuỗi dữ liệu cho đến khi gặp ký tự 
    int firstComma = data.indexOf(','); // Vị trí dấu phẩy đầu tiên
    int secondComma = data.indexOf(',', firstComma + 1); // Vị trí dấu phẩy thứ hai
    if (firstComma > 0 && secondComma > firstComma) {
      String baseSpeedStr = data.substring(0, firstComma);
      String color = data.substring(firstComma + 1, secondComma);
      String state = data.substring(secondComma + 1);
      int baseSpeed = baseSpeedStr.toInt();  
      state.replace("\n", "");
      state.replace("\r", "");
      state.replace("\"", "\\\""); 
      Firebase.RTDB.setString(&fbdo, "/color",color);
      Firebase.RTDB.setInt(&fbdo, "/baseSpeed",baseSpeed);
      Firebase.RTDB.setString(&fbdo, "/state", state);
    }
  }
}

