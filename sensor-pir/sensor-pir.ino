#include <WiFi.h>
#include <PubSubClient.h>

// Konfigurasi WiFi & MQTT
const char* ssid = "Ini Wi-Fi Mpa";
const char* password = "Yours--..-..";
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

// Pin
const int pirPin = 18;
const int buzzer = 4;

// Variabel
bool isGelap = false;
bool cooldown = false;
unsigned long cooldownStart = 0;
bool sistemAktif = true;
bool lastPirState = LOW;

WiFiClient espClient;
PubSubClient client(espClient);

// Fungsi Wifi

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

// Reconnect

void reconnect() {
  while (!client.connected()) {
    Serial.println("Menghubungkan MQTT...");

    if (client.connect("ESP32_Node_Pintu")) {
      Serial.println("MQTT Connected");
      client.subscribe("sistem/cahaya");
      client.subscribe("sistem/control");
      Serial.println("Subscribe sukses");
    } else {
      Serial.print("Gagal MQTT, rc=");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

// Callback MQTT

void callback(char* topic, byte* payload, unsigned int length) {
  String pesan = "";
  for (int i = 0; i < length; i++) {
    pesan += (char)payload[i];
  }

  pesan.toLowerCase();
  
  if (String(topic) == "sistem/cahaya") {

    if (pesan == "gelap" || pesan == "sangat gelap") {
      isGelap = true;
    } else {
      isGelap = false;
    }
  }

  if (String(topic) == "sistem/control") {
    if (pesan == "on") {
      sistemAktif = true;
      kirimLog("SYSTEM ARMED");
    } else if (pesan == "off") {
      sistemAktif = false;
      kirimLog("SYSTEM DISARMED");
    }
  }
}

// Fungsi kirimm log

void kirimLog(String pesan) {
  Serial.println(pesan);
  client.publish("sistem/log", pesan.c_str());
}

// fungsi buzzer

void alarmCyberpunk() {
  for(int i=0;i<15;i++) {
    ledcWriteTone(buzzer, 1200);
    delay(120);
    ledcWriteTone(buzzer, 1800);
    delay(120);
  }

  ledcWriteTone(buzzer, 0);
}

// Setup utama

void setup() {
  Serial.begin(115200);

  pinMode(pirPin, INPUT);

  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  ledcAttach(buzzer, 2000, 8);

  Serial.println("PIR WARMING UP...");
  client.publish("sistem/log", "PIR WARMING UP");

  delay(30000);

  Serial.println("PIR READY");
  client.publish("sistem/log", "PIR READY");
}

// Fungsi Main

void loop() {
  reconnect();

  client.loop();

  static unsigned long lastHeartbeat = 0;
  
  if (millis() - lastHeartbeat > 5000) {
    client.publish("sistem/heartbeat", "ONLINE");
    lastHeartbeat = millis();
  }

  if (cooldown) {
    if (millis() - cooldownStart > 10000) {
      cooldown = false;
      kirimLog("SYSTEM READY");
      client.publish("sistem/status", "READY");
    } else {
      return;
    }
  }

  if (!sistemAktif) {
    return;
  }
  
  // Logika deteksi orang
  bool currentPirState = digitalRead(pirPin);

  if (currentPirState == HIGH && lastPirState == LOW) {
    kirimLog("GERAKAN TERDETEKSI");

    if (isGelap) {
      kirimLog("PENYUSUP TERDETEKSI");
      alarmCyberpunk();
      client.publish(
        "sistem/notifikasi",
        "PENYUSUP!"
      );

      cooldown = true;
      cooldownStart = millis();
      client.publish(
        "sistem/status",
        "COOLDOWN"
      );
    } else {
      kirimLog("STATUS AMAN");
      return;
    }
  }

  lastPirState = currentPirState;
}