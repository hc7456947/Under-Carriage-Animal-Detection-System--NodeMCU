#include <WiFi.h>
#include "ThingSpeak.h"
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <LiquidCrystal_I2C.h>

uint32_t tsLastReport = 0;

int inputPin = 19;
const int led = 13;
const char* ssid = "OPPO A15s";   // your network SSID (name)
const char* password = "chauhan1234";   // your network password
WiFiClient client;

unsigned long myChannelNumber = 1;
const char* myWriteAPIKey = "M7QHA66NUJX457ZF";

unsigned long lastTime = 0;
unsigned long timerDelay = 20000;
#define trigPin1 2
#define echoPin1 4
#define trigPin2 5
#define echoPin2 18

long duration, distance, RearSensor, BackSensor, FrontSensor, UnderSensor;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Function prototype
void SonarSensor(int trigPin, int echoPin);

void setup()
{
  pinMode(inputPin, INPUT);
  mlx.begin();
  pinMode(trigPin1, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  Serial.begin(115200);
  ThingSpeak.begin(client);
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("ANIMAL GUARDIAN");
  delay(5000);
}

void loop() {
    digitalWrite(led, HIGH);
    lcd.clear();
    lcd.backlight();
    lcd.setCursor(2, 1);
    lcd.print("READY TO GO");

    Serial.print("Ambient = ");
    Serial.print(mlx.readAmbientTempC());
    Serial.print("*C\tObject = ");
    Serial.print(mlx.readObjectTempC());
    Serial.println("*C");
    Serial.print("Ambient = ");
    Serial.print(mlx.readAmbientTempF());
    Serial.print("*F\tObject = ");
    Serial.print(mlx.readObjectTempF());
    Serial.println("*F");

    int tt = mlx.readObjectTempC(); // Changed int to float
    int ts = 27; // You might want to adjust this threshold according to your needs
    Serial.println(tt); // Corrected Serial printing
    int val = digitalRead(inputPin);
    Serial.println(val);

    if (val == 1) {
        digitalWrite(led, LOW);
        lcd.clear();
        lcd.backlight();
        lcd.setCursor(0, 0);
        lcd.print("ANIMAL  DETECTED");
        delay(1000);
    } else if (val == 0) {
        if (tt > ts) {
            digitalWrite(led, LOW);
            lcd.clear();
            lcd.backlight();
            lcd.setCursor(0, 1);
            lcd.print("ANIMAL  DETECTED");
            delay(1000);
        } else {
            SonarSensor(trigPin1, echoPin1);
            UnderSensor = distance;
            Serial.println(RearSensor);
            if (distance < 5) {
                lcd.clear();
                lcd.backlight();
                lcd.setCursor(0, 0);
                lcd.print("OBJECT  DETECTED");
                digitalWrite(led, LOW);
                delay(1000);
            }

            SonarSensor(trigPin2, echoPin2);
            RearSensor = distance;
            Serial.println(FrontSensor);
            if (distance < 4) {
                lcd.clear();
                lcd.backlight();
                lcd.setCursor(0, 1);
                lcd.print("OBJECT  DETECTED");
                digitalWrite(led, LOW);
                delay(1000);
            }
        }
    }

  // THINGSPEAK
  if ((millis() - lastTime) > timerDelay)
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.print("Attempting to connect");
      while (WiFi.status() != WL_CONNECTED)
      {
        WiFi.begin(ssid, password);
        delay(5000);
      }
      Serial.println("\nConnected.");
    }

    SonarSensor(trigPin1, echoPin1);
    RearSensor = distance;
    ThingSpeak.setField(1, distance);

    SonarSensor(trigPin2, echoPin2);
    FrontSensor = distance;
    ThingSpeak.setField(2, distance);

    int tt = mlx.readObjectTempC();
    ThingSpeak.setField(3, tt);

    int val = digitalRead(inputPin);
    ThingSpeak.setField(4, val);

    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if (x == 200)
    {
      Serial.println("Channel update successful.");
    }
    else
    {
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }

    lastTime = millis();
  }
}

// Function definition
void SonarSensor(int trigPin, int echoPin)
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration / 2) / 29.1;
}
