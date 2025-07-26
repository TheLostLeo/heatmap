#include <WiFi.h>
#include "esp_wifi.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

typedef struct {
  unsigned frame_ctrl:16;
  unsigned duration_id:16;
  uint8_t addr2[6];
  uint8_t addr3[6];  
  unsigned seq_ctrl:16;
} wifi_ieee80211_mac_hdr_t;

void sendToBackend(String mac, int rssi, int device_id) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://your-backend-url.com/endpoint");
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<200> doc;
    doc["device_id"] = device_id;
    doc["mac"] = mac;
    doc["rssi"] = rssi;

    String requestBody;
    serializeJson(doc, requestBody);

    int httpResponseCode = http.POST(requestBody);
    Serial.printf("Sent to backend. Response: %d\n", httpResponseCode);

    http.end();
  } else {
    Serial.println("Not connected to Wi-Fi");
  }
}


void snifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
  wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
  const wifi_ieee80211_mac_hdr_t *hdr = (wifi_ieee80211_mac_hdr_t *) pkt->payload;

  int rssi = pkt->rx_ctrl.rssi;

  char macStr[18];
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
    hdr->addr2[0], hdr->addr2[1], hdr->addr2[2],
    hdr->addr2[3], hdr->addr2[4], hdr->addr2[5]);

  Serial.printf("Detected MAC: %s | RSSI: %d dBm\n", macStr, rssi);

  sendToBackend(String(macStr), rssi, 2); // Set your own device ID here

}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin("password", "12345678"); // Replace with your actual credentials
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected to Wi-Fi");

  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(snifferCallback);

  Serial.println("Promiscuous mode active. Listening for MACs...");
}

void loop() {
  delay(1);
}
