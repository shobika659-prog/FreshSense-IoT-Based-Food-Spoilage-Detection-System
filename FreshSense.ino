#include <WiFi.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>

// WiFi Credentials
const char* ssid = "SMART"; // SSID of your Wi-Fi
const char* password = "123456789"; // Password of your Wi-Fi

WiFiServer server(80);

// Pin definitions
int gas_pin = 34;  // For ESP32, use ADC pins
int gas_val = 0;
const int trigPin = 12;
const int echoPin = 13;

// DHT Sensor setup
#define DHTPIN 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float h = 0, t = 0, f = 0;

// LCD setup
LiquidCrystal_I2C lcd(0x3F, 16, 2); // Use your I2C address and LCD dimensions

void setup() {
  // Start the DHT sensor and LCD
  dht.begin();
  lcd.init();
  lcd.setBacklight(1);
  lcd.clear();
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password); // Connect to Wi-Fi
  while (WiFi.status() != WL_CONNECTED) { // Wait until connected
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  server.begin();
  Serial.println("Server started");
  Serial.println(WiFi.localIP()); // Print the IP address
  delay(3000);
}

void loop() {
  // Read DHT sensor
  h = dht.readHumidity();
  t = dht.readTemperature();
  f = dht.readTemperature(true);
  
  // Read Gas Sensor
  gas_val = analogRead(gas_pin);
  
  // Print sensor data to serial monitor
  Serial.print("Humidity: "); Serial.println(h);
  Serial.print("Temperature (C): "); Serial.println(t);
  Serial.print("Temperature (F): "); Serial.println(f);
  Serial.print("Gas Value: "); Serial.println(gas_val);

  // Read Ultrasonic Distance Sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH);
  float distanceCm = duration * 0.034 / 2;
  
  // Print distance
  Serial.print("Distance (cm): "); Serial.println(distanceCm);

  // Update LCD Display
  lcd.setCursor(0, 0);
  lcd.print("Temp: " + String(t) + "C");
  lcd.setCursor(0, 1);
  lcd.print("Gas: " + String(gas_val));

  // Handle client request
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  String req = client.readStringUntil('\r');
  Serial.println("Request: " + req);

  // Handle GPIO requests
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE html><html><head><title>Environment Monitoring</title><style>";
  s += "a:link {background-color: YELLOW; text-decoration: none;} table, th, td {text-align: center; padding: 8px;} </style>";
  s += "<meta http-equiv='refresh' content='3'></head><body bgcolor='lightgreen'>";
  s += "<h1 ALIGN=CENTER>FOOD MONITORING</h1><h2 ALIGN=CENTER>Sensor Details</h2><table border=1 ALIGN=CENTER>";
  
  // Display sensor data in a table
  s += "<tr><th>Humidity (%)</th><td><h2>" + String(h) + "</h2></td></tr>";
  s += "<tr><th>Temperature (C)</th><td><h2>" + String(t) + "</h2></td></tr>";
  s += "<tr><th>Freshness</th><td><h2>" + String(gas_val) + "</h2></td></tr>";
  s += "<tr><th>Moisure )</th><td><h2>" + String(distanceCm) + "</h2></td></tr>";
  s += "</table><br>";
  
  s += "</body></html>";

  // Send the response to the client
  client.print(s);
  delay(100);
}