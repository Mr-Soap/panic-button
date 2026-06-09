#include <WiFi.h>
#include <PubSubClient.h>

// --- Konfigurasi WiFi & MQTT ---
const char* ssid = "Ini Wi-Fi Mpa";
const char* password = "Yours--..-..";
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// Pin
const int pinLDR = 34; // Pin Analog AO
String lastStatus = "";

// Wi-Fi

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
    if (client.connect("ESP32_Node_Lampu")) {
      // Terhubung
    } else {
      delay(5000);
    }
  }
}

// Log

void kirimLog(String pesan) {
  Serial.println(pesan);
  client.publish("sistem/log", pesan.c_str());
}

// Setup Utama

void setup() {
  Serial.begin(115200);
  pinMode(pinLDR, INPUT);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

// Fungsi Main

void loop() {
  static unsigned long lastHeartbeat = 0;

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Membaca intensitas cahaya
  int nilaiCahaya = analogRead(pinLDR);

  // Konversi ke persentase cahaya
  int persenCahaya = map(nilaiCahaya, 4095, 0, 0, 100);

  // Menentukan kondisi luminansi
  String statusLuminansi;

  if (persenCahaya >= 80) {
    statusLuminansi = "Sangat Terang";
  }
  else if (persenCahaya >= 60) {
    statusLuminansi = "Terang";
  }
  else if (persenCahaya >= 40) {
    statusLuminansi = "Redup";
  }
  else if (persenCahaya >= 20) {
    statusLuminansi = "Gelap";
  }
  else {
    statusLuminansi = "Sangat Gelap";
  }
  
  // Publish ke MQTT Broker agar dibaca oleh Node Pintu
  client.publish("sistem/cahaya", statusLuminansi.c_str());

  if (statusLuminansi != lastStatus) {
    lastStatus = statusLuminansi;
  }

  String dataLDR =
    String(nilaiCahaya) + "," +
    String(persenCahaya) + "," +
    statusLuminansi;
  
  client.publish("sistem/ldr", dataLDR.c_str());
  
  // Data ke processing
  Serial.print(nilaiCahaya);\
  Serial.print(",");
  Serial.print(persenCahaya);
  Serial.print(",");
  Serial.println(statusLuminansi);

  if (millis() - lastHeartbeat > 5000) {

    client.publish("sistem/heartbeat/ldr", "ONLINE");

    lastHeartbeat = millis();
  }
  
  delay(2000); // Kirim update setiap 2 detik
}