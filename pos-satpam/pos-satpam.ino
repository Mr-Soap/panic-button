#include <WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

// --- Konfigurasi WiFi & MQTT ---
const char* ssid = "Ini Wi-Fi Mpa";
const char* password = "Yours--..-..";
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// pin

LiquidCrystal_I2C lcd(0x27,16,2);
Servo gateServo;
const int buzzerPin = 23;

// variables

bool gateClosed = false;
unsigned long gateCloseTime = 0;

// Wi-Fi

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  Serial.println(WiFi.localIP());
}

// Reconnect

void reconnect() {
  while (!client.connected()) {
    
    if (client.connect("ESP32_Pos_Satpam")) {
      client.subscribe("sistem/notifikasi");

      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("MQTT CONNECT");
      
      lcd.setCursor(0,1);
      lcd.print("ONLINE");

      lcd.setCursor(0,0);
      lcd.print("POS SATPAM");

      lcd.setCursor(0,1);
      lcd.print("STATUS NORMAL");
    } else {
      Serial.print("MQTT GAGAL. RC=");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

//setup

void setup() {
  Serial.begin(115200);
  setup_wifi();

  //setup lcd
  Wire.begin(21, 22);

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0,0);
  lcd.print("POS SATPAM");
  lcd.setCursor(0,1);
  lcd.print("SIAP");

  //setup mqtt
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();

  //setup servo
  gateServo.attach(18);
  gateServo.write(90);

  //setup buzzer
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
}

//callback

void callback(char* topic, byte* payload, unsigned int length) {
  String pesan="";

  for(int i=0;i<length;i++)
    pesan+=(char)payload[i];

  Serial.println(pesan);

  if(String(topic)=="sistem/notifikasi") {
    int separator = pesan.indexOf('|');
    String baris1 = pesan;
    String baris2 = "";

    if (separator != -1) {
      baris1 = pesan.substring(0, separator);
      baris2 = pesan.substring(separator + 1);
    }

    lcd.clear();

    lcd.setCursor(0,0);
    lcd.print(baris1.substring(0,16));

    lcd.setCursor(0,1);
    lcd.print(baris2.substring(0,16));

    digitalWrite(buzzerPin, HIGH);

    gateServo.write(0);

    gateClosed = true;
    gateCloseTime = millis();
  }
}

// loop

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  delay(5000);

  static unsigned long lastHeartbeat = 0;

  if (millis() - lastHeartbeat > 5000) {
    client.publish("pos_satpam/status", "ONLINE");
    lastHeartbeat = millis();
  }


  if (gateClosed && millis() - gateCloseTime > 10000) {
    gateServo.write(90);
    digitalWrite(buzzerPin, LOW);
    gateClosed = false;

    lcd.clear();

    lcd.setCursor(0,0);
    lcd.print("POS SATPAM");

    lcd.setCursor(0,1);
    lcd.print("STATUS NORMAL");
  }
}