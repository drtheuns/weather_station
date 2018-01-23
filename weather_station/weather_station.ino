#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "ArduinoJson.h"
#include "icons.h"

// see https://wiki.wemos.cc/products:d1:d1_mini for GPIO pin values.
 int motion_pin = 4;
 int audio_pin = 15;

// digital pins for the LED matrix
 int DIN = 13;
 int CS = 12;
 int CLK = 14;

 char* ssid = "arduino";
 char* password = "arduinotest";

 char* url = "https://baran-it.com/wst/stations/1/getWeather";
// sha1 fingerprint for HTTPS
 char* fingerprint = "39 FC 06 60 1D 63 DB CC C1 12 BF A7 65 D2 59 40 4B 70 06 F1";

 int notification_tone[50] = { 103, 839, 462, 336, 257, 269, 228, 929, 757, 499, 159, 988, 249, 671, 523, 165, 911, 470, 994, 883, 294, 290, 946, 337, 121, 526, 414, 250, 270, 854, 125, 354, 134, 653, 585, 295, 227, 406, 261, 919, 966, 481, 242, 36, 524, 356, 440, 37, 42, 841 };

// max7219 registers
byte max7219_reg_noop        = 0x00;
byte max7219_reg_digit0      = 0x01;
byte max7219_reg_digit1      = 0x02;
byte max7219_reg_digit2      = 0x03;
byte max7219_reg_digit3      = 0x04;
byte max7219_reg_digit4      = 0x05;
byte max7219_reg_digit5      = 0x06;
byte max7219_reg_digit6      = 0x07;
byte max7219_reg_digit7      = 0x08;
byte max7219_reg_decodeMode  = 0x09;
byte max7219_reg_intensity   = 0x0a;
byte max7219_reg_scanLimit   = 0x0b;
byte max7219_reg_shutdown    = 0x0c;
byte max7219_reg_displayTest = 0x0f;

void setup()
{
    Serial.begin(115200);

    pinMode(motion_pin, INPUT);
    pinMode(DIN, OUTPUT);
    pinMode(CLK, OUTPUT);
    pinMode(CS, OUTPUT);
    initMax7219();
    delay(10);

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    Serial.println("");
    Serial.println("Wifi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    setupWifi();
}

/*
 * This function will hang the arduino until it connects to the wifi network
 * as specified by the `ssid` and `password` above. It will flash different
 * icons based on the wifi status. (see `icons.h#WIFI_STATES`)
 */
void setupWifi()
{
    WiFi.mode(WIFI_STA); // Otherwise the chip would act as an access point
    WiFi.begin(ssid, password);

    bool wifi_state = false;
    while (WiFi.status() != WL_CONNECTED) {
        drawShape(WIFI_STATES[wifi_state]);
        wifi_state = !wifi_state;
        delay(500);
        Serial.print(".");
    }
    clearScreen();

    // Show a check icon for 0.5 seconds to show that wifi is connected.
    drawShape(WIFI_STATES[2]);
    delay(500);
    clearScreen();
}

void putByte(byte data)
{
    byte i = 8;
    byte mask;
    while(i > 0)
    {
        mask = 0x01 << (i - 1);      // get bitmask
        digitalWrite(CLK, LOW);   // tick
        if (data & mask)
            digitalWrite(DIN, HIGH);
        else
            digitalWrite(DIN, LOW);
        digitalWrite(CLK, HIGH);   // tock
        --i;
    }
}

void maxSingle(byte reg, byte col)
{
    digitalWrite(CS, LOW);
    putByte(reg);
    putByte(col);
    digitalWrite(CS,HIGH);
}

void drawShape(byte shape[])
{
    for(int i = 1; i <= 8; i++) {
        maxSingle(i, shape[i - 1]);
    }
}

void clearScreen()
{
    byte clear[] = {0, 0, 0, 0, 0, 0, 0, 0};
    drawShape(clear);
}

void initMax7219()
{
    //initiation of the max 7219
    maxSingle(max7219_reg_scanLimit, 0x07);
    maxSingle(max7219_reg_decodeMode, 0x00);     // using an led matrix (not digits)
    maxSingle(max7219_reg_shutdown,   0x01);     // not in shutdown mode
    maxSingle(max7219_reg_displayTest, 0x00);    // no display test
    clearScreen();
    maxSingle(max7219_reg_intensity, 0x0f & 0x0f);  // the first 0x0f is the value you can set
}

/*
 * Performs an HTTP request to the weather (station) API.
 * Returns either raw JSON String (to be parsed later,
 * see `getWeatherInformation()`), or an empty String.
 */
String getJson()
{
    Serial.print("Reading from ");
    Serial.println(url);

    HTTPClient client;
    client.begin(url, fingerprint);
    int httpCode = client.GET();
    String payload = "";

    if (httpCode > 0) {
        Serial.println("reached success");
        payload = client.getString();
    }
    Serial.print("Payload: ");
    Serial.println(payload);

    client.end();
    return payload;
}

/*
 * Get the weather information from the API and parse the json string
 * into a usable object.
 */
JsonObject& getWeatherInformation()
{
    StaticJsonBuffer<200> jsonBuffer;
    String jsonString = getJson();
    JsonObject& root = jsonBuffer.parseObject(jsonString);
    return root;
}


/*
 * Determines which icon to use depending on the weather category from
 * OpenWeatherMap. Returns an interger with the index to the icon in
 * icon array (see `icons.h`)
 */
int determineIcon( int weather_code)
{
    // see https://openweathermap.org/weather-conditions
    if (weather_code == 800)
        return 0;
    if (weather_code >= 500 && weather_code < 600)
        return 1;
    if (weather_code >= 600 && weather_code < 700)
        return 2;
    return -1;
}

void playTone()
{
    // Peshmerge's favorite tune, commented out for obvious reasons
    /* for (int i = 0; i < 50; i++) { */
    /*     tone(audio_pin, notification_tone[i], 150); */
    /*     delay(150); */
    /* } */
    tone(audio_pin, 200, 1000);
    delay(150);
}

/*
 * Handle the actual "motion" event. Mainly to keep `loop()` clean.
 */
void handleEvent()
{
    JsonObject& weatherJson = getWeatherInformation();
    int icon = determineIcon(weatherJson["weather_code"]);
    if (icon >= 0) {
        drawShape(IMAGES[icon]);
        if (weatherJson["sound"])
            playTone();
    }
}

void loop()
{
    if (WiFi.status() != WL_CONNECTED) {
        setupWifi();
    }

    int motion_value = digitalRead(motion_pin);

    if (motion_value == HIGH) {
        Serial.println("Motion detected");
        handleEvent();
        delay(5000);
        clearScreen();
    }
    delay(100);
}
