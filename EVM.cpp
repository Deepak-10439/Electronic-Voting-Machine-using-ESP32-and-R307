#include <WiFi.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>
#include <ArduinoJson.h>

const char* ssid = "Deepak";
const char* password = "12345678";

#define buzzerPin 25

int btn1 = 13; 
int btn2 = 33;
int btn3 = 26;

LiquidCrystal_I2C lcd(0x27, 16, 2);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial2);

const char* server = "maker.ifttt.com";

String Event_Name = "voting";
String Key = "pWLojlzNujCebRIpuaUr5PNSLJZe7WYY-sgOpVcUPo";
String resource = "/trigger/" + Event_Name + "/json/with/key/" + Key;

String candidates[] = {"USDI", "USAR", "USAP"};

const unsigned long VOTE_TIMEOUT = 30000; // Timeout for voting process (in milliseconds)

void makeIFTTTRequest(const char* Event_Name, const char* Key, int ID, const String& candidateName) {
  HTTPClient http;
  String url = "https://maker.ifttt.com/trigger/voting/json/with/key/pWLojlzNujCebRIpuaUr5PNSLJZe7WYY-sgOpVcUPo";
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<200> jsonBuffer;
  JsonObject root = jsonBuffer.to<JsonObject>();

  root["value1"] = ID;
  root["value2"] = candidateName;

  String json;
  serializeJson(root, json);

  int httpResponseCode = http.POST(json);

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
  delay(1000);
}

void lcdPrint(uint8_t row, uint8_t position, String message) {
  lcd.setCursor(position, row);
  lcd.print(message);
}

void lcdClear() {
  lcd.clear();
}

void lcdSetup() {
  lcd.init();
  lcd.clear();
  lcd.backlight();
}

void buzzer(String type) {
  if (type == "error") {
    digitalWrite(buzzerPin, HIGH);
    delay(300);
    digitalWrite(buzzerPin, LOW);
    delay(200);
    digitalWrite(buzzerPin, HIGH);
    delay(300);
    digitalWrite(buzzerPin, LOW);
  } else if (type == "success") {
    digitalWrite(buzzerPin, HIGH);
    delay(500);
    digitalWrite(buzzerPin, LOW);
  } else if (type == "already_voted") {
    digitalWrite(buzzerPin, HIGH);
    delay(300);
    digitalWrite(buzzerPin, LOW);
    delay(200);
    digitalWrite(buzzerPin, HIGH);
    delay(300);
    digitalWrite(buzzerPin, LOW);
    delay(200);
    digitalWrite(buzzerPin, HIGH);
    delay(300);
    digitalWrite(buzzerPin, LOW);
  }
}

void ConnectToWifi() {
  lcdSetup();
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  lcdPrint(0, 0, "Connecting to");
  lcdPrint(1, 0, ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("Connected to WiFi network with IP Address: ");
  lcdClear();
  lcdPrint(0, 0, "IP: ");
  lcdPrint(0, 1, WiFi.localIP().toString());
  delay(3000);
  lcdClear();
  Serial.println(WiFi.localIP());
}

boolean getButtonInput2(int buttonPin) {
  boolean state = digitalRead(buttonPin);
  if (state == HIGH) {
    delay(50);
    return true;
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(57600);
  while (!Serial);
  delay(100);
  
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome to ");
  lcd.setCursor(0, 1);
  lcd.print("Smart EVM");
  pinMode(btn1, INPUT_PULLUP);
  pinMode(btn2, INPUT_PULLUP);
  pinMode(btn3, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);

    delay(3000);
  ConnectToWifi();

  if (finger.verifyPassword()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Found fingerprint");
    lcd.setCursor(0, 1);
    lcd.print("sensor!");
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Did not find");
    lcd.setCursor(0, 1);
    lcd.print("fingerprint sensor :(");
    while (1) {
      delay(1);
    }
  }

  finger.getTemplateCount();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sensor contains");
  lcd.setCursor(0, 1);
  lcd.print(finger.templateCount);
  lcd.print(" templates");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for");
  lcd.setCursor(0, 1);
  lcd.print("valid finger...");
}

void loop() {
  uint8_t p = finger.getImage();
  if (p == FINGERPRINT_OK) {
    p = finger.image2Tz();
    if (p == FINGERPRINT_OK) {
      p = finger.fingerFastSearch();
      if (p == FINGERPRINT_OK) {
        Serial.print("Found ID #"); Serial.print(finger.fingerID);
        Serial.print(" with confidence of "); Serial.println(finger.confidence);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("!!-");
        lcd.print(finger.fingerID);
        lcd.setCursor(0, 1);
        lcd.print("Vote for:");

        lcd.setCursor(0, 0);
        lcd.print("1     2     3");
        lcd.setCursor(0,1);
        lcd.print(candidates[0]);
        lcd.print(" ");
        lcd.print(candidates[1]);
        lcd.print(" ");
        lcd.print(candidates[2]);

        boolean voteCast = false;
        unsigned long voteStartTime = millis();

        while (true) {
          int btnState1 = digitalRead(btn1);
          int btnState2 = digitalRead(btn2);
          int btnState3 = digitalRead(btn3);
          
          if (btnState1 == HIGH) {
            makeIFTTTRequest("event_name", "IFTTT_key", p, "USDI");
            voteCast = true;
            buzzer("success");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Vote casted to!");
            lcd.setCursor(0,1);
            lcd.print("USDI");
            delay(7000);
            lcd.setCursor(0, 0);
            lcd.print("Voting process");
            lcd.setCursor(0,1);
            lcd.print("completed");
            break;
          } else if (btnState2 == HIGH) {
            makeIFTTTRequest("event_name", "IFTTT_key", p, "USAR");
            voteCast = true;
            buzzer("success");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Vote casted to!");
            lcd.setCursor(0,1);
            lcd.print("USAR");
            delay(5000);
            lcd.setCursor(0, 0);
            lcd.print("Voting process");
            lcd.setCursor(0,1);
            lcd.print("completed");
            break;
          } else if (btnState3 == HIGH) {
            makeIFTTTRequest("event_name", "IFTTT_key", p, "USAP");
            voteCast = true;
            buzzer("success");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Vote casted to!");
            lcd.setCursor(0,1);
            lcd.print("USAP");
            delay(5000);
            lcd.setCursor(0, 0);
            lcd.print("Voting process");
            lcd.setCursor(0,1);
            lcd.print("completed");
            break;
          } else {
            delay(100); 
            }
        }

        if (!voteCast) {
          lcd.clear();
          lcd.print("Vote timeout!");
          delay(2000);
        }
      } else {
        Serial.println("Did not find a match");
      }
    } else {
      Serial.println("Failed to convert image");
    }
  } else {
    Serial.println("Failed to get image");
  }

  delay(500);
}
