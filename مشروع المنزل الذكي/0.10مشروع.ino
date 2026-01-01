//created by I24712I
//Goin me in Git-Hub
//Engoy#include <Servo.h>         
#include <SoftwareSerial.h> 
#include <NewPing.h>       

// ******* 1. تعريف أطراف الدوائر *******
const int BT_RX_PIN = 2; 
const int BT_TX_PIN = 3; 
const int TRIGGER_PIN = 10; 
const int ECHO_PIN = 11;    
const int SERVO_PIN = 9; 
const int BUTTON_PIN = 7;    
const int LED_OPEN_PIN = 4;  
const int LED_CLOSE_PIN = 5; 
const int BUZZER_PIN = 8; 

// ******* 2. تعريف الثوابت *******
const int MAX_DISTANCE = 200; 
const int OPEN_ANGLE = 170;   
const int CLOSE_ANGLE = 10;   
const int SAFETY_THRESHOLD = 5; 

// ******* 3. إنشاء الكائنات (Instances) *******
Servo windowServo;
SoftwareSerial bluetooth(BT_RX_PIN, BT_TX_PIN); 
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// ******* 4. المتغيرات الحالة *******
bool isWindowOpen = false; 

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
  
  // ⭐️⭐️⭐️ تعديل حاسم: تفعيل مقاومة السحب الداخلية ⭐️⭐️⭐️
  pinMode(BUTTON_PIN, INPUT_PULLUP); 

  windowServo.attach(SERVO_PIN);

  // وميض للتأكيد
  digitalWrite(LED_CLOSE_PIN, HIGH); 
  delay(500);
  digitalWrite(LED_CLOSE_PIN, LOW);  
  delay(500);

  // إغلاق النافذة في البداية
  setWindowState(CLOSE_ANGLE);
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

void handleManualButton() {
  // ⭐️⭐️⭐️ تعديل حاسم: الآن نختبر إذا كانت القراءة LOW ⭐️⭐️⭐️
  if (digitalRead(BUTTON_PIN) == LOW) { 
    delay(200); 

    if (isWindowOpen) {
      setWindowState(CLOSE_ANGLE);
    } else {
      setWindowState(OPEN_ANGLE);
    }

    // الانتظار حتى يتم رفع اليد عن الزر
    while (digitalRead(BUTTON_PIN) == LOW) {}
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
