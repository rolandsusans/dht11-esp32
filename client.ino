#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"

// Uncomment one of the lines below for whatever DHT sensor type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// Wifi configuration 
const char* ssid = "_pentagon_";
const char* password =  "_pentagon_";
const char* payloadUrl = "http://example.com/";
const char* token = "Token 123";

// DHT Sensor
const int DHTPin = 16; // Data PIN for DHT sensor
const int LED_PIN = 2;// PINOUT for internal LED
const int REPEAT_TIME = 10000; // Timeout between measurments

// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);


class Measurment
{
public:
    float temperature;
    float humidity;

    Measurment* get()
    {
        this->temperature = dht.readTemperature();
        this->humidity = dht.readHumidity();
        return this;
    };

    char* asJson()
    {
        static char cT[7], hT[7];

        if (isnan(this->temperature) || isnan(this->humidity))
        {
            strcpy(cT,"null");
            strcpy(hT, "null");
        }
        else
        {
            dtostrf(this->temperature, 6, 2, cT);
            dtostrf(this->humidity, 6, 2, hT);
        }
        static char result[100];

        strcpy(result,"{ \"data\":{ \"temperature\":");
        strcat(result,cT); // append temperature
        strcat(result,", \"humidity\":"); // append string two to the result.
        strcat(result,hT); // append string two to the result.
        strcat(result,"} }"); // append string two to the result.

        return result;

    }

};



void setup()
{

    pinMode(LED_PIN, OUTPUT); //initialize build in LED
    dht.begin();// initialize the DHT sensor

    //Initialize serial and wait for port to open:
    Serial.begin(115200);
    while(!Serial)
    {
        ; // wait for serial port to connect. Needed for native USB port only
    }

    Serial.begin(115200);
    blinkLed(4000,10,LED_PIN);   //Delay needed before calling the WiFi.begin


    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)   //Check for the connection
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }

    // We start by connecting to a WiFi network
    Serial.print("Connecting to ");
    Serial.println(ssid);

    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

}

void loop()
{

    if(WiFi.status() == WL_CONNECTED)    //Check WiFi connection status
    {
        digitalWrite(LED_PIN, HIGH);// keep blue light while action
        HTTPClient http;// initialize http client

        http.begin(payloadUrl);  
        http.addHeader("Content-Type", "application/json");//Specify content-type header
        http.addHeader("Authorization", token);//Specify content-type header
        Measurment* m =  new Measurment();
        char payload[100];
        strcpy(payload,m->get()->asJson());
      
        int httpResponseCode = http.POST(payload);   //Send the actual POST request
        Serial.print(payloadUrl);
        Serial.print(" POST: ");
        Serial.print(payload);
        
        
        if(199 < httpResponseCode && httpResponseCode < 227)
        {
            
            String response = http.getString();  //Get the response to the request
            Serial.print(" --> Respone code:");
            Serial.println(httpResponseCode);   //Print return code

        }
        else
        {

            Serial.print(" --> Error: ");
            Serial.println(httpResponseCode);
            Serial.println(http.getString());

        }

        http.end();  //Free resources


    }
    else
    {

        Serial.println("Error in WiFi connection");

    }

    digitalWrite(LED_PIN, LOW);
    delay(REPEAT_TIME);  //Send a request every 10 seconds

}

void blinkLed(int time, int blinks, int pinNum)
{

    int cycleTime  = time / blinks;

    for(int i = 0; i < blinks; i++)
    {
        digitalWrite(pinNum, HIGH); // sets the digital pin 13 on
        delay(cycleTime/2);                  // waits for a second
        digitalWrite(pinNum, LOW);  // sets the digital pin 13 off
        delay(cycleTime/2);                  // waits for a second
    }

    digitalWrite(pinNum, LOW);

}
