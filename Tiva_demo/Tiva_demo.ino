#include "HX711.h"

#define calibration_factor -10390//-9950


HX711 scale;
const int arraySize = 10;

void setup() {
  Serial.begin(9600);
  Serial.println("ScaleNet Load Cell Demo");
 
  scale.begin(38, 19);
  scale.set_scale(calibration_factor);
  scale.tare();
  Serial.println("Begin:");

  delay(20);
}

void loop() {
  int iter = 0;
  float data[arraySize];
  float dataAverage = 0; float currentValue = 0;
  char activeStatus = '0';

  while(1)
  {
    if(Serial.available())
    {
      activeStatus = Serial.read();
    }
    if(activeStatus == '0')
    {
      currentValue = scale.get_units();
      data[iter] = currentValue;
      //Serial.print(data[iter]);
      //Serial.println();
      if(iter >= arraySize)
      {
        iter = 0; 
      }
      else
      {
        iter += 1;
      }
    }
    if(activeStatus == '1')
    {
      dataAverage = 0;
      for(int i= 0; i < arraySize; i++)
      {
        dataAverage = dataAverage + data[i];
        Serial.print("Test Data ");
        Serial.print(data[i]);
        Serial.print(" lbs");
        Serial.println();
      }
      dataAverage = dataAverage / arraySize;
      Serial.print("Average: ");
      Serial.print(dataAverage);
      Serial.print(" lbs");
      Serial.println();
    }
    activeStatus = '0';
  }
}
