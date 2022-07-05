#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid     = "9R";
const char* password = "Nginx123";

const int gasAnalog = 34;

WiFiServer server(80);

String header;

const int motor = 2;

IPAddress local_IP(192, 168, 0, 125);
IPAddress gateway(192, 168, 0, 1);

IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);  
IPAddress secondaryDNS(8, 8, 4, 4);

void setup() {
  Serial.begin(115200);
  pinMode(motor, OUTPUT);
  digitalWrite(motor, LOW);

  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }
  
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop(){
  WiFiClient client = server.available();  

//   int analogGas = 999;
   int analogGas = analogRead(gasAnalog);
  Serial.println(analogGas);
   

   if (analogGas >= 3000 && analogGas < 3100) {
     HTTPClient http;
     String serverPath = "http://192.168.0.217:3000/gas-sensor/" + String(analogGas);
     http.begin(serverPath.c_str());
     int httpResponseCode = http.GET();  
   } else if (analogGas >= 3100) {
     digitalWrite(motor, HIGH);
     HTTPClient http;
     String serverPath = "http://192.168.0.217:3000/esp32/20Intensity%20is%20High=>%20Opening%20the%20Window";
     http.begin(serverPath.c_str());
     int httpResponseCode = http.GET();
   }


  if (client) {                            
    Serial.println("New Client.");       
    String currentLine = "";                
    while (client.connected()) {            
      if (client.available()) {             
        char c = client.read();           
        Serial.write(c);                
        header += c;
        if (c == '\n') {                  
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            if (header.indexOf("GET /window/open") >= 0) {
              Serial.println("GPIO 26 on");
              if (digitalRead(motor) == HIGH) {
                client.println("HIGH");
                break;
              }
              digitalWrite(motor, HIGH);
            } else if (header.indexOf("GET /window/close") >= 0) {
              if (digitalRead(motor) == LOW) {
                client.println("LOW");
                break;
              }
              digitalWrite(motor, LOW);
            }

            client.println("ok");

            break;
          } else { 
            currentLine = "";
          }
        } else if (c != '\r') { 
          currentLine += c;  
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
