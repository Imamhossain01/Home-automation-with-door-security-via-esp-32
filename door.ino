#define BLYNK_TEMPLATE_ID "TMPL6EF4P_nHq"
#define BLYNK_TEMPLATE_NAME "HIRT Project"
#define BLYNK_AUTH_TOKEN "ILhOvhDML2k5QFJHetzk7Y7tEQ9svX5R"

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

#define DOOR_SENSOR  4
#define buzzer1      2    // Fixed buzzer (ON/OFF)
#define buzzer2      12   // Blinking buzzer (D12)

#define RELAY1       18
#define RELAY2       19
#define RELAY3       22
#define RELAY4       23

#define BYPASS_LED   5   // D5 physical LED

BlynkTimer timer;
WidgetLED ledBypass(V1);
WidgetLED ledDoor(V2);

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "HIRT";
char pass[] = "imam121#";

bool bypassSensor = false;
int flag = 0;
bool buzzer2State = false;
int blinkTimerId = -1;

void blinkBuzzer2() {
  buzzer2State = !buzzer2State;
  digitalWrite(buzzer2, buzzer2State ? HIGH : LOW);
}

void stopBuzzer2Blink() {
  if (blinkTimerId != -1) {
    timer.deleteTimer(blinkTimerId);
    blinkTimerId = -1;
  }
  digitalWrite(buzzer2, LOW);  // Ensure buzzer is off
}

void notifyOnButtonPress() {
  if (bypassSensor) {
    Serial.println("🚫 Sensor check skipped (bypass ON)");
    digitalWrite(buzzer1, LOW);
    stopBuzzer2Blink();
    digitalWrite(BYPASS_LED, HIGH);
    ledDoor.off();
    return;
  }

  int isButtonPressed = digitalRead(DOOR_SENSOR);
  if (isButtonPressed == 1 && flag == 0) {
    Serial.println("Security Door Is Open !");
    Blynk.logEvent("security_alert", "Security Door Is Open !");
    digitalWrite(buzzer1, HIGH);
    if (blinkTimerId == -1) {
      blinkTimerId = timer.setInterval(200L, blinkBuzzer2);  // Start blinking D12
    }
    ledDoor.on();
    flag = 1;
  } else if (isButtonPressed == 0) {
    flag = 0;
    Serial.println("Door Closed");
    digitalWrite(buzzer1, LOW);
    stopBuzzer2Blink();
    ledDoor.off();
  }
}

BLYNK_WRITE(V0) {
  int value = param.asInt(); // 1 = ON, 0 = OFF
  bypassSensor = (value == 1);

  if (bypassSensor) {
    Serial.println("🔕 Door sensor bypassed");
    digitalWrite(buzzer1, LOW);
    stopBuzzer2Blink();
    ledBypass.on();
    digitalWrite(BYPASS_LED, HIGH);
  } else {
    Serial.println("🔔 Door sensor active");
    ledBypass.off();
    digitalWrite(BYPASS_LED, LOW);
    flag = 0;                 // ✅ Reset alert state
    notifyOnButtonPress();    // ✅ Re-check door state immediately
  }
}

// Relay control from Blynk
BLYNK_WRITE(V3) { digitalWrite(RELAY1, param.asInt() ? HIGH : LOW); }
BLYNK_WRITE(V4) { digitalWrite(RELAY2, param.asInt() ? HIGH : LOW); }
BLYNK_WRITE(V5) { digitalWrite(RELAY3, param.asInt() ? HIGH : LOW); }
BLYNK_WRITE(V6) { digitalWrite(RELAY4, param.asInt() ? HIGH : LOW); }

void setup() {
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);

  pinMode(DOOR_SENSOR, INPUT_PULLUP);
  pinMode(buzzer1, OUTPUT);
  pinMode(buzzer2, OUTPUT);
  pinMode(BYPASS_LED, OUTPUT);
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  pinMode(RELAY4, OUTPUT);

  digitalWrite(buzzer1, LOW);
  digitalWrite(buzzer2, LOW);
  digitalWrite(BYPASS_LED, LOW);
  digitalWrite(RELAY1, LOW);
  digitalWrite(RELAY2, LOW);
  digitalWrite(RELAY3, LOW);
  digitalWrite(RELAY4, LOW);

  timer.setInterval(5000L, notifyOnButtonPress);

  Blynk.syncVirtual(V0);
  Blynk.syncVirtual(V3, V4, V5, V6);
}

void loop() {
  Blynk.run();
  timer.run();
}
