/*Nom: Hans Simo
  Date: 2023-04-10
  
  Sources:https://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-web-server-arduino-ide/
        https://fontawesome.com/start
        https://chat.openai.com/chat

*/


#include <Arduino.h>


#include "WiFi.h"

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>


// Pin SDA et SCL pour l'I2C
#define SDA_PIN 21
#define SCL_PIN 22

const int LED_PIN = 25;


Adafruit_BME280 bme; // Créer une instance de BME280

// Replace with your network credentials
const char* ssid = "BELL369";
const char* password = "E39E7FC92339";

//const char* ssid = "hans";
//const char* password = "12345678";





// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


void updateLED(float temperature) {
  if (temperature > 25) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}

String readBMETemperature() {
  float t = bme.readTemperature();

  if (isnan(t)) {    
    Serial.println("Failed to read from BME sensor!");
    return "--";
  }
  else {
    Serial.println(t);
    updateLED(t); // mettre à jour l'état de la LED
    return String(t);
  }
}

String readBMEHumidity() {
  
  float h = bme.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from BME sensor!");
    return "--";
  }
  else {
    Serial.println(h);
    return String(h);
  }
}

String readBMEPressure() {
  
  float p = bme.readPressure();
  if (isnan(p)) {
    Serial.println("Failed to read from BME sensor!");
    return "--";
  }
  else {
    Serial.println(p);
    return String(p);
  }
}

String readBMEAltitude() {
  
  float a = bme.readAltitude(1013.25);
  if (isnan(a)) {
    Serial.println("Failed to read from BME sensor!");
    return "--";
  }
  else {
    Serial.println(a);
    return String(a);
  }
}


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP32 Station Meteo</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">&percnt;</sup>
  </p>
  <p>
    <i class="fas fa-compass" style="color:#015dd6;"></i> 
    <span class="dht-labels">Pressure</span>
    <span id="pressure">%PRESSURE%</span>
    <sup class="units">ph;</sup>
  </p>
  <p>
    <i class="fas fa-mountain" style="color:#068dd6;"></i> 
    <span class="dht-labels">Altitude</span>
    <span id="altitude">%ALTITUDE%</span>
    <sup class="units">m;</sup>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("pressure").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/pressure", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("altitude").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/altitude", true);
  xhttp.send();
}, 10000 ) ;
</script>
</html>)rawliteral";

// Replaces placeholder with DHT values
String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return readBMETemperature();
  }
  else if(var == "HUMIDITY"){
    return readBMEHumidity();
  }
   else if(var == "PRESSURE"){
    return readBMEPressure();
  }
   else if(var == "ALTITUDE"){
    return readBMEAltitude();
  }
  return String();
}

void setup() {
  Serial.begin(9600); // Initialiser la communication série
  pinMode(LED_PIN, OUTPUT);
  Wire.begin(SDA_PIN, SCL_PIN); // Initialiser l'I2C
  if (!bme.begin(0x76)) { // Initialiser le capteur BME280
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                  Adafruit_BME280::SAMPLING_X1,
                  Adafruit_BME280::SAMPLING_X1,
                  Adafruit_BME280::SAMPLING_X1,
                  Adafruit_BME280::FILTER_OFF);


  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readBMETemperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readBMEHumidity().c_str());
  });
  server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readBMEPressure().c_str());
  });
  server.on("/altitude", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readBMEAltitude().c_str());
  });


  AsyncElegantOTA.begin(&server);    // Start ElegantOTA

  // Start server
  server.begin();
  Serial.println("HTTP server started");









}
 
void loop(){
  
}