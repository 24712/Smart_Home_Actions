//created by I24712I
//Goin me in Git-Hub
//Engoy
#include <Servo.h>         // للتحكم في محرك السيرفو
#include <SoftwareSerial.h> // للتحكم بالبلوتوث
#include <NewPing.h>       // للتحكم بمستشعر المسافة

// ******* 1. تعريف أطراف الدوائر (مطابقة لتوصيلاتك النهائية) *******

// البلوتوث HC-05 
const int BT_RX_PIN = 2; 
const int BT_TX_PIN = 3; 

// مستشعر المسافة HC-SR04
const int TRIGGER_PIN = 10; // الإرسال (Pin 10)
const int ECHO_PIN = 11;    // الاستقبال (Pin 11)

// محرك السيرفو 
const int SERVO_PIN = 9; // (Pin 9)

// الزر الضاغط والـ LEDs 
const int BUTTON_PIN = 7;    // الزر اليدوي (Pin 7)
const int LED_OPEN_PIN = 4;  // LED أخضر (النافذة مفتوحة)
const int LED_CLOSE_PIN = 5; // LED أحمر (النافذة مغلقة)

// الجرس الصوتي للتنبيه الأمني 
const int BUZZER_PIN = 8; // (Pin 8)

// ******* 2. تعريف الثوابت وقيم الزوايا *******
const int MAX_DISTANCE = 200; 
const int OPEN_ANGLE = 170;   
const int CLOSE_ANGLE = 10;   
// تم تعديل مسافة الأمان إلى 5 سم بناءً على طلبك
const int SAFETY_THRESHOLD = 5; // حد الأمان الجديد (5 سم)

// ******* 3. إنشاء الكائنات (Instances) *******
Servo windowServo;
SoftwareSerial bluetooth(BT_RX_PIN, BT_TX_PIN); 
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// ******* 4. المتغيرات الحالة *******
bool isWindowOpen = false; // مغلقة في البداية

// **********************************************
// ************** الدالة setup ******************
// **********************************************
void setup() {
  Serial.begin(9600);
  bluetooth.begin(9600);

  // تهيئة أطراف المخرجات
  pinMode(LED_OPEN_PIN, OUTPUT);
  pinMode(LED_CLOSE_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // تهيئة أطراف المدخلات
  pinMode(BUTTON_PIN, INPUT);

  windowServo.attach(SERVO_PIN);

  // ⭐️⭐️⭐️ وميض عند التشغيل (3 مرات) ⭐️⭐️⭐️
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_CLOSE_PIN, HIGH); 
    delay(200);
    digitalWrite(LED_CLOSE_PIN, LOW);  
    delay(200);
  }
  
  // إغلاق النافذة في البداية (LED أحمر مضاء)
  setWindowState(CLOSE_ANGLE);
  Serial.println("Smart Window System Initialized. Window is Closed.");
}

// **********************************************
// ************** الدالة loop *******************
// **********************************************
void loop() {
  handleBluetoothControl(); 
  handleManualButton();     
  checkSafetyDistance();    
}

// **********************************************
// ************ تعريف الدوال المساعدة ***********
// **********************************************

// 1. دالة تغيير حالة النافذة (تتحكم في السيرفو والـ LEDs)
void setWindowState(int angle) {
  // إيقاف الجرس في حال كان يعمل
  digitalWrite(BUZZER_PIN, LOW);

  if (angle == OPEN_ANGLE) {
    isWindowOpen = true;
    digitalWrite(LED_OPEN_PIN, HIGH);  // إنارة أخضر
    digitalWrite(LED_CLOSE_PIN, LOW);
    Serial.println("Window is OPEN.");
  } else if (angle == CLOSE_ANGLE) {
    isWindowOpen = false;
    digitalWrite(LED_OPEN_PIN, LOW);
    digitalWrite(LED_CLOSE_PIN, HIGH); // إنارة أحمر
    Serial.println("Window is CLOSED.");
  }
  windowServo.write(angle);
}

// 2. دالة معالجة أوامر البلوتوث (1 للفتح، 0 للإغلاق)
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

// 3. دالة معالجة ضغط الزر اليدوي
void handleManualButton() {
  if (digitalRead(BUTTON_PIN) == HIGH) {
    delay(200); // Debouncing

    // تبديل الحالة
    if (isWindowOpen) {
      setWindowState(CLOSE_ANGLE);
    } else {
      setWindowState(OPEN_ANGLE);
    }

    // الانتظار حتى يتم رفع اليد عن الزر
    while (digitalRead(BUTTON_PIN) == HIGH) {}
  }
}

// 4. دالة التحقق من المسافة (ميزة الأمان)
void checkSafetyDistance() {
  // ميزة الأمان تعمل فقط إذا كانت النافذة مغلقة
  if (!isWindowOpen) {
    long duration = sonar.ping_cm(); 

    // الشرط الجديد: إذا كانت المسافة بين 1 سم و 5 سم 
    if (duration > 1 && duration <= SAFETY_THRESHOLD) {
      
      // إيقاف السيرفو في مكانه وتفعيل الجرس (صفير وغلق النافذة)
      windowServo.write(windowServo.read()); 
      digitalWrite(BUZZER_PIN, HIGH); 

      Serial.print("!! DANGER !! Object detected at ");
      Serial.print(duration);
      Serial.println(" cm. Window close halted, Buzzer ON.");
    } else {
      // إطفاء الجرس في حال عدم وجود خطر
      digitalWrite(BUZZER_PIN, LOW);
    }
  }
}
