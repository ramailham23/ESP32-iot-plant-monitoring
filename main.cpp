#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ================= WIFI & MQTT =================
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;
const char* mqtt_topic = "iot_monitoring/IlhamRama1211/sensor";

WiFiClient espClient;
PubSubClient client(espClient);

// ================= PENGATURAN PIN =================
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define TRIG_PIN 5
#define ECHO_PIN 18
#define MQ2_PIN 34

LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long previousMillis = 0;
const long interval = 3000;
int screenState = 0;

void setup_wifi() {
  delay(10);
  Serial.print("Menghubungkan ke WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" terhubung!");
}

void reconnect_mqtt() {
  while (!client.connected()) {
    Serial.print("Menghubungkan ke MQTT broker...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("terhubung");
    } else {
      Serial.print("gagal, rc=");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void publishData(float t, float h, float distance, int mq2Value) {
  StaticJsonDocument<200> doc;
  doc["device_id"] = "esp32_tanaman_01";
  doc["suhu"] = t;
  doc["kelembapan"] = h;
  doc["jarak_air"] = distance;
  doc["gas_value"] = mq2Value;

  char buffer[200];
  serializeJson(doc, buffer);
  client.publish(mqtt_topic, buffer);
  Serial.print("Terkirim: ");
  Serial.println(buffer);
}

void setup() {
  Serial.begin(115200);

  dht.begin();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Sistem Monitor");
  lcd.setCursor(0, 1);
  lcd.print("Tanaman ESP32");
  delay(2000);
  lcd.clear();

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop();

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  float distance = duration * 0.034 / 2;

  int mq2Value = analogRead(MQ2_PIN);

  if (isnan(h) || isnan(t)) {
    Serial.println("Gagal membaca dari sensor DHT!");
    return;
  }

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    lcd.clear();

    publishData(t, h, distance, mq2Value);

    if (mq2Value > 2000) {
      lcd.setCursor(0, 0);
      lcd.print("!! BAHAYA !!");
      lcd.setCursor(0, 1);
      lcd.print("ASAP TERDETEKSI!");
    } 
    else {
      if (screenState == 0) {
        lcd.setCursor(0, 0);
        if (t > 35.0) lcd.print("Suhu: PANAS!");
        else if (t < 20.0) lcd.print("Suhu: DINGIN!");
        else { lcd.print("Suhu: "); lcd.print(t, 1); lcd.print(" C"); }

        lcd.setCursor(0, 1);
        if (h < 40.0) lcd.print("Lembap: KERING!");
        else { lcd.print("Lembap: "); lcd.print(h, 1); lcd.print(" %"); }

        screenState = 1;
      } 
      else if (screenState == 1) {
        lcd.setCursor(0, 0);
        if (distance > 20.0) lcd.print("Air: HABIS!");
        else lcd.print("Air: AMAN (Cukup)");

        lcd.setCursor(0, 1);
        lcd.print("Asap: AMAN");

        screenState = 0;
      }
    }
  }
}
