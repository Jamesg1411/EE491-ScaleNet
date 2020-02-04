#include "HX711.h"

HX711 scale;
#define calibration_factor -10390//-9950
#define requestPin 25
#define FLpin 26
#define FRpin 27
#define BLpin 28
#define BRpin 29
int arraySize = 10;
float configTemp = 18.33; //65 degrees F convereteed to C
static float FLbias = 0;
static float FRbias = 0;
static float BLbias = 0;
static float BRbias = 0;

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

void setup() {   
    //const int arraySize = 10; 
    Serial.begin(9600);
    pinMode(requestPin, INPUT);
    pinMode(FLpin, OUTPUT);
    pinMode(FRpin, OUTPUT);
    pinMode(BLpin, OUTPUT);
    pinMode(BRpin, OUTPUT);
    Serial.println("ScaleNet Main Function");
    scale.begin(38, 19);
    scale.set_scale(calibration_factor);
    scale.tare();
    Serial.println("Begin:");
    delay(30);
}
 
void loop() {
    // put your main code here, to run repeatedly: 
    int iter = 0;
    float FrontLeftData = 0;
    float FrontRightData = 0;
    float BackLeftData = 0;
    float BackRightData = 0;
    //hardcoding the array values to provide sample data for tesing without strain gauges
    float data[arraySize] = {160.0, 161.1, 162.2, 163.3, 164.4, 165.5, 166.6, 167.7, 168.8, 169.9};
    float currentValue = 0;
    float temp = 18.33;

    char request = '0';
    while(request == '0')
    {
        /*currentValue = scale.get_units();
        data[iter] = currentValue;
        if(iter >= arraySize)
        { 
            iter = 0; 
        }
        else
        {
            iter += 1;
        }*/
        if(Serial.available())
        {
            request = Serial.read();
        }
    }

    if(request == '1' || request == '2')
    {
      float dataAverage = getAverage(data);
      FrontLeftData = tempCorrect(dataAverage, temp);
      getSlaveData(&FrontRightData, &BackLeftData, &BackRightData);
      if(request == '2') //configuration TODO add configuration pin
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
      //digitalWrite(FLpin, FrontLeftData - FLbias);
      //digitalWrite(FRpin, FrontRightData - FRbias);
      //digitalWrite(BLpin, BackLeftData - BLbias);
      //digitalWrite(BRpin, BackRightData - BRbias);
    }
}
