#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
String data;
int cpu_temp = 0;
int gpu_temp = 0;
int prev_cpu_temp = 0;
int prev_gpu_temp = 0;

const int Pump = 7;
const int relayPin = 8;

int peltierLevel = 0;

unsigned long previousMillis = 0;
unsigned long relayOnTime = 0;
unsigned long relayOffTime = 0;
bool relayState = LOW;

void setup() {
  lcd.init(); 
  lcd.backlight();
  lcd.print("Loading...");
  Serial.begin(9600);
  pinMode(Pump, OUTPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(Pump, HIGH);
  digitalWrite(relayPin, LOW);
}

void loop() {
  while (Serial.available()) {
    data = Serial.readStringUntil('\n');
    int a1 = data.indexOf("a");
    int c1 = data.indexOf("c");
    int new_cpu_temp = data.substring(a1 + 1, c1).toInt();
    int new_gpu_temp = data.substring(c1 + 1).toInt();

    if (new_cpu_temp != 0) {
      cpu_temp = new_cpu_temp;
    }
    if (new_gpu_temp != 0) {
      gpu_temp = new_gpu_temp;
    }
  }

  readPeltierLevel();
  controlRelay();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CPU  GPU  POWER");
  lcd.setCursor(0, 1);
  lcd.print(formatTemp(cpu_temp) + "C " + formatTemp(gpu_temp) + "C");

  displayPeltierBar(peltierLevel);

  delay(1000); // 1초마다 업데이트
}

void readPeltierLevel() {
  int average_temp = (cpu_temp + gpu_temp) / 2;
  peltierLevel = map(average_temp, 40, 80, 0, 5); // 평균 온도를 0에서 5단계로 매핑
}

String formatTemp(int temp) {
  if (temp < 10) {
    return "0" + String(temp) + (char)223;
  } else {
    return String(temp) + (char)223;
  }
}

void displayPeltierBar(int level) {
  if (cpu_temp == 0 && gpu_temp == 0) {
    lcd.setCursor(10, 1);
    lcd.print(" ");
  } else {
    int barLength = map(level, 0, 5, 1, 6);

    lcd.setCursor(10, 1);
    for (int i = 0; i < barLength; i++) {
      lcd.print((char)255);
    }
    for (int i = barLength; i <= 6; i++) {
      lcd.print(" ");
    }
  }
}

void controlRelay() {
  unsigned long currentMillis = millis();
  switch (peltierLevel) {
    case 0:
      digitalWrite(relayPin, LOW); // 릴레이 끄기
      break;
    case 1:
      relayPulse(2000, 3000, currentMillis); // 2초 켜고 3초 끔
      break;
    case 2:
      relayPulse(4000, 2000, currentMillis); // 4초 켜고 2초 끔
      break;
    case 3:
      relayPulse(6000, 2000, currentMillis); // 6초 켜고 2초 끔
      break;
    case 4:
      relayPulse(8000, 1000, currentMillis); // 8초 켜고 1초 끔
      break;
    case 5:
      digitalWrite(relayPin, HIGH); // 릴레이 켜기
      break;
  }
}

void relayPulse(int onTime, int offTime, unsigned long currentMillis) {
  if (relayState == LOW && (currentMillis - previousMillis >= relayOffTime)) {
    relayState = HIGH;
    previousMillis = currentMillis;
    digitalWrite(relayPin, HIGH); // 펠티어 켜기
    relayOnTime = onTime;
  } 
  else if (relayState == HIGH && (currentMillis - previousMillis >= relayOnTime)) {
    relayState = LOW;
    previousMillis = currentMillis;
    digitalWrite(relayPin, LOW); // 펠티어 끄기
    relayOffTime = offTime;
  }
}
