/* Senseur de distance Ultrason HC-SR04:
 VCC sur Nodemcu 5v (VIN) 
 GND sur Arduino GND
 Echo sur Arduino broche 4 (D2) 
 Trig sur Arduino broche 5 (D1)
 
/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/

// Including the ESP8266 WiFi library
#include <ESP8266WiFi.h>
//#include "DHT.h"

// Uncomment one of the lines below for whatever DHT sensor type you're using!
//#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// Definition des pin pour le capteur de distance
#define echoPin 4 // broche Echo 
#define trigPin 5 // broche Trigger (declenchement)
#define LEDPin 13 // LED de la carte Ardiono (branché sur la broche 13)


// Replace with your network details
//const char* ssid = "devolo-rdc";
const char* ssid = "WiFi-2.4-b97a";
const char* password = "366A3E94DD";

// Web Server on port 80
WiFiServer server(80);

// DHT Sensor
//const int DHTPin = 2;
// Initialize DHT sensor.
//DHT dht(DHTPin, DHTTYPE);

// Temporary variables
//static char celsiusTemp[7];
//static char fahrenheitTemp[7];
//static char humidityTemp[7];
static char DistStr[7];
int maximumRange = 400; // distance Maximale acceptée (en cm)
int minimumRange = 0;   // distance Minimale acceptée (en cm)
long duration;
float distance, distance1; // Durée utilisé pour calculer la distance

// only runs once on boot
void setup() {
  // Initializing serial port for debugging purposes
  Serial.begin(9600);
  delay(10);

  //dht.begin();
  
  // Connecting to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Starting the web server
  server.begin();
  Serial.println("Web server running. Waiting for the ESP IP...");
  delay(10000);
  
  // Printing the ESP IP address
  Serial.println(WiFi.localIP());
 // Activer les broches
 pinMode(trigPin, OUTPUT);
 pinMode(echoPin, INPUT);
 pinMode(LEDPin, OUTPUT); // activer la LED sur la carte (si nécessaire)
}







// Partie du code continuellement exécuté
// Son but est d'effectuer un cycle de détection pour déterminer 
// la distance de l'objet le plus proche (par réverbération de 
// l'onde sur ce dernier)
//

float ReadDistance()
{
// Envoi une impulsion de 10 micro seconde sur la broche "trigger" 
 digitalWrite(trigPin, LOW); 
 delayMicroseconds(2); 

 digitalWrite(trigPin, HIGH);
 delayMicroseconds(10); 
 
 digitalWrite(trigPin, LOW);

 // Attend que la broche Echo passe au niveau HAUT 
 // retourne la durée
 duration = pulseIn(echoPin, HIGH);
 //Serial.println("duration");
 //Serial.println(duration);
 //Calculer la distance (en cm, basé sur la vitesse du son).
 distance = duration/58.2;
 
 // Si la distance mesurée est HORS des valeurs acceptables
 if (distance >= maximumRange || distance <= minimumRange){
    /* Envoyer une valeur négative sur la liaison série.
       Activer la LED pour indiquer que l'erreur */
   Serial.println("-1");
   digitalWrite(LEDPin, HIGH); 
   return (-1);
 }
 else {
   /* Envoyer la distance vers l'ordinateur via liaison série.
      Eteindre la LED pour indiquer une lecture correcte. */
   Serial.println(distance);
   digitalWrite(LEDPin, LOW); 
   return (distance);
 }
 
  
}


void loop() 
{
   // Listenning for new clients
  WiFiClient client = server.available();
  float d = ReadDistance();
  Serial.print("Distance: ");
  Serial.print(d);
  if (client) {
    Serial.println("New client");
    // bolean to locate when the http request ends
    boolean blank_line = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        
        if (c == '\n' && blank_line) {
            // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
            //float d = ReadDistance();
            // Check if any reads failed and exit early (to try again).
            if (isnan(d)) {
              Serial.println("Failed to read from Distance sensor!");
              strcpy(DistStr,"Failed");     
            }
            else{
              // Computes temperature values in Celsius + Fahrenheit and Humidity
              //float d = ReadDistance();  
              dtostrf(d, 6, 2, DistStr);             
              // You can delete the following Serial.print's, it's just for debugging purposes
              Serial.print("WEB Distance: ");
              Serial.print(d);
            }
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            // your actual web page that displays temperature and humidity
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            client.println("<head></head><body><h1>ESP8266 - Distance</h1>");
            client.println("<h3>Mon IP: ");
            client.println(WiFi.localIP());
             client.println("</h3>");
            client.println("<h3>Distance en Cm: ");
            client.println(DistStr);
            client.println("cm</h3><h3>");
            client.println("</body></html>");     
            break;
        }
        if (c == '\n') {
          // when starts reading a new line
          blank_line = true;
        }
        else if (c != '\r') {
          // when finds a character on the current line
          blank_line = false;
        }
      }
    }  
    // closing the client connection
    delay(1);
    client.stop();
    Serial.println("Client disconnected.");
  }
}
