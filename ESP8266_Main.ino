#define BLINKER_WIFI
#define BLINKER_MIOT_SENSOR

#include <Blinker.h> //Blinker IoT Platform SDK header
#include <ESP8266WiFi.h>
#include <PlainProtocol.h> //puloadThread header, easier way to broadcast on Serial
#include <ESP8266HTTPClient.h> // SDK to use ESP8266 as HTTP Client header

char auth[] = "32f4cf4c8549"; // Blinker IoT Platform Secret Key
char ssid[] = "WiFi SSID"; //Wi-Fi SSID
char pswd[] = "12345678"; //Wi-Fi Password
String sc_key = "SCT80024Tl1jEYoNAevjbax2EdLIIexDT"; //ServerChan Secret Key

//Declare sensor data var from IPC according to Serial
float Envir_Temp = 0;
float Envir_Humi = 0;
int Plant_Mois = 0;
float Plant_Temp = 0;

PlainProtocol puloadThread(Serial); //Declare LoadThread Class

//Declare IoT Platform Virtual Widget 
BlinkerNumber BlinkEnvirNumberTemp("num-dz5"); //IoT environment temperature number
BlinkerNumber BlinkEnvirNumberHumi("num-4nf"); //IoT environment humidity number
BlinkerNumber BlinkPlantNumberTemp("num-wlt"); //IoT Plant Temperature number
BlinkerNumber BlinkPlantNumberMois("num-4s7"); //IoT Plant Moisture number
BlinkerSlider BlinkPlantSliderTarget("ran-d5i"); //IoT Plant Moisture slider
BlinkerButton BlinkStatusBtn("btn-vi4"); //IoT Water Pump states button
BlinkerButton BlinkManualBtn("btn-q1w"); //IoT Manually watering button

// Callback function for Xiaomi Mijia and Xiaoai assistant
void MI_IoT_Query(int32_t queryCode)
{
    Blinker.log("MIOT Query Received");
    switch (queryCode)
    {
        case BLINKER_CMD_QUERY_HUMI_NUMBER :
            Blinker.log("MIOT Humi requested");
            BlinkerMIOT.humi(Envir_Humi);
            BlinkerMIOT.print();
            break;
        case BLINKER_CMD_QUERY_TEMP_NUMBER :
            Blinker.log("MIOT Temp requested");
            BlinkerMIOT.temp(Envir_Temp);
            BlinkerMIOT.print();
            break;
        default :
            BlinkerMIOT.temp(Envir_Temp);
            BlinkerMIOT.humi(Envir_Humi);
            BlinkerMIOT.print();
            break;
    }
}

//Blinker IoT Platform Slider callback function, this slider is to change Soil Moisture Threshold
void BlinkPlantSliderTarget_callback(int32_t value){
  int Moisture_target = value;
  puloadThread.write("SET",Moisture_target);
  Blinker.log("Moisture Target Value set by slider:"+ (String)Moisture_target);
}

//Blinker IoT Platform Status Button callback function,which is no need to do anything when callback,
//because this button is to display water pump states
void BlinkStatusBtn_callback(const String & state){
  Blinker.log("BlinkStatusBtn callback received");
}

//Blinker Manual Control Button, Water Pump manually turn on/off when user press this button on App
void BlinkManualBtn_callback(const String & state){
  if(state == "off"){
    puloadThread.write("RLSE");
    BlinkManualBtn.color("#808080");
    BlinkManualBtn.print("off");
    Blinker.log("BlinkManualBtn callback received, water pump manual off");
  }
  if(state == "on"){
    puloadThread.write("MAN");
    BlinkManualBtn.color("#7CFC00");
    BlinkManualBtn.print("on");
    Blinker.log("BlinkManualBtn callback received, water pump manual on");
  }
  
}

//Send Heartbeat data btw update sensor data to IoT, 1 time per 30-60 secs
void blinker_heartbeat(){ 
  BlinkEnvirNumberTemp.print(Envir_Temp);
  BlinkEnvirNumberHumi.print(Envir_Humi);
  BlinkPlantNumberTemp.print(Plant_Temp);
  BlinkPlantNumberMois.print(Plant_Mois);
  Blinker.log("heartbeat data uploaded");
}

//Server-Chan WeChat Notification Function
void sc_wechat(String message){ 
  HTTPClient http;
  http.begin("https://sctapi.ftqq.com/"+ sc_key + ".send?text=Auto+Watering+System&desp=" + message);
  int httpCode = http.GET();
  http.end();
  Blinker.log("WeChat Message Sended");
}

int wechat_flag_water = 0; //Flag var to Avoid WeChat Notification Repetition
int wechat_flag_stop = 0;

//Function to Get Sensor data and Water Pump status from IPC
void GET_IPC_DATA(){
  if(puloadThread.available()){
    if(puloadThread.equals("E_TEM")){ //Get Environment Temperature
      Envir_Temp = puloadThread.read();
      //BlinkEnvirNumberTemp.print(Envir_Temp);
    }
    else if(puloadThread.equals("E_HUM")){ //Get Environment Humidity
      Envir_Humi = puloadThread.read();
      //BlinkEnvirNumberHumi.print(Envir_Humi);
    }
    else if(puloadThread.equals("SOIL")){ //Get Plant Soil Moisture
      Plant_Mois = puloadThread.read();
      //BlinkPlantNumberMois.print(Plant_Mois);
    }
    else if(puloadThread.equals("STEM")){ //Get Plant Soil Temperature
      Plant_Temp = puloadThread.read();
      //BlinkPlantNumberTemp.print(Plant_Temp);
    }
    else if(puloadThread.equals("WATERING")){ // Receive Broadcast that Water Pump is watering
      BlinkStatusBtn.color("#66CCFF"); //turn the status button to blue
      BlinkStatusBtn.print("on"); //turn the button to 'on'
      wechat_flag_stop = 0;
      if(wechat_flag_water ==0){
        sc_wechat("Watering,Plant+Moisture+now+" + (String)Plant_Mois);
        Blinker.log("Auto Watering triggered");
        wechat_flag_water++;
      }
    }
    else if(puloadThread.equals("STOP")){ // Receive Broadcast that Water Pump is standby
      BlinkStatusBtn.color("#808080"); //turn the status button to gray
      BlinkStatusBtn.print("off"); //turn the button to 'off'
      wechat_flag_water = 0;
      if(wechat_flag_stop == 0){
        sc_wechat("Done,Plant+Moisture+now+" + (String)Plant_Mois);
        Blinker.log("Auto Watering stopped");
        wechat_flag_stop++;
      }
    }
  }
  //Blinker.log("IPC Data Updated");
}

void setup() {
  puloadThread.begin(115200); //Serial Broadcast start with 115200
  Blinker.begin(auth, ssid, pswd);  //start to connect wifi and Blinker IoT Platform
  BlinkStatusBtn.attach(BlinkStatusBtn_callback); //register callback function of BlinkStatusBtn
  BlinkManualBtn.attach(BlinkManualBtn_callback); //register callback function of BlinkManualBtn
  Blinker.attachHeartbeat(blinker_heartbeat); //attach customize heartbeat function
  BlinkPlantSliderTarget.attach(BlinkPlantSliderTarget_callback); //register callback function of BlinkPlantSliderTarget
  BlinkerMIOT.attachQuery(MI_IoT_Query); //register callback function of Mi-Mijia
  sc_wechat("Auto+Watering+System+Booted");
  Blinker.log("System Booted");
}

void loop() {
  Blinker.run();
  GET_IPC_DATA();
  //Blinker.delay(1000);
}
