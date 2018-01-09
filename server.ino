#include <WiFi.h>
#include "DHT.h"

// Uncomment one of the lines below for whatever DHT sensor type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// Replace with your network credentials
const char* ssid     = "_pentagon_";
const char* password = "_secret_password_";

WiFiServer server(80);

// DHT Sensor
const int DHTPin = 16; // Data PIN for DHT sensor
const int LED_PIN = 2;

// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

// Temporary variables
static char celsiusTemp[7];
static char fahrenheitTemp[7];
static char humidityTemp[7];

// Client variables 
char linebuf[80];
int charcount=0;

void setup() {
  pinMode(LED_PIN, OUTPUT); //initialize build in LED for blinking while sending data
  dht.begin();// initialize the DHT sensor
  
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  while(!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  // attempt to connect to Wifi network:
  while(WiFi.status() != WL_CONNECTED) {
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    delay(25000);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  server.begin();
}

void loop() {
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New client");
    memset(linebuf,0,sizeof(linebuf));
    charcount=0;
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      digitalWrite(LED_PIN, HIGH);
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        //read char by char HTTP request
        linebuf[charcount]=c;
        if (charcount<sizeof(linebuf)-1) charcount++;
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
            // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
            float h = dht.readHumidity();
            // Read temperature as Celsius (the default)
            float t = dht.readTemperature();
            // Read temperature as Fahrenheit (isFahrenheit = true)
            float f = dht.readTemperature(true);
            // Check if any reads failed and exit early (to try again).
            if (isnan(h) || isnan(t) || isnan(f)) {
              Serial.println("Failed to read from DHT sensor!");
              strcpy(celsiusTemp,"null");
              strcpy(fahrenheitTemp, "null");
              strcpy(humidityTemp, "null");         
            }
            else{
              // Computes temperature values in Celsius + Fahrenheit and Humidity
              float hic = dht.computeHeatIndex(t, h, false);       
              dtostrf(hic, 6, 2, celsiusTemp);             
              float hif = dht.computeHeatIndex(f, h);
              dtostrf(hif, 6, 2, fahrenheitTemp);         
              dtostrf(h, 6, 2, humidityTemp);
          }

          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: application/json");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println();
          client.print("{\"celsius\":");
          client.print(celsiusTemp);
          client.print(", \"fahrenheit\":");
          client.print(fahrenheitTemp);
          client.print(", \"humidity\":");
          client.print(humidityTemp);
          client.print("}");
        
          /**
        
          Serial.print("Humidity: ");
          Serial.print(h);
          Serial.print(" %\t Temperature: ");
          Serial.print(t);
          Serial.print(" *C ");
          Serial.print(f);
          Serial.print(" *F\t Heat index: ");
          Serial.print(hic);
          Serial.print(" *C ");
          Serial.print(hif);
          Serial.print(" *F");
          Serial.print("Humidity: ");
          Serial.print(h);
          Serial.print(" %\t Temperature: ");
          Serial.print(t);
          Serial.print(" *C ");
          Serial.print(f);
          Serial.print(" *F\t Heat index: ");
          Serial.print(hic);
          Serial.print(" *C ");
          Serial.print(hif);
          Serial.println(" *F");

          */
         
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
          memset(linebuf,0,sizeof(linebuf));
          charcount=0;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);

    // close the connection:
    client.stop();
    digitalWrite(LED_PIN, LOW);
    Serial.println("client disconnected");
  }
}