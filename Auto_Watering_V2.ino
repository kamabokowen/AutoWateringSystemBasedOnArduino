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
//SimpleTimer checkTem;

//declare data variable
float humidity; //variable to save environment humidity data from DHT11 sensor
float temperature;//variable to save environment temperature from DHT11 sensor
float SoilTemp;//variable to save Soil temperature from DS18B20 sensor
int SoilMoisture;//variable to save plant soil moisture from soil sensor
int SoilMoistureThreshold = 50; //declare moisture target variable and initialize to 50 percent by default
short ManualModeFlag = 0;
short Pump_Status_Flag = 0;

void WaterPump(int flag){ //flag 1 for watering, 0 for stop
  Pump_Status_Flag = flag;//用来调试
  if(flag==1){
    puloadThread.write("PUMP",255); //tell everyone now is watering
    puloadThread.write("PUMP",255);
    digitalWrite(7,LOW); 
    digitalWrite(4,LOW);    
    digitalWrite(5,HIGH);
    digitalWrite(6,HIGH);    
    Serial.println("[MCU/PUMP] POWER ON");
  }
  else{
    puloadThread.write("PUMP",0); //tell everyone now is not watering
    puloadThread.write("PUMP",0);
    digitalWrite(7,LOW); 
    digitalWrite(4,LOW);    
    digitalWrite(5,LOW);
    digitalWrite(6,LOW);   
    Serial.println("[MCU/PUMP] POWER OFF");
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

void uploadDataCB(){
  humidity = dht.getHumidity();//get environment humidity data from DHT11 sensor
  temperature = dht.getTemperature();//get environment temperature from DHT11 sensor
  Serial.println("[sensor]get DHT11 data(humi/temp):"+(String)humidity+"/"+(String)temperature);
  SoilTemp = DS18B20getTemp(); //get plant soil temperature from DS18B20 sensor
  Serial.println("[sensor]get DS18B20 tem data:"+(String)SoilTemp);
  SoilMoisture = map(analogRead(A2), 0, 1023, 0, 100);;//get plant soil moisture from soil sensor which is analog signal, map to percent
  Serial.println("[sensor]get plant moi data:"+(String)SoilMoisture);

  puloadThread.write("SENSOR",(int)humidity,(int)temperature,(int)SoilMoisture,(int)SoilTemp,Pump_Status_Flag);
  Serial.println("[Serial1]upload to serial");
}

void ESP8266_Callback(){
  if (puloadThread.available()) {
    if (puloadThread.equals("SET")) { //if user change the moisture target from IoT according to ESP8266,set to threshold variable
      SoilMoistureThreshold = puloadThread.read();
      Serial.println("[ESP8266]Moisture Target changed to"+(String)SoilMoistureThreshold);
    }
    if (puloadThread.equals("MANUAL")){ //if user trigger manually watering, open water pump
      if(puloadThread.read()==255){
        ManualModeFlag = 1;
        Serial.println("[ESP8266]Pump Control MANUAL mode");
      }
      else{
        ManualModeFlag = 0;
        Serial.println("[ESP8266]Pump Control AUTO mode");
      }
    }
  }
}

/*
void ESP8266_DEBUG(){
  int ESP_INFO_FLAG,esp_data_1,esp_data_2;
  if (puloadThread.available() && puloadThread.equals("ESP")){
    ESP_INFO_FLAG = puloadThread.read();
    esp_data_1 = puloadThread.read();
    esp_data_2 = puloadThread.read();
    switch (ESP_INFO_FLAG){
    case 0: //8266心跳时触发，data格式土壤温湿度
      Serial.println("[ESP8266]Hearbeat Sended,PlantTem:"+(String)esp_data_1+",Moi"+(String)esp_data_2);
      if(esp_data_1 != SoilTemp || esp_data_2 != SoilMoisture){
        Serial.println("[ERROR]heartbeat data not correct,reupload data");
        puloadThread.write("SENSOR",(int)humidity,(int)temperature,(int)SoilMoisture,(int)SoilTemp);
      }
      break;
    
    case 1://接收上位机data时触发，data格式土壤温湿度
      Serial.println("[ESP8266]receive sensor data,plant temp:"+(String)esp_data_1+",moi:"+(String)esp_data_2);
      if(esp_data_1 != SoilTemp || esp_data_2 != SoilMoisture){
        Serial.println("[ERROR]receive sensor data not correct,reupload data!");
        puloadThread.write("SENSOR",(int)humidity,(int)temperature,(int)SoilMoisture,(int)SoilTemp);
      }
      break;

    case 2://主动上传云服务时触发，data格式：环境温湿度，土壤温湿度
      Serial.println("[ESP8266]refresh IoT Data,Plant temp:"+(String)esp_data_1+",moi:"+(String)esp_data_2);
      if(esp_data_1 != SoilTemp || esp_data_2 != SoilMoisture){
        Serial.println("[ERROR]refresh IoT Data not correct,reupload data!");
        puloadThread.write("SENSOR",(int)humidity,(int)temperature,(int)SoilMoisture,(int)SoilTemp);
      }
      break;

    case 3://云服务手动浇水回调时触发，data格式 水泵模式，水泵状态
      Serial.print("[ESP8266/Request]Pump Mode changed:");
      if(esp_data_1==0){
        Serial.print("AUTO,Status:");
        if(esp_data_2==0){
          Serial.println("Off");
        }
        else if(esp_data_2==1){
          Serial.println("On");
        }
      }
      else if(esp_data_1==1){
        Serial.print("Manual,Status:");
        if(esp_data_2==0){
          Serial.println("Off");
          Serial.println("[ERROR]Impossible Status!");
        }
        else if(esp_data_2==1){
          Serial.println("On");
        }
      }
      if(esp_data_1 != ManualModeFlag){
        Serial.println("[ERROR]Pump Mode not correct!");
        ManualModeFlag = esp_data_1;
      }
      if(esp_data_2 != Pump_Status_Flag){
        Serial.println("[ERROR]Pump Status not correct!Reuploading...");
        if(Pump_Status_Flag == 0){
          puloadThread.write("PUMP",0);
        }
        else{
          puloadThread.write("PUMP",255);

        }
      }
      break;

    case 4://云服务调整湿度目标回调时触发，data格式：湿度目标，水泵状态
      Serial.println("[ESP8266/request]Moisture Target changed:"+(String)esp_data_1+" Pump Status:"+(String)esp_data_2);
      if(esp_data_1 != SoilMoistureThreshold ){
        Serial.println("[ERROR]MCU missed Moisture Target request,refreshing");
        Serial.print("[MCU]Moisture Target changed from "+SoilMoistureThreshold);
        SoilMoistureThreshold = esp_data_1;
        Serial.print("to"+SoilMoistureThreshold);
        }
        if(esp_data_2 != Pump_Status_Flag){
          Serial.println("[ERROR]Pump Status not correct,refreshing");
          if(Pump_Status_Flag == 0){
            puloadThread.write("PUMP",0);

          }
          else{
            puloadThread.write("PUMP",255);

          }
        }
      break;

    case 5://水泵状态按钮回调时触发，data格式：水泵模式，水泵状态
      Serial.println("[ESP8266/request]Pump Status Callback,Mode:"+(String)esp_data_1+",Status:"+(String)esp_data_2);
      if(esp_data_1 != ManualModeFlag){
        Serial.println("[ERROR]Manual Mode not correct!");
        ManualModeFlag = esp_data_1;
      }
      if(esp_data_2 != Pump_Status_Flag){
        Serial.println("[ERROR]Status Callback pump status error!");
        if(Pump_Status_Flag == 0){
          puloadThread.write("PUMP",0);

        }
        else{
          puloadThread.write("PUMP",255);

        }
      }
      break;
    }
  }
}
*/

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

void loop() {
  if(ManualModeFlag == 1){
    WaterPump(1);
    Serial.println("[MCU]Manual Mode,Pump on");
  }
  else{
    if(SoilMoisture < SoilMoistureThreshold){   //soil moisture now lower than threshold
      WaterPump(1); //do watering
      Serial.println("[MCU]AUTO Mode,Pump ON");
    }
    else{ //soil moisture now higher than threshold
      WaterPump(0); // stop watering
      Serial.println("[MCU]AUTO Mode,Pump OFF");
    }
  }
  ESP8266_Callback();
  uploadTimer.run();
}
