#include <PlainProtocol.h>//head files of broadcast thread to ESP8266
#include "DHT.h"//head files of DHT11 sensor
#include <TimeLib.h>
#include <OneWire.h> //head files of DS18B20 sensor

typedef void(*functionType)(void);

DHT dht; //declare DHT11 sensor
OneWire ds(8); //declare DS18B20 Signal pin on digital 8 

class SimpleTimer{ //declare class simple timer
public:
  SimpleTimer();
  void setInterval(long value,functionType cb);
  void run();
private:
  long delayTime;
  unsigned long prevMillis;
  functionType callback;
};

SimpleTimer::SimpleTimer(){
  delayTime = 0;
  prevMillis = 0;
}

void SimpleTimer::setInterval(long value,functionType cb){
  delayTime = value;
  callback = cb;
  prevMillis = millis();
}

void SimpleTimer::run(){
  if((millis() - this->prevMillis) > delayTime){
    this->prevMillis = millis();
    callback();
  }
}


//declare needed class
PlainProtocol puloadThread(Serial1); //VSCode noticed not declared because Leonardo have Serial1 but VSCode don't know
SimpleTimer uploadTimer;
SimpleTimer checkTem;

//declare data variable
float humidity; //variable to save environment humidity data from DHT11 sensor
float temperature;//variable to save environment temperature from DHT11 sensor
float SoilTemp;//variable to save Soil temperature from DS18B20 sensor
int SoilMoisture;//variable to save plant soil moisture from soil sensor
int SoilMoistureThreshold = 50; //declare moisture target variable and initialize to 50 percent by default

//function to control water pump status
void WaterPump(int flag){ //flag 1 for watering, 0 for stop
if(flag==1){
  puloadThread.write("WATERING",255); //tell everyone now is watering
  digitalWrite(7,LOW); 
  digitalWrite(4,LOW);    
  digitalWrite(5,HIGH);
  digitalWrite(6,HIGH);    
}
else{
  puloadThread.write("STOP",0); //tell everyone now is not watering
  digitalWrite(7,LOW); 
  digitalWrite(4,LOW);    
  digitalWrite(5,LOW);
  digitalWrite(6,LOW);   
}
}

float DS18B20getTemp(){
  //returns the temperature from one DS18S20 in DEG Celsius
  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
      //no more sensors on chain, reset search
      ds.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("DS18B20:CRC is not valid!");
      return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("DS18B20:Device is not recognized");
      return -1000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE); // Read Scratchpad

  
  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }
  
  ds.reset_search();
  
  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;
  
  return TemperatureSum;
  
}
//function for upload data to thread and do watering when hit threshold
void uploadDataCB(){
  humidity = dht.getHumidity();//get environment humidity data from DHT11 sensor
  temperature = dht.getTemperature();//get environment temperature from DHT11 sensor
  SoilTemp = DS18B20getTemp(); //get plant soil temperature from DS18B20 sensor
  SoilMoisture = map(analogRead(A2), 0, 1023, 0, 100);;//get plant soil moisture from soil sensor which is analog signal, map to percent
  puloadThread.write("E_HUM",humidity);//broadcast environment humidity data to thread
  puloadThread.write("E_TEM",temperature);//broadcast environment temperature data to thread
  puloadThread.write("SOIL",SoilMoisture);//broadcast soil moisture data to thread
  puloadThread.write("STEM",SoilTemp);//broadcast soil temperature data to thread
  if(SoilMoisture < SoilMoistureThreshold){   //soil moisture now lower than threshold
    WaterPump(1); //do watering
  }
  else{ //soil moisture now higher than threshold
    WaterPump(0); // stop watering
  }
}

//run only once when boot up
void setup() {
  pinMode(4,OUTPUT);//initialize waterpump control
  pinMode(5,OUTPUT);
  pinMode(6,OUTPUT);
  pinMode(7,OUTPUT);
  Serial.begin(115200);
  puloadThread.begin(115200);
  dht.setup(9);//init DHT11 Signal pin on digital 9
  uploadTimer.setInterval(2000, uploadDataCB);
}

//loop program
void loop() {
  if (puloadThread.available()) {
    if (puloadThread.equals("SET")) { //if user change the moisture target from IoT according to ESP8266,set to threshold variable
      SoilMoistureThreshold = puloadThread.read();
      Serial.println(SoilMoistureThreshold);
      Serial.println("IoT:soil moisture threshold changed");
    }
    else if (puloadThread.equals("MAN")){ //if user trigger manually watering, open water pump
      WaterPump(1);
    }
    else if (puloadThread.equals("RLSE")){ //if user trigger stop manual mode, turn off water pump
      WaterPump(0);
    }
  }
  uploadTimer.run();
}