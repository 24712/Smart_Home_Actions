//created by I24712I
//Goin me in Git-Hub
//Engoy
#include <Servo.h>         // للتحكم في محرك السيرفو
#include <SoftwareSerial.h> // للتحكم بالبلوتوث عبر أطراف (2 و 3)
#include <NewPing.h>       // للتحكم بمستشعر المسافة (HC-SR04)

// ******* 1. تعريف أطراف الدوائر *******

// البلوتوث HC-05 (توصيل آمن باستخدام مقسم الجهد على Pin 3)
const int BT_RX_PIN = 2; // مستقبل الأردوينو -> يتصل بـ TXD في البلوتوث (مباشر)
const int BT_TX_PIN = 3; // مرسل الأردوينو -> يتصل بـ RXD في البلوتوث (عبر مقسم الجهد)

// مستشعر المسافة HC-SR04
const int TRIGGER_PIN = 9; // إرسال الموجات
const int ECHO_PIN = 8;    // استقبال الموجات

// محرك السيرفو
const int SERVO_PIN = 6;

// الزر الضاغط والـ LEDs (مؤشرات الحالة)
const int BUTTON_PIN = 7; // الزر اليدوي (متصل بمقاومة Pull-down 10kOhm)
const int LED_OPEN_PIN = 4; // LED أخضر (النافذة مفتوحة)
const int LED_CLOSE_PIN = 5; // LED أحمر (النافذة مغلقة)
const int BUZZER_PIN = 10; // الجرس الصوتي للتنبيه الأمني

// ******* 2. تعريف الثوابت وقيم الزوايا *******
const int MAX_DISTANCE = 200; // أقصى مسافة يقيسها المستشعر (2 متر)
const int OPEN_ANGLE = 170; // زاوية الفتح
const int CLOSE_ANGLE = 10; // زاوية الإغلاق
const int SAFETY_THRESHOLD = 20; // حد الأمان (20 سم) لوقف الإغلاق

// ******* 3. إنشاء الكائنات (Instances) *******
Servo windowServo;
SoftwareSerial bluetooth(BT_RX_PIN, BT_TX_PIN); // RX, TX
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// ******* 4. المتغيرات الحالة *******
bool isWindowOpen = false; // لمتابعة حالة النافذة (مغلقة في البداية)

// **********************************************
// ************** الدالة setup ******************
// **********************************************
void setup() {
  // تهيئة الاتصالات التسلسلية
  Serial.begin(9600);    // للمراقبة على الكمبيوتر
  bluetooth.begin(9600); // للاتصال بوحدة HC-05

  // تهيئة أطراف المخرجات (Outputs)
  pinMode(LED_OPEN_PIN, OUTPUT);
  pinMode(LED_CLOSE_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // تهيئة أطراف المدخلات (Inputs)
  pinMode(BUTTON_PIN, INPUT);

  // توصيل السيرفو بالطرف المحدد
  windowServo.attach(SERVO_PIN);

  // إغلاق النافذة في البداية
  setWindowState(CLOSE_ANGLE);
  Serial.println("Smart Window System Initialized. Window is Closed.");
}

// **********************************************
// ************** الدالة loop *******************
// **********************************************
void loop() {
  handleBluetoothControl(); // 1. معالجة أوامر البلوتوث
  handleManualButton();     // 2. معالجة ضغطة الزر اليدوي
  checkSafetyDistance();    // 3. التحقق من الأمان أثناء الإغلاق
}


// **********************************************
// ************ الدوال المساعدة ******************
// **********************************************

// 1. دالة تغيير حالة النافذة (تتحكم في السيرفو والـ LEDs)
void setWindowState(int angle) {
  // إيقاف الجرس في حال كان يعمل
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

// 2. دالة معالجة أوامر البلوتوث (1 للفتح، 0 للإغلاق)
void handleBluetoothControl() {
  if (bluetooth.available()) {
    char command = bluetooth.read();
    Serial.print("Received Command: ");
    Serial.println(command);

    if (command == '1') {
      setWindowState(OPEN_ANGLE);
      Serial.println("Window Opened via Bluetooth.");
    } else if (command == '0') {
      setWindowState(CLOSE_ANGLE);
      Serial.println("Window Closed via Bluetooth.");
    }
  }
}

// 3. دالة معالجة ضغط الزر اليدوي
void handleManualButton() {
  // التحقق من الضغط (مع المقاومة Pull-down، ننتظر قراءة HIGH)
  if (digitalRead(BUTTON_PIN) == HIGH) {
    delay(200); // Debouncing لتفادي القراءة المزدوجة

    // تبديل الحالة
    if (isWindowOpen) {
      setWindowState(CLOSE_ANGLE);
      Serial.println("Window Closed via Button.");
    } else {
      setWindowState(OPEN_ANGLE);
      Serial.println("Window Opened via Button.");
    }

    // الانتظار حتى يتم رفع اليد عن الزر
    while (digitalRead(BUTTON_PIN) == HIGH) {
      // لا تفعل شيئًا
    }
  }
}

// 4. دالة التحقق من المسافة (ميزة الأمان)
void checkSafetyDistance() {
  // التحقق يتم فقط عندما تكون النافذة مغلقة
  if (!isWindowOpen) {
    // قياس المسافة
    long duration = sonar.ping_cm(); // قياس المسافة بالسنتيمتر
    
    // إذا كانت المسافة بين 1 سم وحد الأمان (20 سم)
    if (duration > 1 && duration <= SAFETY_THRESHOLD) { 
      // تم اكتشاف جسم! إيقاف السيرفو وإصدار تنبيه
      windowServo.write(windowServo.read()); // إبقاء السيرفو في موضعه الحالي
      
      digitalWrite(BUZZER_PIN, HIGH);
      
      // طباعة التنبيه
      Serial.print("!! DANGER !! Object detected at ");
      Serial.print(duration);
      Serial.println(" cm. Window close halted.");
      
      // لا نستخدم delay هنا حتى لا نعطل باقي الكود، 
      // سنترك الجرس يعمل حتى يتم إزالة الجسم أو إعادة فتح النافذة.
    } else {
      // إطفاء الجرس إذا كان يعمل وتم رفع الخطر
      digitalWrite(BUZZER_PIN, LOW);
    }
  }
}#include <Servo.h>         // للتحكم في محرك السيرفو
#include <SoftwareSerial.h> // للتحكم بالبلوتوث عبر أطراف (2 و 3)
#include <NewPing.h>       // للتحكم بمستشعر المسافة (HC-SR04)

// ******* 1. تعريف أطراف الدوائر *******

// البلوتوث HC-05 (توصيل آمن باستخدام مقسم الجهد على Pin 3)
const int BT_RX_PIN = 2; // مستقبل الأردوينو -> يتصل بـ TXD في البلوتوث (مباشر)
const int BT_TX_PIN = 3; // مرسل الأردوينو -> يتصل بـ RXD في البلوتوث (عبر مقسم الجهد)

// مستشعر المسافة HC-SR04
const int TRIGGER_PIN = 9; // إرسال الموجات
const int ECHO_PIN = 8;    // استقبال الموجات

// محرك السيرفو
const int SERVO_PIN = 6;

// الزر الضاغط والـ LEDs (مؤشرات الحالة)
const int BUTTON_PIN = 7; // الزر اليدوي (متصل بمقاومة Pull-down 10kOhm)
const int LED_OPEN_PIN = 4; // LED أخضر (النافذة مفتوحة)
const int LED_CLOSE_PIN = 5; // LED أحمر (النافذة مغلقة)
const int BUZZER_PIN = 10; // الجرس الصوتي للتنبيه الأمني

// ******* 2. تعريف الثوابت وقيم الزوايا *******
const int MAX_DISTANCE = 200; // أقصى مسافة يقيسها المستشعر (2 متر)
const int OPEN_ANGLE = 170; // زاوية الفتح
const int CLOSE_ANGLE = 10; // زاوية الإغلاق
const int SAFETY_THRESHOLD = 20; // حد الأمان (20 سم) لوقف الإغلاق

// ******* 3. إنشاء الكائنات (Instances) *******
Servo windowServo;
SoftwareSerial bluetooth(BT_RX_PIN, BT_TX_PIN); // RX, TX
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// ******* 4. المتغيرات الحالة *******
bool isWindowOpen = false; // لمتابعة حالة النافذة (مغلقة في البداية)

// **********************************************
// ************** الدالة setup ******************
// **********************************************
void setup() {
  // تهيئة الاتصالات التسلسلية
  Serial.begin(9600);    // للمراقبة على الكمبيوتر
  bluetooth.begin(9600); // للاتصال بوحدة HC-05

  // تهيئة أطراف المخرجات (Outputs)
  pinMode(LED_OPEN_PIN, OUTPUT);
  pinMode(LED_CLOSE_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // تهيئة أطراف المدخلات (Inputs)
  pinMode(BUTTON_PIN, INPUT);

  // توصيل السيرفو بالطرف المحدد
  windowServo.attach(SERVO_PIN);

  // إغلاق النافذة في البداية
  setWindowState(CLOSE_ANGLE);
  Serial.println("Smart Window System Initialized. Window is Closed.");
}

// **********************************************
// ************** الدالة loop *******************
// **********************************************
void loop() {
  handleBluetoothControl(); // 1. معالجة أوامر البلوتوث
  handleManualButton();     // 2. معالجة ضغطة الزر اليدوي
  checkSafetyDistance();    // 3. التحقق من الأمان أثناء الإغلاق
}


// **********************************************
// ************ الدوال المساعدة ******************
// **********************************************

// 1. دالة تغيير حالة النافذة (تتحكم في السيرفو والـ LEDs)
void setWindowState(int angle) {
  // إيقاف الجرس في حال كان يعمل
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

// 2. دالة معالجة أوامر البلوتوث (1 للفتح، 0 للإغلاق)
void handleBluetoothControl() {
  if (bluetooth.available()) {
    char command = bluetooth.read();
    Serial.print("Received Command: ");
    Serial.println(command);

    if (command == '1') {
      setWindowState(OPEN_ANGLE);
      Serial.println("Window Opened via Bluetooth.");
    } else if (command == '0') {
      setWindowState(CLOSE_ANGLE);
      Serial.println("Window Closed via Bluetooth.");
    }
  }
}

// 3. دالة معالجة ضغط الزر اليدوي
void handleManualButton() {
  // التحقق من الضغط (مع المقاومة Pull-down، ننتظر قراءة HIGH)
  if (digitalRead(BUTTON_PIN) == HIGH) {
    delay(200); // Debouncing لتفادي القراءة المزدوجة

    // تبديل الحالة
    if (isWindowOpen) {
      setWindowState(CLOSE_ANGLE);
      Serial.println("Window Closed via Button.");
    } else {
      setWindowState(OPEN_ANGLE);
      Serial.println("Window Opened via Button.");
    }

    // الانتظار حتى يتم رفع اليد عن الزر
    while (digitalRead(BUTTON_PIN) == HIGH) {
      // لا تفعل شيئًا
    }
  }
}

// 4. دالة التحقق من المسافة (ميزة الأمان)
void checkSafetyDistance() {
  // التحقق يتم فقط عندما تكون النافذة مغلقة
  if (!isWindowOpen) {
    // قياس المسافة
    long duration = sonar.ping_cm(); // قياس المسافة بالسنتيمتر
    
    // إذا كانت المسافة بين 1 سم وحد الأمان (20 سم)
    if (duration > 1 && duration <= SAFETY_THRESHOLD) { 
      // تم اكتشاف جسم! إيقاف السيرفو وإصدار تنبيه
      windowServo.write(windowServo.read()); // إبقاء السيرفو في موضعه الحالي
      
      digitalWrite(BUZZER_PIN, HIGH);
      
      // طباعة التنبيه
      Serial.print("!! DANGER !! Object detected at ");
      Serial.print(duration);
      Serial.println(" cm. Window close halted.");
      
      // لا نستخدم delay هنا حتى لا نعطل باقي الكود، 
      // سنترك الجرس يعمل حتى يتم إزالة الجسم أو إعادة فتح النافذة.
    } else {
      // إطفاء الجرس إذا كان يعمل وتم رفع الخطر
      digitalWrite(BUZZER_PIN, LOW);
    }
  }
}
