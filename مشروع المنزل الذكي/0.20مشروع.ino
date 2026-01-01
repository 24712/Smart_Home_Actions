//created by I24712I
//Goin me in Git-Hub
//Engoy
#include <ESP8266WiFi.h>
#include <Servo.h>
#include <DHT.h>

// ==========================================================
// 1. إعداد المكتبات والثوابت
// ==========================================================

// -- بيانات الواي فاي (يجب تعديلها) --
const char* ssid = "Smart_Window_wether";  
const char* password = "Wither=01;"; 

// -- تعريف الأطراف (GPIOs) - *** بناءً على آخر جدول توصيلات مُصحَّح *** --
// الأطراف الرقمية (Digital Pins)

#define SERVO_PIN D8    // GPIO15 (SG90 Signal)
#define DHT_PIN D4      // GPIO2 (DHT11 Data)
#define RAIN_PIN D0     // GPIO16 (مستشعر المطر - الطرف الرقمي DO)
#define TRIG_PIN D3     // GPIO0 (HC-SR04 Trigger)
#define ECHO_PIN D7     // GPIO13 (HC-SR04 Echo)
#define BUZZER_PIN D1   // GPIO5 (البازر)

// مؤشرات LED
#define LED_SUMMER 9    // SD2 - GPIO9 (اصفر)
#define LED_SPRING D5   // GPIO14 (ابيض)
#define LED_WINTER D6   // GPIO12 (ازرق)
#define LED_OPEN 10     // SD3 - GPIO10 (اخضر - للفتح العام)
#define LED_ALARM D2    // GPIO4 (احمر - للانذار فقط)

// الأزرار الضاغطة (مستخدمة كـ INPUT_PULLUP)
#define BTN_SUMMER 11   // CMD - زر الصيف
#define BTN_SPRING 12   // CLK - زر الربيع
#define BTN_WINTER 13   // SD0 - زر الشتاء

// -- زوايا السيرفو والثوابت الأخرى --
int angleSummer = 170; // مفتوح بالكامل
int angleSpring = 90;  // فتح متوسط
int angleWinter = 10;  // مغلق تقريباً
int currentAngle = angleWinter;
int safetyDistance = 30; // مسافة الأمان بالسنتيمتر
int rainThreshold = 0; // القيمة الرقمية لـ DO (0 يعني مطر، 1 يعني جاف)

Servo windowServo;
DHT dht(DHT_PIN, DHT11);

// ==========================================================
// 2. الدوال المساعدة
// ==========================================================

void setupWiFi() {
  Serial.print("\nConnecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void sendNotification(String message) {
  // *** يجب استبدال هذه الدالة بكود يرسل تنبيه فعلي (مثل IFTTT أو Telegram) ***
  Serial.print("ALERT SENT: ");
  Serial.println(message);
}

// دالة لتحديث زاوية السيرفو وحالة الـ LED
void setWindowAngle(int targetAngle, int ledPin) {
  if (currentAngle != targetAngle) {
    windowServo.write(targetAngle);
    currentAngle = targetAngle;
    
    // إطفاء جميع أضواء المواسم قبل تشغيل المطلوب
    digitalWrite(LED_SUMMER, LOW);
    digitalWrite(LED_SPRING, LOW);
    digitalWrite(LED_WINTER, LOW);
    
    // تشغيل ضوء الموسم المطلوب
    if (ledPin != 0) {
        digitalWrite(ledPin, HIGH);
    }

    // تشغيل LED الاخضر اذا كانت النافذة مفتوحة
    if (currentAngle > 15) {
        digitalWrite(LED_OPEN, HIGH);
    } else {
        digitalWrite(LED_OPEN, LOW);
    }
    Serial.print("Window set to angle: ");
    Serial.println(targetAngle);
  }
}

// ==========================================================
// 3. التحكم والأمان
// ==========================================================

void checkSafetyAndRain() {
  // 3.1. فحص الحرامي (مستشعر المسافة)
  long duration, distance;
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = (duration/2) / 29.1;

  if (distance > 0 && distance < safetyDistance) {
    // تفعيل الانذار: احمر فقط والبازر
    digitalWrite(LED_ALARM, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    windowServo.detach(); // ايقاف السيرفو مؤقتاً
    sendNotification("Security ALERT: Intruder detected!");
    
    // إيقاف جميع الأضواء الأخرى باستثناء الأحمر
    digitalWrite(LED_SUMMER, LOW);
    digitalWrite(LED_SPRING, LOW);
    digitalWrite(LED_WINTER, LOW);
    digitalWrite(LED_OPEN, LOW);
    
    return; // نوقف التنفيذ لاحقاً
  } else {
    digitalWrite(LED_ALARM, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    if (!windowServo.attached()) {
        windowServo.attach(SERVO_PIN); // إعادة تفعيل السيرفو
    }
  }

  // 3.2. فحص المطر (يتم فقط إذا كان الإنذار غير مفعّل)
  // قراءة رقمية: 0 يعني رطب/مطر، 1 يعني جاف
  int rainDetected = digitalRead(RAIN_PIN);
  
  // إذا كان هناك مطر (rainDetected == 0) والنافذة مفتوحة (currentAngle > angleWinter)
  if (rainDetected == rainThreshold && currentAngle > angleWinter) {
    // إغلاق إجباري بغض النظر عن الموسم
    setWindowAngle(angleWinter, LED_WINTER);
    sendNotification("Rain Detected! Window closed automatically.");
  }
}

void checkAutoMode() {
  float temp = dht.readTemperature();
  if (isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  // تحديد المواسم حسب درجة الحرارة
  if (temp > 30) {
    // صيف (أقصى فتح)
    setWindowAngle(angleSummer, LED_SUMMER);
  } else if (temp >= 15 && temp <= 25) {
    // ربيع / خريف (فتح متوسط)
    setWindowAngle(angleSpring, LED_SPRING);
  } else if (temp < 15) {
    // شتاء (إغلاق جزئي)
    setWindowAngle(angleWinter, LED_WINTER);
  }
}

void checkButtons() {
  // الأزرار تعمل بنظام Pull-Up (LOW عند الضغط)
  if (digitalRead(BTN_SUMMER) == LOW) {
    setWindowAngle(angleSummer, LED_SUMMER);
    delay(300); 
  }
  if (digitalRead(BTN_SPRING) == LOW) {
    setWindowAngle(angleSpring, LED_SPRING);
    delay(300);
  }
  if (digitalRead(BTN_WINTER) == LOW) {
    setWindowAngle(angleWinter, LED_WINTER);
    delay(300);
  }
}

// ==========================================================
// 4. دالة Setup و Loop
// ==========================================================

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  // تهيئة الأطراف كمخرجات (LEDs والتحذير والسيرفو)
  pinMode(LED_SUMMER, OUTPUT);
  pinMode(LED_SPRING, OUTPUT);
  pinMode(LED_WINTER, OUTPUT);
  pinMode(LED_OPEN, OUTPUT);
  pinMode(LED_ALARM, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  
  // تهيئة الأطراف كمدخلات (الأزرار والمستشعرات)
  pinMode(RAIN_PIN, INPUT);
  pinMode(ECHO_PIN, INPUT);
  
  // تفعيل Pull-Up للأزرار (لتوصيل الزر بـ GND فقط)
  pinMode(BTN_SUMMER, INPUT_PULLUP);
  pinMode(BTN_SPRING, INPUT_PULLUP);
  pinMode(BTN_WINTER, INPUT_PULLUP);

  // ربط السيرفو بالطرف والبدء مغلقاً
  windowServo.attach(SERVO_PIN);
  windowServo.write(angleWinter); 
  
  setupWiFi();
}

void loop() {
  // 1. فحص الأمان والمطر (الأولوية القصوى)
  checkSafetyAndRain(); 
  
  // 2. التحكم اليدوي والآلي يتم فقط إذا لم يكن الإنذار مفعلاً
  if (digitalRead(LED_ALARM) == LOW) {
    checkButtons();
    
    // 3. التحكم الآلي يتم كل 5 دقائق
    static unsigned long lastAutoCheck = 0;
    if (millis() - lastAutoCheck > 300000) { // 300000ms = 5 minutes
      checkAutoMode();
      lastAutoCheck = millis();
    }
  }
  
  delay(50); 
}
