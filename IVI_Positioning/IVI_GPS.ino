#include <WiFi.h>
#include <WiFiUdp.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

/* Cấu hình kết nối WiFi */
const char* wifi_ssid     = "kmagooner2210";
const char* wifi_password = "mtt22102004";

/* Cấu hình ngoại vi GPS NEO-M10 */
const int      GPS_RX_PIN = 16;
const int      GPS_TX_PIN = 17;
const uint32_t GPS_BAUD   = 38400;   

#define DEBUG_RAW_NMEA   0   
#define DEBUG_GPS_STATS  1   

/* Khởi tạo các đối tượng ngoại vi và truyền thông */
TinyGPSPlus    gps;
HardwareSerial gps_serial(1);
WiFiUDP        udp_sender;

/* Cấu hình địa chỉ đích và cổng truyền UDP */
IPAddress      pi_ip(192, 168, 105, 7);
const int      UDP_TX_PORT = 5555;

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("[BOOT] ESP32-S3 GPS Node starting...");

  /* Khởi tạo cổng Serial cứng cho GPS */
  gps_serial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

  /* Khởi tạo kết nối mạng WiFi */
  WiFi.begin(wifi_ssid, wifi_password);
  Serial.print("[WiFi] Connecting");
  int wifi_retry_count = 0;
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    wifi_retry_count++;
    if (wifi_retry_count > 60) { 
      Serial.println("\n[WiFi] Timeout! Restarting...");
      esp_restart();  
    }
  }
  
  Serial.printf("\n[WiFi] Connected. IP: %s\n", WiFi.localIP().toString().c_str());
  Serial.println("[GPS] Waiting for satellite fix...");
}

/* ==========================================
   CÁC HÀM XỬ LÝ CHỨC NĂNG ĐƯỢC ĐÓNG GÓI MỚI
   ========================================== */

/**
 * @brief Đọc luồng dữ liệu byte từ GPS Serial và giải mã NMEA
 */
void Read_GPS_Serial(void) {
  while (gps_serial.available() > 0) {
    char c = gps_serial.read();
    #if DEBUG_RAW_NMEA
    Serial.write(c);   
    #endif
    gps.encode(c);
  }
}

/**
 * @brief Đóng gói và gửi chuỗi dữ liệu GPS qua giao thức UDP
 */
void Send_UDP_Packet(double lat, double lng, double speed, int sats) {
  udp_sender.beginPacket(pi_ip, UDP_TX_PORT);
  udp_sender.printf("$GPS,%.6f,%.6f,%.2f,%d\n", lat, lng, speed, sats);
  udp_sender.endPacket();
}

/**
 * @brief Xử lý logic dữ liệu GPS định kỳ và thực hiện truyền thông
 */
void Process_GPS_Transmission(void) {
  double current_lat   = 0.0;
  double current_lng   = 0.0;
  double current_speed = 0.0;
  
  /* Đọc số lượng vệ tinh bắt được */
  int satellites_detected = gps.satellites.isValid() ? gps.satellites.value() : 0;

  /* Kiểm tra tọa độ GPS hợp lệ */
  if (gps.location.isValid()) {
    current_lat   = gps.location.lat();
    current_lng   = gps.location.lng();
    current_speed = gps.speed.kmph();

    Serial.printf("[GPS] Satellites: %d | Lat: %.6f | Lng: %.6f | Speed: %.2f km/h\n",
                  satellites_detected, current_lat, current_lng, current_speed);
  } else {
    Serial.printf("[GPS] Waiting for fix... Satellites detected: %d\n", satellites_detected);
  }

  #if DEBUG_GPS_STATS
  /* Kiểm tra tình trạng dữ liệu GPS */
  if (gps.charsProcessed() < 10) {
    Serial.println("[DEBUG] Warning: Gần như không có dữ liệu vào -> Kiểm tra dây RX/TX");
  } else if (gps.failedChecksum() > gps.passedChecksum()) {
    Serial.println("[DEBUG] Warning: Lỗi checksum -> Kiểm tra lại GPS_BAUD");
  }
  #endif

  /* Thực hiện gửi dữ liệu đi */
  Send_UDP_Packet(current_lat, current_lng, current_speed, satellites_detected);
}

void loop() {
  /* Tác vụ 1: Liên tục đọc luồng dữ liệu Serial */
  Read_GPS_Serial();

  /* Tác vụ 2: Xử lý định kỳ mỗi 1000ms */
  static uint32_t last_send_time_ms = 0;
  if (millis() - last_send_time_ms >= 1000) {
    last_send_time_ms = millis();
    Process_GPS_Transmission();
  }
}
