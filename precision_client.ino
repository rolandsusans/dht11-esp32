#include <WiFi.h>
#include <HTTPClient.h>

//#define DHTTYPE DHT21   // DHT 21 (AM2301)
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

#define MAXTIMINGS 85

// Wifi configuration, POST configuration
const char *ssid = "_pentagon_";
const char *password = "_pentagon_";

//API CONFING
const char *payloadUrl = "http://example.com/"; //Payload URL
const char *token = "Token 123";                //API token

// DHT Sensor
const int DHTPIN = 16;         // Data PIN for DHT sensor
const int LED_PIN = 2;         // PINOUT for internal LED
const int REPEAT_TIME = 30000; // Timeout between measurments

// Debugging
const bool DEBUG = true;            //enables disables debugging to serial output
const int SERIAL_BIT_RATE = 115200; //bit rate for serial logging

class Measurment
{
  public:
    int dht11_dat[5] = {0, 0, 0, 0, 0};
    bool isValid = false;

    Measurment *get()
    {

        uint8_t laststate = HIGH;
        uint8_t counter = 0;
        uint8_t j = 0, i;
        float f; /* fahrenheit */

        this->dht11_dat[0] = dht11_dat[1] = dht11_dat[2] = dht11_dat[3] = dht11_dat[4] = 0;

        /* pull pin down for 18 milliseconds */
        pinMode(DHTPIN, OUTPUT);
        digitalWrite(DHTPIN, LOW);
        delay(18);
        /* then pull it up for 40 microseconds */
        digitalWrite(DHTPIN, HIGH);
        delayMicroseconds(40);
        /* prepare to read the pin */
        pinMode(DHTPIN, INPUT);
        /* detect change and read data */
        for (i = 0; i < MAXTIMINGS; i++)
        {
            counter = 0;
            while (digitalRead(DHTPIN) == laststate)
            {
                counter++;
                delayMicroseconds(1);
                if (counter == 255)
                {
                    break;
                }
            }
            laststate = digitalRead(DHTPIN);

            if (counter == 255)
                break;

            /* ignore first 3 transitions */
            if ((i >= 4) && (i % 2 == 0))
            {
                /* shove each bit into the storage bytes */
                this->dht11_dat[j / 8] <<= 1;
                if (counter > 16)
                    this->dht11_dat[j / 8] |= 1;
                j++;
            }
        }

        /*
         * check we read 40 bits (8bit x 5 ) + verify checksum in the last byte
         * print it out if data is good
         */
        if ((j >= 40) &&
            (this->dht11_dat[4] == ((this->dht11_dat[0] + this->dht11_dat[1] + this->dht11_dat[2] + this->dht11_dat[3]) & 0xFF))) //CRC CHECK
        {
            if (DEBUG)
            {
                printf("Humidity = %d.%d Temperature = %d.%d *C\n",
                       this->dht11_dat[0], this->dht11_dat[1], this->dht11_dat[2], this->dht11_dat[3]);
            }

            this->isValid = true;
        }
        else
        {
            if (DEBUG)
            {
                printf("H= %d.%d T= %d.%d\n", this->dht11_dat[0], this->dht11_dat[1], this->dht11_dat[2], this->dht11_dat[3]);
                printf("Data not good, skip\n");
            }
        }

        return this;
    };

    char *asJson()
    {
        static char cT[8], hT[8];

        if (!this->isValid)
        {
            strcpy(cT, "null");
            strcpy(hT, "null");
        }
        else
        {
            snprintf(hT, sizeof(hT), "%d.%d", this->dht11_dat[0], this->dht11_dat[1]);
            snprintf(cT, sizeof(cT), "%d.%d", this->dht11_dat[2], this->dht11_dat[3]);
        }
        static char result[100];

        strcpy(result, "{ \"data\":{ \"temperature\":");
        strcat(result, cT);                // append temperature
        strcat(result, ", \"humidity\":"); // append string two to the result.
        strcat(result, hT);                // append string two to the result.
        strcat(result, "} }");             // append string two to the result.

        return result;
    }
};

void setup()
{

    pinMode(LED_PIN, OUTPUT); //initialize build in LED
    if (DEBUG)
    {
        //Initialize serial and wait for port to open:
        Serial.begin(SERIAL_BIT_RATE);
        while (!Serial)
        {
            ; // wait for serial port to connect. Needed for native USB port only
        }

        Serial.begin(SERIAL_BIT_RATE);
    }

    blinkLed(4000, 10, LED_PIN); //Delay needed before calling the WiFi.begin

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) //Check for the connection
    {
        blinkLed(1000, 4, LED_PIN);
        if (DEBUG)
        {
            Serial.println("Connecting to WiFi...");
        }
    }

    if (DEBUG)
    {
        // We start by connecting to a WiFi network
        Serial.print("Connecting to ");
        Serial.println(ssid);

        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
    }
}

void loop()
{

    if (WiFi.status() == WL_CONNECTED) //Check WiFi connection status
    {
        digitalWrite(LED_PIN, HIGH); // keep blue light while action
        HTTPClient http;             // initialize http client

        http.begin(payloadUrl);
        http.addHeader("Content-Type", "application/json"); //Specify content-type header
        http.addHeader("Authorization", token);             //Specify content-type header
        Measurment *m = new Measurment();
        char payload[100];
        strcpy(payload, m->get()->asJson());

        int httpResponseCode = http.POST(payload); //Send the actual POST request
        if (DEBUG)
        {
            Serial.print(payloadUrl);
            Serial.print(" POST: ");
            Serial.print(payload);

            if (199 < httpResponseCode && httpResponseCode < 227)
            {

                String response = http.getString(); //Get the response to the request
                Serial.print(" --> Respone code:");
                Serial.println(httpResponseCode); //Print return code
            }
            else
            {

                Serial.print(" --> Error: ");
                Serial.println(httpResponseCode);
                Serial.println(http.getString());
            }
        }

        http.end(); //Free resources
    }
    else
    {
        if (DEBUG)
        {
            Serial.println("Error in WiFi connection");
        }

        WiFi.begin(ssid, password);

        while (WiFi.status() != WL_CONNECTED) //Check for the connection
        {
            blinkLed(1000, 2, LED_PIN);
            if (DEBUG)
            {
                Serial.println("Connecting to WiFi...");
            }
        }
    }

    digitalWrite(LED_PIN, LOW);
    delay(REPEAT_TIME); //Send a request every REPEAT_TIME seconds
}

void blinkLed(int time, int blinks, int pinNum)
{

    int cycleTime = time / blinks;

    for (int i = 0; i < blinks; i++)
    {
        digitalWrite(pinNum, HIGH); // sets the digital pin 13 on
        delay(cycleTime / 2);       // waits for a second
        digitalWrite(pinNum, LOW);  // sets the digital pin 13 off
        delay(cycleTime / 2);       // waits for a second
    }

    digitalWrite(pinNum, LOW);
}
