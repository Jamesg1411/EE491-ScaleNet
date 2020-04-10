#include "Energia.h"
#ifndef __CC3200R1M1RGC__
#include <SPI.h>
#endif
#include <WiFi.h>
#include <Arduino.h>
#include "HX711.h" 
#include "mainOnWifi.h"

#define calibrationFactor -10150//-9950
// Define structures and classes
HX711 scale;
char wifi_name[] = "ScaleNet";
char wifi_password[] = "Launchpad";
WiFiServer myServer(80);

//declare global variables
uint8_t oldCountClients = 0;
uint8_t countClients = 0;

int ARRAY_SIZE = 10;
float CONFIG_TEMP = 18.33;

static float FLbias = 0;
static float FRbias = 0;
static float BLbias = 0;
static float BRbias = 0;
static float frontLeftData = 0;
static float frontRightData = 0;
static float backLeftData = 0;
static float backRightData = 0;
static float frontDiff = 0;
static float backDiff = 0;
static float leftDiff = 0;
static float rightDiff = 0;
static float LRDiff = 0;
static float FBDiff = 0;
static float total = 0;



/*
 **************************************************************************************************************************
 * Name:          getSlaveData
 * Description:   This function is repsonsible for fetching slave weight data
 * Arguments:     4 floats, one for each temperature corrected weight, passed by reference
 **************************************************************************************************************************
 */
void getSlaveData( float *frontLeftData, float *frontRightData, float *backLeftData, float *backRightData)
{
    //DAN: Add ur code that interfaces with the Bluetooth chips over SPI or I2C (Don't care)
    //*frontLeftData = 160.07;
    //*frontRightData = 163.07;
    //*backLeftData = 155.09;
    //*backRightData = 161.45;
}

/*
 **************************************************************************************************************************
 * Name:          getAverage (TEMPORARY)
 * Description:   Finds the average of the input array, (uses constant ARRAY_SIZE to find end of array
 * Arguments:     a pointer to the data array
 **************************************************************************************************************************
 */
float getAverage(float *data)
{
      float dataAverage = 0;
      for(int i= 0; i < ARRAY_SIZE; i++)
      {
        dataAverage = dataAverage + data[i];
      }
      dataAverage = dataAverage / ARRAY_SIZE;
      return dataAverage;
}

/*
 **************************************************************************************************************************
 * Name:          tempCorrect (TEMPORARY)
 * Description:   correcting the weight value accoridng to temperature
 * Arguments:     dataAverage is the average of the data array and temp is the temperature reading of the thermistor
 **************************************************************************************************************************
 */
float tempCorrect(float dataAverage, float temp)
{
    float correctedWeight = dataAverage - (((temp - CONFIG_TEMP)/10)*2);
    return correctedWeight;
}

/*
 **************************************************************************************************************************
 * Name:          setup
 * Description:   responisble for wifi board and HX711 bring-up
 * Arguments:     N/A
 **************************************************************************************************************************
 */
void setup()
{
    Serial.begin(115200);
    delay(500);
    
    Serial.println("*** LaunchPad CC3200 WiFi Web-Server in AP Mode");
    Serial.print("Starting AP... ");
    WiFi.beginNetwork(wifi_name, wifi_password);
    while (WiFi.localIP() == INADDR_NONE)
    {
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
    
    IPAddress ip = WiFi.localIP();
    Serial.print("Webserver IP address = ");
    Serial.println(ip);
    
    Serial.print("Web-server port = ");
    myServer.begin();                           // start the web server on port 80
    Serial.println("80");
    Serial.println();

    scale.begin(10,9);
    scale.set_scale(calibrationFactor);
    scale.tare();
    delay(30);
}

/*
 **************************************************************************************************************************
 * Name:          loop
 * Description:   The main body of the code, continuously looping
 * Arguments:     N/A
 **************************************************************************************************************************
 */
void loop()
{
    int iter = 0;

    float data[ARRAY_SIZE];
    float currentValue = 0;
    float temp = 18.33;
    
    countClients = WiFi.getTotalDevices();
    if (countClients != oldCountClients)
    {

        if (countClients > oldCountClients)
        {  // Client connect
            //            digitalWrite(RED_LED, !digitalRead(RED_LED));
            //Serial.println("Client connected to AP");

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
    {
        char buffer[150] = {0};
        int8_t i = 0;
        while (myClient.connected())
        {
            if (myClient.available())
            {
                char c = myClient.read();
                //Serial.write(c);
                if (c == '\n') {
                    if (strlen(buffer) == 0)
                    {
                        myClient.println("HTTP/1.1 200 OK");
                        myClient.println("Content-type:text/html");
                        myClient.println();
                        
                        myClient.println(HTMLchunk1);
                        myClient.println(backLeftData);
                        myClient.println(HTMLchunk2);
                        myClient.println(backDiff);
                        myClient.println(HTMLchunk3);
                        myClient.println(backRightData);
                        myClient.println(HTMLchunk4);
                        myClient.println(leftDiff);
                        myClient.println(HTMLchunk5);
                        myClient.println(rightDiff);
                        myClient.println(HTMLchunk6);
                        myClient.println(frontLeftData);
                        myClient.println(HTMLchunk7);
                        myClient.println(frontDiff);
                        myClient.println(HTMLchunk8);
                        myClient.println(frontRightData);
                        myClient.println(HTMLchunk9);
                        myClient.println(LRDiff);
                        myClient.println(HTMLchunk10);
                        myClient.println(total);
                        myClient.println(HTMLchunk11);
                        myClient.println(FBDiff);
                        myClient.println(HTMLchunk12);
                        
                        myClient.println();
                        break;
                    }
                    else
                    {
                        memset(buffer, 0, 150);
                        i = 0;
                    }
                }
                else if (c != '\r')
                {
                    buffer[i++] = c;
                }
                
                String text = buffer;
                if (text.endsWith("GET /ZO") || text.endsWith("GET /UP"))
                {
//beginning of TEMP code (remove once BT mod is integrated)
                        digitalWrite(18, HIGH);
                        for(int i=0; i < ARRAY_SIZE; i++)
                        {
                          currentValue = scale.get_units();
                          data[i] = currentValue;
                        }
                        digitalWrite(18, LOW);
                        float dataAverage = getAverage(data);
                        frontLeftData = tempCorrect(dataAverage, temp);
//end of TEMP code
                        getSlaveData(&frontLeftData, &frontRightData, &backLeftData, &backRightData);
                        if(text.endsWith("GET /ZO"))
                        {
                            FLbias = frontLeftData;
                            FRbias = frontRightData;
                            BLbias = backLeftData;
                            BRbias = backRightData;
                        
                        }
                        Serial.print("TestData:  ");
                        Serial.print(FLbias);
                        frontLeftData -= FLbias;
                        frontRightData -= FRbias;
                        backLeftData -= BLbias;
                        backRightData -= BRbias;
                        Serial.print("FrontLeft:  ");
                        Serial.println(frontLeftData);
                        Serial.print("FrontRight: ");
                        Serial.println(frontRightData);
                        Serial.print("BackLeft:   ");
                        Serial.println(backLeftData);
                        Serial.print("BackRight:  ");
                        Serial.println(backRightData);  

                        frontDiff = frontLeftData - frontRightData;
                        leftDiff = frontLeftData - backLeftData;
                        rightDiff = frontRightData - backRightData;
                        backDiff = backLeftData - backRightData;
                        LRDiff = frontDiff + backDiff;
                        FBDiff = leftDiff + rightDiff;
                        total = frontLeftData + frontRightData + backLeftData + backRightData;
                }
            }
        }
        myClient.stop();
    }
}
