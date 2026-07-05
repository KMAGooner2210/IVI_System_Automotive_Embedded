#include <WiFi.h>
#include <WiFiUdp.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>


const char* ssid     = "kmagooner2210";
const char* password = "mtt22102004";

/* ===== GPS NEO-M10 ===== */
const int     GPS_RX_PIN = 16;
const int     GPS_TX_PIN = 17;
const uint32_t GPS_BAUD  = 38400;   



#define DEBUG_RAW_NMEA   0   
#define DEBUG_GPS_STATS  1   


TinyGPSPlus    gps;
HardwareSerial gpsSerial(1);
WiFiUDP        udpSender;


IPAddress      pi_ip(192, 168, 105, 7);
const int      UDP_TX_PORT = 5555;


void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("[BOOT] ESP32-S3 GPS Node starting...");

  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);


  WiFi.begin(ssid, password);
  Serial.print("[WiFi] Connecting");
  int wifi_retry = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    wifi_retry++;
    if (wifi_retry > 60) { 
      Serial.println("\n[WiFi] Timeout! Restarting...");
      esp_restart();  
    }
  }
  Serial.printf("\n[WiFi] Connected. IP: %s\n", WiFi.localIP().toString().c_str());
  Serial.println("[GPS] Waiting for satellite fix... (cold start có thể mất 30s - vài phút)");
}


void loop() {

  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();

#if DEBUG_RAW_NMEA
    Serial.write(c);   
#endif

    gps.encode(c);
  }

 
  static uint32_t lastSendTime = 0;
  if (millis() - lastSendTime >= 1000) {
    lastSendTime = millis();

    double lat   = 0.0;
    double lng   = 0.0;
    double speed = 0.0;
    int    sats  = gps.satellites.isValid() ? gps.satellites.value() : 0;


    if (gps.location.isValid()) {
      lat   = gps.location.lat();
      lng   = gps.location.lng();
      speed = gps.speed.kmph();

 
      Serial.printf("[GPS] Satellites: %d | Lat: %.6f | Lng: %.6f | Speed: %.2f km/h\n",
                    sats, lat, lng, speed);
    } else {
      // Nếu chưa bắt được vệ tinh, vẫn in trạng thái chờ để bạn dễ debug
      Serial.printf("[GPS] Waiting for fix... Satellites detected: %d\n", sats);
    }

#if DEBUG_GPS_STATS
   
    Serial.printf("[DEBUG] charsProcessed=%lu | failedChecksum=%lu | passedChecksum=%lu | sentencesWithFix=%lu\n",
                  gps.charsProcessed(), gps.failedChecksum(),
                  gps.passedChecksum(), gps.sentencesWithFix());

    if (gps.charsProcessed() < 10) {
      Serial.println("[DEBUG] ⚠️ Gần như không có dữ liệu vào -> kiểm tra wiring RX/TX hoặc GPS_BAUD");
    } else if (gps.failedChecksum() > gps.passedChecksum()) {
      Serial.println("[DEBUG] ⚠️ Checksum lỗi nhiều -> có thể sai GPS_BAUD, thử đổi 38400");
    }
#endif


    udpSender.beginPacket(pi_ip, UDP_TX_PORT);
    udpSender.printf("$GPS,%.6f,%.6f,%.2f,%d\n", lat, lng, speed, sats);
    udpSender.endPacket();
  }
}