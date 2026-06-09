import processing.serial.*;
import mqtt.*;

MQTTClient client;

// DATA SENSOR

int adc = 0;
int persen = 0;
String statusLuminansi = "UNKNOWN";

boolean systemArmed = true;
boolean motionDetected = false;
boolean intruderAlert = false;
boolean cooldownMode = false;
boolean sensorOnline = true;
boolean ldrOnline = false;
long lastLdrHeartbeat = 0;
long lastHeartbeat = 0;
long lastMotionTime = 0;

// ANIMASI

float smoothBar = 0;
float pulse = 0;

// LOG SYSTEM

ArrayList<String> logs = new ArrayList<String>();
int scrollOffset = 0;

// SETUP

void setup() {
  client = new MQTTClient(this);
  client.connect("tcp://broker.hivemq.com:1883", "ProcessingDashboard"  + millis());
  
  delay(1000);
  client.subscribe("sistem/log");
  client.subscribe("sistem/status");
  client.subscribe("sistem/heartbeat");
  client.subscribe("sistem/heartbeat/ldr");
  client.subscribe("sistem/notifikasi");
  client.subscribe("sistem/ldr");
  
  size(1200, 700);
  surface.setTitle("CYBERPUNK SMART SECURITY");
  
  textFont(createFont("Consolas", 20));
  tambahLog("SYSTEM INITIALIZED");
}

// DRAW

void draw() {
  background(8, 8, 15);
  
  if (millis() - lastHeartbeat > 7000) {
    sensorOnline = false;
  }
  
  if (millis() - lastLdrHeartbeat > 7000) {
    ldrOnline = false;
  }
  
  if (!intruderAlert && motionDetected && millis() - lastMotionTime > 3000) {
    motionDetected = false;
  }
  
  pulse += 0.05;
  smoothBar = lerp(smoothBar, persen, 0.08);
  drawBackgroundGrid();
  drawTitle();
  drawSystemPanel();
  drawLightPanel();
  drawSecurityPanel();
  drawLogPanel();
  drawFooter();
}

// BACKGROUND GRID

void drawBackgroundGrid() {
  stroke(0, 120, 120, 40);
  for (int x = 0; x < width; x += 40) {
    line(x, 0, x, height);
  }
  
  for (int y = 0; y < height; y += 40) {
    line(0, y, width, y);
  }
}

// TITLE

void drawTitle() {
  fill(0, 255, 255);
  textSize(36);
  textAlign(CENTER);
  text("CYBERPUNK SMART SECURITY SYSTEM", width/2, 50);
}

// PANEL SYSTEM

void drawSystemPanel() {
  neonRect(40, 100, 300, 240, color(255, 0, 255));
  fill(255);
  textSize(24);
  textAlign(LEFT);
  text("SYSTEM STATUS", 60, 140);
  fill(0, 255, 100);
  textSize(20);
  text("MQTT : ONLINE", 60, 190);
  
  if (sensorOnline) {
    fill(0,255,100);
    text("MOTION SENSOR : ONLINE", 60, 220);
  } else {
    fill(255,0,0);
    text("MOTION SENSOR : OFFLINE", 60, 220);
  }
  
  if (ldrOnline) {
    fill(0,255,100);
    text("LDR SENSOR : ONLINE", 60, 250);
  } else {
    fill(255,0,0);
    text("LDR SENSOR : OFFLINE", 60, 250);
  }
  
  if (systemArmed) {
    fill(255, 80, 80);
    text("SECURITY : ARMED", 60, 290);
  } else {
    fill(100, 255, 255);
    text("SECURITY : DISARMED", 60, 290);
  }

  // BUTTON

  if (systemArmed) {
    fill(255, 0, 80);
  } else {
    fill(0, 200, 255);
  }

  rect(190, 302, 120, 28, 8);
  fill(255);
  textAlign(CENTER, CENTER);

  if (systemArmed) {
    text("TURN OFF", 250, 315);
  } else {
    text("TURN ON", 250, 315);
  }
}

// PANEL MONITOR LAMPUU

void drawLightPanel() {
  neonRect(380, 100, 380, 240, color(0, 255, 260));
  fill(255);
  textAlign(LEFT);
  textSize(24);
  text("LIGHT MONITOR", 400, 140);
  fill(255);
  textSize(20);
  text("ADC VALUE : " + adc, 400, 190);
  text("LUMINANCE : " + persen + "%", 400, 230);
  fill(0, 255, 255);
  rect(400, 270, map(smoothBar, 0, 100, 0, 300), 25, 10);
  noFill();
  stroke(0, 255, 255);
  rect(400, 270, 300, 25, 10);
  fill(255);
  text(statusLuminansi, 400, 320);
}

// PANEL SECURITY

void drawSecurityPanel() {
  color securityColor;
  
  if (intruderAlert) {
    float glow = map(sin(pulse * 4), -1, 1, 100, 255);
    securityColor = color(glow, 0, 0);
  } else {
    securityColor = color(255, 50, 120);
  }

  neonRect(800, 100, 340, 220, securityColor);
  fill(255);
  textAlign(LEFT);
  textSize(24);
  text("SECURITY STATUS", 820, 140);
  textSize(20);

  if (motionDetected) {
    fill(255,180,0);
    text("PIR : MOTION DETECTED", 820, 200);
  } else {
    fill(0,255,100);
    text("PIR : IDLE", 820, 200);
  }

  if (intruderAlert) {
    fill(255, 0, 0);
    textSize(30);
    text("INTRUDER ALERT", 820, 270);
  } else {
    fill(0, 255, 100);
    text("ALARM : SAFE", 820, 270);
  }
  
  if (cooldownMode) {
      float blink = map(sin(pulse * 5), -1, 1, 80, 255);
      fill(0, blink, 255);
      textSize(18);
      text("SYSTEM COOLDOWN", 820, 305);
   }
}

// LOG PANEL

void drawLogPanel() {
  neonRect(40, 370, 1100, 250, color(180, 0, 255));
  fill(255);
  textAlign(LEFT);
  textSize(24);
  text("SYSTEM LOG", 60, 410);
  textSize(18);
  
  for (int i = 0; i < logs.size(); i++) {
    float y = 450 + i * 22 - scrollOffset;

    if (y > 430 && y < 600) {
      fill(0, 255, 180);
      text("> " + logs.get(i), 70, y);
    }
  }
}

// FOOTER

void drawFooter() {
  fill(100);
  textAlign(CENTER);
  textSize(14);
  text("ESP32 + MQTT + PROCESSING CYBERPUNK DASHBOARD", width/2, height - 20);
}

// LOG FUNCTION

void tambahLog(String pesan) {
  logs.add(0, jamSekarang() + "  " + pesan);
  
  if (logs.size() > 100) {
    logs.remove(logs.size() - 1);
  }
}

// TIME

String jamSekarang() {
  return nf(hour(), 2) + ":" +
    nf(minute(), 2) + ":" +
    nf(second(), 2);
}

// NEON RECT

void neonRect(float x, float y, float w, float h, color c) {
  noFill();
  
  for (int i = 8; i > 0; i--) {
    stroke(red(c), green(c), blue(c), 20);
    strokeWeight(i);
    rect(x, y, w, h, 18);
  }

  stroke(c);
  strokeWeight(2);
  rect(x, y, w, h, 18);
}

// MOUSE CLICK

void mousePressed() {

  // BUTTON AREA

  if (mouseX > 190 && mouseX < 310 && mouseY > 302 && mouseY < 330) {
    systemArmed = !systemArmed;

    if (systemArmed) {
      client.publish("sistem/control", "on");
    } else {
      client.publish("sistem/control", "off");
    }
  }
}

// MOUSE WHEEL

void mouseWheel(processing.event.MouseEvent event) {

  scrollOffset += event.getCount() * 20;

  scrollOffset = constrain(
    scrollOffset,
    0,
    max(0, logs.size() * 22 - 150)
  );
}

// MQTT CAllback

void messageReceived(String topic, byte[] payload) {
  String pesan = new String(payload);
  println("MQTT [" + topic + "] : " + pesan);
  
  if (topic.equals("sistem/ldr")) {
    String[] nilai = split(pesan, ',');
    
    if (nilai.length == 3) {

      adc = int(nilai[0]);
      persen = int(nilai[1]);
      statusLuminansi = nilai[2];

      ldrOnline = true;
      lastLdrHeartbeat = millis();
    }
  }
  
  if (topic.equals("sistem/notifikasi")) {

    String[] bagian = split(pesan, '|');

    if (bagian.length >= 2) {

      String alert = bagian[0];
      String rumah = bagian[1];

      if (alert.equals("PENYUSUP!")) {

        intruderAlert = true;
        motionDetected = true;
        lastMotionTime = millis();
      }
    }
  }
  
  if (topic.equals("sistem/log")) {
    tambahLog(pesan);

    switch(pesan) {
      case "GERAKAN TERDETEKSI":
        motionDetected = true;
        lastMotionTime = millis();
        break;

      case "SYSTEM ARMED":
        systemArmed = true;
        break;

      case "SYSTEM DISARMED":
        systemArmed = false;
        break;
    }
  }
  
  if (topic.equals("sistem/status")) {
    if (pesan.equals("COOLDOWN")) {
      cooldownMode = true;
      tambahLog("SYSTEM COOLDOWN");
    }

    if (pesan.equals("READY")) {
      cooldownMode = false;
      intruderAlert = false;
      motionDetected = false;
      lastMotionTime = 0;
      tambahLog("SYSTEM READY");
    }
  }
  
  if (topic.equals("sistem/heartbeat")) {
    lastHeartbeat = millis();
    sensorOnline = true;
  }
  
  if (topic.equals("sistem/heartbeat/ldr")) {
    ldrOnline = true;
    lastLdrHeartbeat = millis();
  }
}
