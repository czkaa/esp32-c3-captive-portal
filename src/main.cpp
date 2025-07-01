#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <esp_wifi.h>
#include <esp_pm.h>

const char *ssid = "lovelynonbinary";
const IPAddress localIP(192, 168, 4, 1);
const IPAddress gatewayIP(192, 168, 4, 1);
const IPAddress subnetMask(255, 255, 255, 0);
const int maxClients = 4;

DNSServer dnsServer;
WebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>My Portal</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial, sans-serif; background: lavender; color: black; text-align: center; padding: 50px; }
        .container { background: rgba(255,255,255,0.7); padding: 30px; border-radius: 15px; max-width: 400px; margin: auto; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Welcome to the Network Charm!</h1>
    </div>
</body>
</html>
)rawliteral";

void handleRoot()
{
  server.send(200, "text/html", index_html);
  Serial.println("📄 Served portal page");
}

void handleNotFound()
{
  // Redirect any unknown path to root
  server.sendHeader("Location", "http://192.168.4.1", true);
  server.send(302, "text/plain", "");
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  esp_pm_config_esp32c3_t pm_config = {
      .max_freq_mhz = 80,
      .min_freq_mhz = 10,
      .light_sleep_enable = true};
  esp_pm_configure(&pm_config);

  // Setup WiFi AP with reduced power
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(localIP, gatewayIP, subnetMask);

  // Reduced WiFi transmission power (0-20, where 20 is max)
  WiFi.setTxPower(WIFI_POWER_11dBm);

  if (WiFi.softAP(ssid, nullptr, 6, false, maxClients))
  {
    // Successfully started AP
    Serial.println("WiFi AP started: " + String(ssid));
  }
  else
  {
    Serial.println("WiFi AP failed to start");
  }

  // Configure WiFi power saving
  esp_wifi_set_ps(WIFI_PS_MIN_MODEM); // Enable modem sleep

  // Start DNS server
  dnsServer.start(53, "*", localIP);

  // Setup web server routes
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
}

void loop()
{
  // Process requests with minimal delay
  dnsServer.processNextRequest();
  server.handleClient();

  // Longer delay to allow CPU to enter light sleep more often
  delay(50);
}