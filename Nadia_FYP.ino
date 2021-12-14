#define BLYNK_TEMPLATE_ID "TMPLFVCxWV9f"
#define BLYNK_DEVICE_NAME "Nadia FYP"
#define BLYNK_AUTH_TOKEN "JGpAa_5TNg6d7UdJt7h_acsRvmlfHopy"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "DHT.h"

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

char auth[] = BLYNK_AUTH_TOKEN;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "11promax";
char pass[] = "lullabypeet";

// Initialize Telegram BOT
#define BOTtoken "2040496907:AAGziT9wP4po9IHwQNOYA88tQeV_ym_VWko"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "550056774"
  
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);


//DHT11 Blynk
DHT dht;

// define the GPIO connected with Relays
#define RelayPin1 21 //D21

#define VPIN_BUTTON_1  V1
#define VPIN_BUTTON_2  V6

int toggleState_1 = 0; //Define integer to remember the toggle state for relay 1
 
int automationSwitch = 0;

BlynkTimer timer;

//Soil Moisture Declaration
int SoilSensor = A0; //VP
int Soilsensorvalue = 0;
int Soiloutputvalue = 0;

//Water Level Declaration
#define Water_POWER_PIN  32 //D32
#define Water_SIGNAL_PIN A7 //A7
int WaterLevel = 0; // variable to store the sensor value


BLYNK_CONNECTED() 
{  
  // Request the latest state from the server  
  Blynk.syncVirtual(VPIN_BUTTON_1);
  Blynk.syncVirtual(VPIN_BUTTON_2);
}

// When App button is pushed - switch the state  
BLYNK_WRITE(VPIN_BUTTON_1) 
{
  toggleState_1 = param.asInt();  
  digitalWrite(RelayPin1, toggleState_1);  
}
BLYNK_WRITE(VPIN_BUTTON_2) 
{
  automationSwitch = param.asInt();
}

void automation()
{
    Soilsensorvalue=analogRead(SoilSensor);
    Soiloutputvalue=map(Soilsensorvalue,4095,1610,0,100); //100,0 pottupaaru

    Serial.print("Soil Moiture Level:");
    Serial.print(Soiloutputvalue);

    if (Soiloutputvalue>65)
    {
      delay(2000);

      digitalWrite(RelayPin1, LOW); // turn off relay 1
      toggleState_1 = 0;
      Serial.println("Pump OFF");
      bot.sendMessage(CHAT_ID, "Pump 1 is turning Off", "");
      Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);   // Update Button Widget
    }

    else if (Soiloutputvalue<25)
    {
      delay(2000);

      digitalWrite(RelayPin1, HIGH); // turn on relay 1
      toggleState_1 = 1;
      Serial.println("Pump ON");
      bot.sendMessage(CHAT_ID, "Pump 1 is turning ON", "");
      Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);   // Update Button Widget
    }

    delay(1000);
    Blynk.virtualWrite(V4,Soiloutputvalue);
}

void manual()
{
  //Soil Moisture Sensor Read And Print
  Soilsensorvalue=analogRead(SoilSensor);
  Soiloutputvalue=map(Soilsensorvalue, 4095, 1610, 0, 100); 

  Serial.print("Soil Moiture Level:");
  Serial.print(Soiloutputvalue);

  delay(1000);
  Blynk.virtualWrite(V4,Soiloutputvalue);
}

void setup() 
{
  Serial.begin(115200); /* Define baud rate for serial communication */
  
  dht.setup(25); //D25

  //Relay Pin
  pinMode(RelayPin1, OUTPUT);

  //During Starting all Relays should TURN OFF  
  digitalWrite(RelayPin1, toggleState_1);

  //Water Level Sensor
  pinMode(Water_POWER_PIN, OUTPUT);   // configure pin as an OUTPUT
  digitalWrite(Water_POWER_PIN, LOW); // turn the sensor OFF

  Blynk.begin(auth, ssid, pass);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  bot.sendMessage(CHAT_ID, "Bot started up", "");
}

void loop() 
{
  if(automationSwitch == 1)
  {
    automation();
  }
  else if(automationSwitch == 0)
  {
    manual();
  }

  //Water Level Sensor Read and Print
  digitalWrite(Water_POWER_PIN, HIGH);  // turn the sensor ON
  delay(10);                      // wait 10 milliseconds
  WaterLevel = analogRead(Water_SIGNAL_PIN); // read the analog value from sensor
  digitalWrite(Water_POWER_PIN, LOW);   // turn the sensor OFF

  Serial.print("Water Level: ");
  Serial.println(WaterLevel);

  delay(1000);

  Blynk.virtualWrite(V5,WaterLevel);

  if (Water_SIGNAL_PIN > 3000 )
  {
    bot.sendMessage(CHAT_ID, "Low Water Level, Please Top Up more Water...", "");
  }

  //To Update Serial Print With Latest Pump State
  if(toggleState_1 == 1)
  {     
    Serial.println("Pump ON");
  }

  else if (toggleState_1 == 0)
  {   
    Serial.println("Pump OFF");  
  }  
  
  delay(500);

  //DHT11 Sensor Read And Print
  Serial.println("Status\tHumidity (%)\tTemperature (C)\t(F)");
  delay(dht.getMinimumSamplingPeriod()); /* Delay of amount equal to sampling period */
  float humidity = dht.getHumidity();/* Get humidity value */
  float temperature = dht.getTemperature();/* Get temperature value */
  Serial.print(dht.getStatusString());/* Print status of communication */
  Serial.print("\t");
  Serial.print(humidity, 1);
  Serial.print("\t\t");
  Serial.print(temperature, 1);
  Serial.print("\t\t");

  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V2, humidity);
  Blynk.virtualWrite(V3, temperature);

  //Blynk WiFi Status Update
  if (WiFi.status() != WL_CONNECTED)  
  {  
    Serial.println("WiFi Not Connected");  
  }  
  else  
  {  
    Serial.println("WiFi Connected");  
  }  
  Blynk.run();
}
