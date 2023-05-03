/*************************************************************
  Blynk is a platform with iOS and Android apps to control
  ESP32, Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build mobile and web interfaces for any
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: https://www.blynk.io
    Sketch generator:           https://examples.blynk.cc
    Blynk community:            https://community.blynk.cc
    Follow us:                  https://www.fb.com/blynkapp
                                https://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************
  This example runs directly on ESP32 chip.

  NOTE: This requires ESP32 support package:
    https://github.com/espressif/arduino-esp32

  Please be sure to select the right ESP32 module
  in the Tools -> Board menu!

  Change WiFi ssid, pass, and Blynk auth token to run :)
  Feel free to apply it to any other example. It's simple!
 *************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

/* Fill in information from Blynk Device Info here */
//#define BLYNK_TEMPLATE_ID           "TMPxxxxxx"
//#define BLYNK_TEMPLATE_NAME         "Device"
//#define BLYNK_AUTH_TOKEN            "YourAuthToken"
#define BLYNK_TEMPLATE_ID "TMPL6UqT9PTqj"
#define BLYNK_TEMPLATE_NAME "PetFeeder"
#define BLYNK_AUTH_TOKEN "9NzOvmE9ftAuf4gqIvFV2WrnLOXlCZsn"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include <Servo.h>
Servo myservo;

int count = 0;
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "MakerSpace_IoT";
char pass[] = "makerspaceiot";

void setup()
{
  // Debug console
  Serial.begin(9600);

  // 서보모터 연결
  myservo.attach(5);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

void loop()
{
  Blynk.run();
  Blynk.virtualWrite(V2, count);
}

BLYNK_WRITE(V0)
{
  int pinValue = param.asInt();
  myservo.write(90);
  delay(1000);
  myservo.write(0);
  count++;
  Blynk.virtualWrite(V2, count);
  Blynk.virtualWrite(V0, !pinValue);

}

BLYNK_WRITE(V1)
{
  int pinValue = param.asInt();
  count = 0;
  Blynk.virtualWrite(V2, count);
  Blynk.virtualWrite(V1, !pinValue);
}
