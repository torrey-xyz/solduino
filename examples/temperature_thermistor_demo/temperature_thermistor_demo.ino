/**
 * Solduino Thermistor Sensor-to-Chain Demo (Scaffold)
 */

#include <WiFi.h>
#include <solduino.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const String RPC_ENDPOINT = SOLDUINO_DEVNET_RPC;

Solduino solduino;
RpcClient rpcClient(RPC_ENDPOINT);

void setup() {
    Serial.begin(115200);
    solduino.begin();
    WiFi.begin(ssid, password);
    rpcClient.begin();
}

void loop() {
    delay(1000);
}
