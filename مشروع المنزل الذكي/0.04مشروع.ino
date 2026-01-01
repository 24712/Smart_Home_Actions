//created by I24712I
//Goin me in Git-Hub
//Engoy
#include <Servo.h>         
// #include <SoftwareSerial.h> // تم إلغاء هذه المكتبة مؤقتاً
#include <NewPing.h>       

// ******* 1. تعريف أطراف الدوائر *******

// البلوتوث HC-05 (تم تعليق الأطراف)
// const int BT_RX_PIN = 2; 
// const int BT_TX_PIN = 3; 

// مستشعر المسافة HC-SR04
const int TRIGGER_PIN = 10; 
const int ECHO_PIN = 11;    

// محرك السيرفو 
const int SERVO_PIN = 9; 

// الزر الضاغط والـ LEDs 
const int BUTTON_PIN = 7;    
const int LED_OPEN_PIN = 4;  
const int LED_CLOSE_PIN = 5; 

// الجرس الصوتي للتنبيه الأمني 
const int BUZZER_PIN = 8; 

// ******* 2. تعريف الثوابت *******
const int MAX_DISTANCE = 200; 
const int OPEN_ANGLE = 170;   
const int CLOSE_ANGLE = 10;   
const int SAFETY_THRESHOLD = 5; 

// ******* 3. إنشاء الكائنات *******
Servo windowServo;
// SoftwareSerial bluetooth(BT_RX_PIN, BT_TX_PIN); // تم تعليق الكائن مؤقتاً
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// ******* 4. المتغيرات الحالة *******
bool isWindowOpen = false; 

// **********************************************
// ************** الدالة setup ******************
// **********************************************
void setup() {
  Serial.begin(9600);
  // bluetooth.begin(9600); // تم تعليق تشغيل البلوتوث

  // تهيئة أطراف المخرجات
  pinMode(LED_OPEN_PIN, OUTPUT);
  pinMode(LED_CLOSE_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // تهيئة أطراف المدخلات
  pinMode(BUTTON_PIN, INPUT);

  // توصيل السيرفو بالطرف المحدد
  windowServo.attach(SERVO_PIN);

  // ⭐️⭐️⭐️ اختبار الـ LED: وميض واحد طويل للتحقق من التنفيذ ⭐️⭐️⭐️
  digitalWrite(LED_CLOSE_PIN, HIGH); // تشغيل LED الأحمر
  delay(1000); // انتظر ثانية كاملة
  digitalWrite(LED_CLOSE_PIN, LOW);  // إطفاء LED الأحمر

  // إغلاق النافذة (LED أحمر مضاء)
  setWindowState(CLOSE_ANGLE);
  Serial.println("System Initialized.");
}

// **********************************************
// ************** الدالة loop *******************
// **********************************************
void loop() {
  // handleBluetoothControl(); // تم تعليق وظيفة البلوتوث مؤقتاً
  handleManualButton();     
  checkSafetyDistance();    
}

// **********************************************
// ************ تعريف الدوال المساعدة ***********
// **********************************************

void setWindowState(int angle) {
  digitalWrite(BUZZER_PIN, LOW);

  if (angle == OPEN_ANGLE) {
    isWindowOpen = true;
    digitalWrite(LED_OPEN_PIN, HIGH);  
    digitalWrite(LED_CLOSE_PIN, LOW);
  } else if (angle == CLOSE_ANGLE) {
    isWindowOpen = false;
    digitalWrite(LED_OPEN_PIN, LOW);
    digitalWrite(LED_CLOSE_PIN, HIGH); 
  }
  windowServo.write(angle);
}

// تم تعليق الدالة بالكامل
/*
void handleBluetoothControl() {
  if (bluetooth.available()) {
    char command = bluetooth.read();
    
    if (command == '1') {
      setWindowState(OPEN_ANGLE);
    } else if (command == '0') {
      setWindowState(CLOSE_ANGLE);
    }
  }
}
*/

void handleManualButton() {
  if (digitalRead(BUTTON_PIN) == HIGH) {
    delay(200); 

    if (isWindowOpen) {
      setWindowState(CLOSE_ANGLE);
    } else {
      setWindowState(OPEN_ANGLE);
    }

    while (digitalRead(BUTTON_PIN) == HIGH) {}
  }
}

void checkSafetyDistance() {
  if (!isWindowOpen) {
    long duration = sonar.ping_cm(); 

    if (duration > 1 && duration <= SAFETY_THRESHOLD) {
      windowServo.write(windowServo.read()); 
      digitalWrite(BUZZER_PIN, HIGH); 
    } else {
      digitalWrite(BUZZER_PIN, LOW);
    }
  }
}
