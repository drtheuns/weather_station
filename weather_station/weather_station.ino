#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "ArduinoJson.h"

const char* ssid = "ssid";
const char* password = "password";

const char* url = "https://baran-it.com/wst/stations/1/getWeather";
// sha1 fingerprint for HTTPS
const char* fingerprint = "39 FC 06 60 1D 63 DB CC C1 12 BF A7 65 D2 59 40 4B 70 06 F1";

void setup()
{
    Serial.begin(115200);
    delay(10);

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("Wifi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    const JsonObject& json = getWeatherInformation();
    const char *temperature = json["temperature"];
    Serial.println(temperature);
}

String getJson()
{
    Serial.print("Reading from ");
    Serial.println(url);

    HTTPClient client;
    client.begin(url, fingerprint);
    int httpCode = client.GET();

    if (httpCode > 0) {
        String payload = client.getString();
        Serial.println(payload);
        return payload;
    }
    client.end();
    return "";
}

JsonObject& getWeatherInformation()
{
    StaticJsonBuffer<200> jsonBuffer;
    String jsonString = getJson();
    JsonObject& root = jsonBuffer.parseObject(jsonString);
    return root;
}

void loop()
{
    delay(5000);
}
