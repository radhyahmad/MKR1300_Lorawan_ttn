#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <MKRWAN.h>

LoRaModem lorawan;

String appEui = "";
String appKey = "";

#define DHTPIN 6
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
unsigned long lastData = 0;

void setup() {

  Serial.begin(9600);
  dht.begin();
  while (!Serial);

  if (!lorawan.begin(US915))
  {
    Serial.println("Failed to start module");
    while (1)
    {
      
    }
    
  };

  Serial.print("Your module version is: ");
  Serial.println(lorawan.version());
  Serial.print("Your device EUI is: ");
  Serial.println(lorawan.deviceEUI());

  int connected = lorawan.joinOTAA(appEui, appKey);
  if (!connected)
  {
    Serial.println("Something went wrong: are you indoor? Move near to a window and retry");
    while (1)
    {
      
    }

  }

  Serial.println("Successfully joined the network!");
  lorawan.minPollInterval(60);
  lorawan.setADR(true);
  lorawan.dataRate(3);
  delay(200);
  
}

void loop() {

  uint16_t temperature = dht.readTemperature(false) * 100;
  uint16_t humidity = dht.readHumidity(false) * 100;

  if (isnan(temperature) || isnan(humidity))
  {
    Serial.println("Failed to read DHT");
    return;
  }

  byte payload[4];
  payload[0] = highByte(temperature);
  payload[1] = lowByte(temperature);
  payload[2] = highByte(humidity);
  payload[3] = lowByte(humidity);

  unsigned long now = millis();

  if (now - lastData > 1000*60)
  {
    lastData = now;
    lorawan.beginPacket();
    lorawan.write(payload, sizeof(payload));

    int err = lorawan.endPacket(true);
    if (err > 0)
    {
      Serial.println("Message sent correctly");
    }
    else
    {
      Serial.println("Error sending message :(");
      Serial.println("(you may send a limited amount of messages per minute, depending on the signal strength");
      Serial.println("it may vary from 1 message every couple of seconds to 1 message every minute)");
    }
    
  }
  
}