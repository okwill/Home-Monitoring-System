

// Load Wi-Fi library
#include <ESP8266WiFi.h>

// Replace with your network credentials
const char* ssid     = "iPhone";
const char* password = "alohomora";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output5State = "off";
String outputDcState= "off";
String outputIR1State="Empty";
String outputIR2State="open";


// Assign output variables to GPIO pins
const int output5 = D5;

int ENA = D2;
int IN1 = D3;
int IN2 = D4;

 int val1 = 0 ;  
 int val2=0;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(output5, OUTPUT);

  // Set outputs to LOW
  digitalWrite(output5, LOW);

  //dcmotor setup

   pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT); 
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
// IR setup
   pinMode(D7,HIGH);  // Led1 Pin Connected To D7 Pin    
   pinMode(D8,HIGH);  // Led2 Pin Connected To D8 Pin

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /5/on") >= 0) {
              Serial.println("GPIO 5 on");
              output5State = "on";
              digitalWrite(output5, HIGH);
            } else if (header.indexOf("GET /5/off") >= 0) {
              Serial.println("GPIO 5 off");
              output5State = "off";
              digitalWrite(output5, LOW);
            } 
            //turns on dcmotor
            if (header.indexOf("GET /4/on") >= 0) {
              Serial.println("Dc motor on");
              outputDcState = "on";
             // run dc motor
            
             Serial.println("change speed");
              digitalWrite(IN1, LOW);
            digitalWrite(IN2, HIGH);
  
               for (int i = 0; i < 256; i++) {
                analogWrite(ENA, i);
                Serial.println(i);
                 delay(20);
                 }
  
               for (int i = 255; i >= 0; --i) {
                 analogWrite(ENA, i);
              Serial.println(i);
              delay(20);
           }
          digitalWrite(IN1, LOW);
           digitalWrite(IN2, LOW);

          
            } else if (header.indexOf("GET /4/off") >= 0) {
              Serial.println("Dc motor");
              outputDcState = "off";
              //turn off dc motor

              Serial.println("off speed");
              digitalWrite(IN1, LOW);
              digitalWrite(IN2, LOW);
            } 
            


            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style> body {  background-color: #cadf9e;}");
            client.println("h1 {  color: #595358;  text-align: center;}");
            client.println("H2{  color:red;text-align: center;}");
            client.println(".content{  color: #313628;  font-size: 28px;  text-align: center;}");
            client.println(".button{ background-color: #4CAF50; border: none;color: white;padding: 15px 32px;text-align: center;text-decoration: none;display: inline-block;font-size: 16px;}");
            
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Home Monitor</h1><br><br>");
            
            // Display current state, and ON/OFF buttons for GPIO 5  
            client.println("<div class=\"content\">Lights : " + output5State + " ");
            // If the output5State is off, it displays the ON button       
            if (output5State=="off") {
              client.println("<a href=\"/5/on\"><button class=\"button\"> Turn ON</button></a>");
            } else {
              client.println("<a href=\"/5/off\"><button class=\"button button2\"> Turn OFF</button></a>");
            } 
             // DC current state, and ON/OFF buttons for Dc motor
            client.println("<br><br>");
              client.println("<div class=\"content\"> Fan : " + outputDcState +  " ");
            // If the outputDcState is off, it displays the ON button       
            if (outputDcState=="off") {
              client.println("<a href=\"/4/on\"><button class=\"button\"> Turn ON</button></a>");
            } else {
              client.println("<a href=\"/4/off\"><button class=\"button button2\"> Turn OFF</button></a>");
            } 

            client.println("<br><br>");
            client.println("Parking Status : "+outputIR1State+" ");
            client.println("<button type= \"reset\" class=\"button\" onClick=\"window.location.reload();\"> Check </button>");
            client.println("<br><br>");
            client.println("Door Status :"+outputIR2State+" ");
            client.println("<button type= \"reset\" class=\"button\" onClick=\"window.location.reload();\"> Check </button>");
            
            client.println("</div></body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
  val1 = digitalRead(D6); // IR Sensor output pin connected to D6 
  val2 = digitalRead(D1); // IR Sensor output pin connected to D1 
 
      // for timer  
  if(val1 == 1 )  
  {  
   digitalWrite(D7,LOW); // LED ON
   outputIR1State="Empty";  
  }  
  else  
  {  
   digitalWrite(D7,HIGH); // LED OFF  
   outputIR1State="Occupied";
  }  
  if(val2 == 1 )  
  {  
   digitalWrite(D8,LOW); // LED ON  
    outputIR2State="open";
  }  
  else  
  {  
   digitalWrite(D8,HIGH); // LED OFF  
   outputIR2State="closed";
  }  



}
/*
void setDirection() {
 Serial.println("change speed");
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  
  for (int i = 0; i < 256; i++) {
    analogWrite(ENA, i);
    Serial.println(i);
    delay(20);
  }
  
  for (int i = 255; i >= 0; --i) {
    analogWrite(ENA, i);
    Serial.println(i);
    delay(20);
  }
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  
}

void setDirection1() {
 Serial.println("off speed");
   digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  
}
*/
