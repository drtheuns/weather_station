#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "ArduinoJson.h"

// change these to match the actual pins on the arduino
const int motion_pin = 2;
const int button_pin = 3;
const int audio_pin  = 8;

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

// should the event type be sent to this function (e.g. motion vs button event)?
void handleEvent()
{
    // Handle both motion and button pressed events here.
    const JsonObject& weatherJson = getWeatherInformation();
}

void loop()
{
    const int motion_value = digitalRead(motion_pin);
    const int button_value = digitalRead(button_pin);

    // if both handle events are the same, they could be in the same `if`.
    if (motion_value == HIGH) {
        // motion detected
    }

    if (button_value == HIGH) {
        // button pressed
    }

    // should delay be longer when motion is detected to avoid multiple checks?
    delay(100);
}
