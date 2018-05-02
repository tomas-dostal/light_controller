#include <ESP8266WiFi.h>
#include <Arduino.h>

// This code is used to:
// 1) control your light using HW switch
// 2) control your light using Web page of your local network (through Wi-Fi)
//    NOTE: You need to have some board with ESP8266 to use this feaure, in this case NodeMCU 1.0
// Author: Tomas Dostal 
// tomas.dostal.opava@gmail.com

// This code is available with no warranty. 

// ----------------------  EDIT THIS BEFORE START! ----------------------

const char* ssid = "SSID";  // SSID of the network you want to connenct to
const char* password = "PASSWORD";  // PW of the network you want to connenct to
int button[] = {D0, D1, D2, D3};  // HW buttons
int lights[] = {D5, D6, D7, D8};  // HW lights

// ----------------------  My variables ----------------------
// if(!you.see(why))
//  {
//    stay_the_fuck_away_from_my_code();
//  }

WiFiServer server(80);
IPAddress ip(192, 168, 1, 10);
IPAddress gateway(192, 168, 1, 1); // set gateway to match your network

int now_light[] = {0, 0, 0, 0};
int required_light[] = {128, 128, 128, 128};

  int now_button[] = {LOW, LOW, LOW, LOW};
int last_button[] = {LOW, LOW, LOW, LOW};


// ----------------------  My functions ----------------------

void set_light(int index, int to)
{
   required_light[index] = to;
}
void set_lights(int to)
{
  for(int a = 0; a < sizeof(lights)/ sizeof (int); a++)
  {
    required_light[a] = to;
  }
}
void on_all_btn_changed(int to)
{
  Serial.print("ALL BTN CHANGED ");
  Serial.print("to: ");
  Serial.println(to);  
  if(to == LOW)
    on_all_btn_off() ;
}
void on_all_btn_off()
{
  Serial.println("ALL BTN OFF ");
}
void on_buttonstate_changed(int index, int from, int to)
{
  Serial.println("");
  Serial.print("Button ");
  Serial.print("Chaged from " );
  Serial.print(from);
  Serial.print(" to ");
  Serial.println(to); 
  set_light(index, to*255); 
}

// ----------------------  SETUP ----------------------
void setup() {

  for(int i = 0; i < sizeof(button)/ sizeof (int); i++)
  {
     pinMode(button[i], INPUT);
     now_button[i] = LOW;
     last_button[i] = LOW;
  }
  for(int i = 0; i < sizeof(lights)/ sizeof (int); i++)
  {
     pinMode(lights[i], OUTPUT);
  
  }
  Serial.begin(115200);
  
  WiFi.begin ( ssid, password );
  Serial.print ( "Connecting to wifi: " );
  Serial.print(ssid);
  Serial.print(" with password: ");
  Serial.println(password);
  // Wait for connection

  while ( WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.write(".");
  }

  Serial.println("Connected!");

  // Start the server
  server.begin();
  Serial.println("Server started.");

  // Print the IP address
  Serial.print("Use this URL : ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");}

// ----------------------  LOOP ----------------------

void loop() 
{
  
 
  // Detect buttton changes
  
  //Serial.print("Buttons: ");
  
  bool all_changed = true;

  for(int i = 0; i < sizeof(button)/ sizeof (int); i++)
  {
    now_button[i] = digitalRead(button[i]);
    
    if(all_changed)
        if(last_button[i] == now_button[i])
          all_changed = false;
          
    if(now_button[i] !=last_button[i])
    {
      on_buttonstate_changed(i, last_button[i], now_button[i]);
      
      last_button[i] = now_button[i];
    } 
    //Serial.print(i);
    //Serial.print(": ");
    if (now_button[i] == HIGH) {
      // turn LED on:
      //Serial.print("ON; ");
      //digitalWrite(ledPin, HIGH);
    } else {
      // turn LED off:
      //Serial.print("OFF; ");

      //digitalWrite(ledPin, LOW);
    }
    
  }
  //Serial.println("");

  if(all_changed)
    on_all_btn_changed(now_button[0]) ;
  
  // ----------------------  WWW ----------------------
  
  // Check if a client has connected
  
  WiFiClient client = server.available();
  if (client && client.available() )
  {
    // Wait until the client sends some data
    Serial.println("new client");
    while (!client.available()) {
      delay(1);
    }

    // Read the first line of the request
    String request = client.readStringUntil('\r');
    Serial.println("Request:");
    Serial.println(request);
    client.flush();

    // take a look for each possible value (0-255) of every light
    for(int light_index = 0; light_index < sizeof(lights)/ sizeof (int); light_index++)
    {
      for(int value = 0; value < 256; value++)
        {
          String req = "/light[" +String(light_index) + "]="+ String(value);
        if (request.indexOf(req) != -1) {
          
          Serial.print("lights[");
          Serial.print(light_index);
          Serial.print("]: ");
          Serial.print(value);
          set_light(light_index, value);
        }   
      }
    }
    // for all lights
    for(int value = 0; value < 256; value++)
    {
      String req = "/all_lights="+ String(value);
      if (request.indexOf(req) != -1) {
        
        Serial.print("all_lights: ");
        Serial.print(value);
        
        set_lights (value);
      }
    }

    // ----------------------  send a response ----------------------
     
    // Return the response
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println(""); //  do not forget this one
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");

    client.println("<br><br>");
    client.println("<head>");
    client.println("<link rel='shortcut icon' href='http://example.com/myicon.ico'>");
    client.println("<meta charset='utf-8'>");
    client.println("<title>slider demo</title>");
    client.println("<style type='text/css'><!-- *{color:#FFF; text-align:center; margin:0; padding:0; text-decoration:none;} .menu{ display:block; margin:auto;} .menu ul{list-style-type:none; text-alighn:center;} u{text-align:center; text-color:#FFF; }.menu li{ margin:auto; padding:2% 0;  float:center; width:50%; display:block; font-size:2em; box-shadow: 0px 0px 5px #888888;}.menu ul li { display:block; background:#333;}  --> </style>");


    client.println("<body>");
    //client.println("<link rel='stylesheet' href='//code.jquery.com/ui/1.12.1/themes/smoothness/jquery-ui.css'>  <style>#slider { margin: 10px; }  </style>  <script src='//code.jquery.com/jquery-1.12.4.js'></script>  <script src='//code.jquery.com/ui/1.12.1/jquery-ui.js'></script>");
    //client.println("<div id='slider'></div> <script> $( '#slider' ).slider();  </script>");
    client.println("<h1 style='color:#333; text-shadow: 0px 0px 5px #EEE; font-size:3em; padding:3%; font-weight: normal;'>Ovladač světel</h1>");
    client.println("<div class='menu'>");

    client.println("<ul>");
    
    for(int i = 0; i < sizeof(lights)/ sizeof (int); i++)
    {
      // if an operation pending...
      if(required_light[i] != now_light[i])
      {
        // turning on at the moment
        if(required_light[i] > now_light[i])
          client.println("<a href=\"/light[" + String(i) + "]=0\"><li style='background-color:#ffd500;;color:#000000;'>LIGHT " + String(i) + "-> on. Turn OFF anyway!</li></a><br>"); 
        // turning off at the moment
        else 
          client.println("<a href=\"/light[" + String(i)+"]=255\"><li style='background-color:#ffd500;;color:#000000;'>LIGHT " + String(i) + "-> off. Turn ON anyway!</li></a><br>"); 
      }
      // light value is stable
      else
      {
        // button turn on
        if(now_light[i] == 0)
        {
           client.println("<a href=\'/light["+String(i)+"]=255\'><li>LIGHT " + String(i) + ": Turn ON</li></a><br>"); 
        }
        // button turn off
        else if(now_light[i] == 255)
        {    
          client.print("<a href=\'/light["+String(i)+"]=0\'><li style='background-color:#FF2222'>LIGHT " + String(i) + ": Turn OFF</li></a><br>");
        }
      }
    }
    // horizontal separator
    client.print("<hr>&nbsp;&nbsp;");

     // all lights on 
    client.print("<a href=\'all_lights=255\'><li>TURN ALL LIGHTS ON</li></a><br>");


    // all lights off
    client.print("<a href=\'/all_lights=0\'><li style='background-color:#FF2222;'>TURN ALL LIGHTS OFF </li></a><br>");
   
  }
  
  client.println("</ul>");
  client.println("</div>");
  client.println("</body>");
  client.println("</html>");
  

 // ----------------------  update changes to lights ----------------------
  
  for(int s = 0; s < sizeof(lights)/ sizeof (int); s++) 
  {
    //Serial.print("Want to: ");
    //Serial.print(required_light[s]);
    //Serial.print(" ; Current: ");
    //Serial.println(now_light[s]);
    if(required_light[s] != now_light[s])
    {
      //Serial.print("Changing light ");
      //Serial.print(s);
      //Serial.print("; final value: ");
      //Serial.print(required_light[s]);  

      // increase light level
      if(required_light[s] > now_light[s])
      {
        if(now_light[s] > 253)
          now_light[s] = required_light[s];
        else
          now_light[s] += 2;
        //Serial.print("Analog write light: ");
        //Serial.print(s);
        //Serial.print(" value: ");
        //Serial.println(now_light[s]);
        
        analogWrite(lights[s], now_light[s]);        
      }
      else // decrease light level
      {
        if(now_light[s] < 20)
            now_light[s] -= 1;
        //else if(now_light[s] < 2)
        //  now_light[s] = required_light[s];
        else
          now_light[s] -= 2;
        analogWrite(lights[s], now_light[s]);        
      }
    } 
  }
  delay(50);

}
