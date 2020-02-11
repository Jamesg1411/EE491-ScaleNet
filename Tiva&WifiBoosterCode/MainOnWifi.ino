#include "Energia.h"

#ifndef __CC3200R1M1RGC__
#include <SPI.h>                // Do not include SPI for CC3200 LaunchPad
#endif
#include <WiFi.h>
#include "HX711.h" 

// Define structures and classes
HX711 scale;
#define calibration_factor -10390//-9950
char wifi_name[] = "energia";
char wifi_password[] = "launchpad";
WiFiServer myServer(80);
uint8_t oldCountClients = 0;
uint8_t countClients = 0;
int arraySize = 10;
float configTemp = 18.33;
static float FLbias = 0;
static float FRbias = 0;
static float BLbias = 0;
static float BRbias = 0;

//added functions
void getSlaveData(float *FrontRightData, float *BackLeftData, float *BackRightData)
{
    //SPI interaction to grab the 3 averaged & temperature corrected weight values from the pico-net
    *FrontRightData = 163.07;
    *BackLeftData = 155.09;
    *BackRightData = 161.45;
}

float getAverage(float *data)
{
      float dataAverage = 0;
      for(int i= 0; i < arraySize; i++)
      {
        dataAverage = dataAverage + data[i];
      }
      dataAverage = dataAverage / arraySize;
      return dataAverage;
}

float tempCorrect(float dataAverage, float temp)
{
    float correctedWeight = dataAverage - (((temp - configTemp)/10)*2);
    return correctedWeight;
}


// Add setup code
void setup()
{
    Serial.begin(115200);
    delay(500);
    
    Serial.println("*** LaunchPad CC3200 WiFi Web-Server in AP Mode");
    
    // Start WiFi and create a network with wifi_name as the network name
    // with wifi_password as the password.
    Serial.print("Starting AP... ");
    WiFi.beginNetwork(wifi_name, wifi_password);
    while (WiFi.localIP() == INADDR_NONE)
    {
        // print dots while we wait for the AP config to complete
        Serial.print('.');
        delay(300);
    }
    Serial.println("DONE");
    
    Serial.print("LAN name = ");
    Serial.println(wifi_name);
    Serial.print("WPA password = ");
    Serial.println(wifi_password);
    
    
    pinMode(RED_LED, OUTPUT);      // set the LED pin mode
    digitalWrite(RED_LED, LOW);
    pinMode(GREEN_LED, OUTPUT);      // set the LED pin mode
    digitalWrite(GREEN_LED, LOW);
    //pinMode(YELLOW_LED, OUTPUT);      // set the LED pin mode
   // digitalWrite(YELLOW_LED, LOW);
    
    IPAddress ip = WiFi.localIP();
    Serial.print("Webserver IP address = ");
    Serial.println(ip);
    
    Serial.print("Web-server port = ");
    myServer.begin();                           // start the web server on port 80
    Serial.println("80");
    Serial.println();

    scale.begin(10,9);
    //scale.begin(14,7);
    scale.set_scale(calibration_factor);
    scale.tare();
    delay(30);
    //Serial.println("Begin Running");
}

// Add loop code
void loop()
{
    //local vars
    int iter = 0;
    float FrontLeftData = 0;
    float FrontRightData = 0;
    float BackLeftData = 0;
    float BackRightData = 0;
//    float FrontDiff = 0;
//    float BackDiff = 0;
//    float LeftDiff = 0;
//    float RightDiff = 0;
    //hardcoding the array values to provide sample data for tesing without strain gauges
    float data[arraySize]; // = {160.0, 161.1, 162.2, 163.3, 164.4, 165.5, 166.6, 167.7, 168.8, 169.9};
    float currentValue = 0;
    float temp = 18.33;
    
    countClients = WiFi.getTotalDevices();
    // Did a client connect/disconnect since the last time we checked?
    if (countClients != oldCountClients)
    {

        if (countClients > oldCountClients)
        {  // Client connect
            //            digitalWrite(RED_LED, !digitalRead(RED_LED));
            //Serial.println("Client connected to AP");
/*            for (uint8_t k = 0; k < countClients; k++)
            {
                Serial.print("Client #");
                Serial.print(k);
                Serial.print(" at IP address = ");
                Serial.print(WiFi.deviceIpAddress(k));
                Serial.print(", MAC = ");
                Serial.println(WiFi.deviceMacAddress(k));
                Serial.println("CC3200 in AP mode only accepts one client.");
            }*/
        }
        else
        {  // Client disconnect
            //            digitalWrite(RED_LED, !digitalRead(RED_LED));
            //Serial.println("Client disconnected from AP.");
            //Serial.println();
        }
        oldCountClients = countClients;
    }
    
    WiFiClient myClient = myServer.available();
    if (myClient)
    {                             // if you get a client,
        //Serial.println(". Client connected to server");           // print a message out the serial port
        char buffer[150] = {0};                 // make a buffer to hold incoming data
        int8_t i = 0;
        while (myClient.connected())
        {            // loop while the client's connected
          //end of added code block 1
            if (myClient.available())
            {             // if there's bytes to read from the client,
              //Serial.println("client available");
                char c = myClient.read();             // read a byte, then
                //Serial.write(c);                    // print it out the serial monitor
                if (c == '\n') {                    // if the byte is a newline character
                    
                    // if the current line is blank, you got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (strlen(buffer) == 0)
                    {
                        // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                        // and a content-type so the client knows what's coming, then a blank line:
                        myClient.println("HTTP/1.1 200 OK");
                        myClient.println("Content-type:text/html");
                        myClient.println();
                        
                        // the content of the HTTP response follows the header:
                        myClient.println("<html><head><title>Energia CC3200 WiFi Web-Server in AP Mode</title></head><body align=center>");
                        myClient.println("<h1 align=center><font color=\"red\">LaunchPad CC3200 WiFi Web-Server in AP Mode</font></h1>");
                        myClient.print("Red LED <button onclick=\"location.href='/RH'\">HIGH</button>");
                        myClient.println(" <button onclick=\"location.href='/RL'\">LOW</button><br>");
                        myClient.print("Green LED <button onclick=\"location.href='/GH'\">HIGH</button>");
                        myClient.println(" <button onclick=\"location.href='/GL'\">LOW</button><br>");
                        myClient.print("Yellow LED <button onclick=\"location.href='/YH'\">HIGH</button>");
                        myClient.println(" <button onclick=\"location.href='/YL'\">LOW</button><br>");
                        
                        // The HTTP response ends with another blank line:
                        myClient.println();
                        // break out of the while loop:
                        break;
                    }
                    else
                    {      // if you got a newline, then clear the buffer:
                        memset(buffer, 0, 150);
                        i = 0;
                    }
                }
                else if (c != '\r')
                {    // if you got anything else but a carriage return character,
                    buffer[i++] = c;      // add it to the end of the currentLine
                }
                
                String text = buffer;
                // Check to see if the client request was "GET /H" or "GET /L":
                if (text.endsWith("GET /RH") || text.endsWith("GET /RL")) //if request
                {
                            //added code block 1
                        digitalWrite(18, HIGH);
                        for(int i=0; i < arraySize; i++)
                        {
                          currentValue = scale.get_units();
                          data[i] = currentValue;
                        }
                        digitalWrite(18, LOW);
                        float dataAverage = getAverage(data);
                        FrontLeftData = tempCorrect(dataAverage, temp);
                        getSlaveData(&FrontRightData, &BackLeftData, &BackRightData);
                        if(text.endsWith("GET /RL")) //configuration TODO add configuration pin
                        {
                            FLbias = FrontLeftData;
                            FRbias = FrontRightData;
                            BLbias = BackLeftData;
                            BRbias = BackRightData;
                        }
                        Serial.print("FrontLeft:  ");
                        Serial.println(FrontLeftData - FLbias);
                        Serial.print("FrontRight: ");
                        Serial.println(FrontRightData - FRbias);
                        Serial.print("BackLeft:   ");
                        Serial.println(BackLeftData - BLbias);
                        Serial.print("BackRight:  ");
                        Serial.println(BackRightData - BRbias);   
                         //do differentials calculation here                
                }
            }
        }
        
        // close the connection:
        myClient.stop();
    }
}
