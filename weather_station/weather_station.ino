#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "ArduinoJson.h"
#include "icons.h"

// see https://wiki.wemos.cc/products:d1:d1_mini for GPIO pin values.
const int motion_pin = 4;
const int button_pin = 0;
const int audio_pin = 15; // pin D8

// digital pins for the LED matrix
const int DIN = 13;
const int CS = 12;
const int CLK = 14;

const char* ssid = "arduino";
const char* password = "arduinotest";

const char* url = "https://baran-it.com/wst/stations/1/getWeather";
// sha1 fingerprint for HTTPS
const char* fingerprint = "39 FC 06 60 1D 63 DB CC C1 12 BF A7 65 D2 59 40 4B 70 06 F1";

//const int notification_tone[15] = { 165, 165, 349, 784, 784, 349, 165, 73, 33, 33, 73, 165, 165, 73, 73 };
const int notification_tone[50] = { 103, 839, 462, 336, 257, 269, 228, 929, 757, 499, 159, 988, 249, 671, 523, 165, 911, 470, 994, 883, 294, 290, 946, 337, 121, 526, 414, 250, 270, 854, 125, 354, 134, 653, 585, 295, 227, 406, 261, 919, 966, 481, 242, 36, 524, 356, 440, 37, 42, 841 };

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
}

void putByte(byte data)
{
  byte i = 8;
  byte mask;
  while(i > 0)
  {
    mask = 0x01 << (i - 1);      // get bitmask
    digitalWrite( CLK, LOW);   // tick
    if (data & mask) digitalWrite(DIN, HIGH);
    else             digitalWrite(DIN, LOW);
    digitalWrite(CLK, HIGH);   // tock
    --i;                         // move to lesser bit
  }
}

void maxSingle( byte reg, byte col)
{
    digitalWrite(CS, LOW);       // begin     
    putByte(reg);                  // specify register
    putByte(col);                  // put data
    digitalWrite(CS,HIGH); 
}

void initMax7219()
{
    //initiation of the max 7219
    maxSingle(max7219_reg_scanLimit, 0x07);      
    maxSingle(max7219_reg_decodeMode, 0x00);     // using an led matrix (not digits)
    maxSingle(max7219_reg_shutdown,   0x01);     // not in shutdown mode
    maxSingle(max7219_reg_displayTest, 0x00);    // no display test
    for (int e = 1; e <= 8; e++) maxSingle(e,0); // empty registers, turn all LEDs off 
    maxSingle(max7219_reg_intensity, 0x0f & 0x0f);  // the first 0x0f is the value you can set
}

void drawShape(const byte shape[]){
    for(int i = 1; i <= 8; i++) {
        maxSingle(i, shape[i]);
    }
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

int determineIcon(const int weather_code)
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
    // play a tone.
    /* Serial.println("Play a tone"); */
    /* for (int i = 0; i < 50; i++) { */
    /*     Serial.print("Tone: "); */
    /*     Serial.println(notification_tone[i]); */
    /*     tone(audio_pin, notification_tone[i], 150); */
    /*     delay(150); */
    /* } */
    tone(audio_pin, 200, 1000);
    delay(150);
}

// should the event type be sent to this function (e.g. motion vs button event)?
void handleEvent()
{
    // Handle both motion and button pressed events here.
    const JsonObject& weatherJson = getWeatherInformation();
    int icon = determineIcon(weatherJson["weather_code"]);
    if (icon >= 0) {
        drawShape(IMAGES[icon]);
        if (weatherJson["sound"])
            playTone();
    }
}

void loop()
{
    int motion_value = digitalRead(motion_pin);
    int button_value = digitalRead(button_pin);

    // if both handle events are the same, they could be in the same `if`.
    if (motion_value == HIGH) {
        // motion detected
        Serial.println("Motion detected");
        handleEvent();
    }

    if (button_value == HIGH) {
        // button pressed
    }

    // should delay be longer when motion is detected to avoid multiple checks?
    delay(100);
}
