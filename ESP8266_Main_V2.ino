#define BLINKER_WIFI
#define BLINKER_MIOT_SENSOR

#include <Blinker.h> //Blinker IoT Platform SDK header
#include <PlainProtocol.h> //puloadThread header, easier way to broadcast on Serial

char auth[] = ""; // Blinker IoT Platform Secret Key
char ssid[] = "Arduino"; //Wi-Fi SSID
char pswd[] = "666123456"; //Wi-Fi Password
String sc_key = ""; //ServerChan Secret Key

int Envir_Temp = 20;
int Envir_Humi = 20;
int Plant_Mois = 20;
int Plant_Temp = 20;
int Moisture_target = 50;
int PumpStatus = 0;
int ManualMode = 0;

PlainProtocol puloadThread(Serial); //Declare LoadThread Class

//Declare IoT Platform Virtual Widget 
BlinkerNumber BlinkEnvirNumberTemp("num-dz5"); //IoT environment temperature number
BlinkerNumber BlinkEnvirNumberHumi("num-4nf"); //IoT environment humidity number
BlinkerNumber BlinkPlantNumberTemp("num-wlt"); //IoT Plant Temperature number
BlinkerNumber BlinkPlantNumberMois("num-4s7"); //IoT Plant Moisture number
BlinkerSlider BlinkPlantSliderTarget("ran-d5i"); //IoT Plant Moisture slider
BlinkerButton BlinkStatusBtn("btn-vi4"); //IoT Water Pump states button
BlinkerButton BlinkManualBtn("btn-q1w"); //IoT Manually watering button
BlinkerButton ManuallyRefresh("btn-t43");

void ManuallyRefreshCallback(const String & state){
  BlinkEnvirNumberTemp.print(Envir_Temp);
  BlinkEnvirNumberHumi.print(Envir_Humi);
  BlinkPlantNumberTemp.print(Plant_Temp);
  BlinkPlantNumberMois.print(Plant_Mois);

  if(PumpStatus == 1){
    BlinkStatusBtn.color("#66CCFF"); //turn the status button to blue
    BlinkStatusBtn.print("on"); //turn the button to 'on'
  }
  else{
    BlinkStatusBtn.color("#808080");
    BlinkStatusBtn.print("off");
  }

  if(ManualMode == 1){
    BlinkManualBtn.color("#7CFC00");
    BlinkManualBtn.print("on");
  }
  else{
    BlinkManualBtn.color("#808080");
    BlinkManualBtn.print("off");
  }

  BlinkPlantSliderTarget.print(Moisture_target);
  puloadThread.write("SET",Moisture_target);

  if(PumpStatus == 1){
    //Blinker.wechat("Title:自动浇花系统", "State:浇花中", "Message: 土壤温度:"+(String)Plant_Temp+",湿度:"+(String)Plant_Mois); 
    sc_wechat("%E6%B5%87%E8%8A%B1%E4%B8%AD%2C%E5%9C%9F%E5%A3%A4%E6%B8%A9%E5%BA%A6%3A"+(String)Plant_Temp+",%E6%B9%BF%E5%BA%A6%3A"+(String)Plant_Mois);
  }
  else {
    //Blinker.wechat("Title:自动浇花系统", "State:待机中", "Message: 土壤温度:"+(String)Plant_Temp+",湿度:"+(String)Plant_Mois); 
    sc_wechat("%E6%B5%87%E8%8A%B1%E7%B3%BB%E7%BB%9F%E5%BE%85%E6%9C%BA%E4%B8%AD%2C%E5%9C%9F%E5%A3%A4%E6%B8%A9%E5%BA%A6%3A"+(String)Plant_Temp+",%E6%B9%BF%E5%BA%A6%3A"+(String)Plant_Mois);
  }
}

// Callback function for Xiaomi Mijia and Xiaoai assistant
void MI_IoT_Query(int32_t queryCode)
{
    switch (queryCode)
    {
        case BLINKER_CMD_QUERY_HUMI_NUMBER :
            BlinkerMIOT.humi(Plant_Mois);
            BlinkerMIOT.print();
            break;
        case BLINKER_CMD_QUERY_TEMP_NUMBER :
            BlinkerMIOT.temp(Plant_Temp);
            BlinkerMIOT.print();
            break;
        default :
            BlinkerMIOT.temp(Plant_Temp);
            BlinkerMIOT.humi(Plant_Mois);
            BlinkerMIOT.print();
            break;
    }
}

//水泵状态按钮回调函数
void BlinkStatusBtn_Callback(const String & state){
  if(PumpStatus == 0){
    BlinkStatusBtn.color("#808080");
    BlinkStatusBtn.print("off");
  }
  else{
    BlinkStatusBtn.color("#66CCFF"); 
    BlinkStatusBtn.print("on"); 
  }
}

//云平台湿度设定Slider回调函数
void BlinkPlantSliderTarget_callback(int32_t value){
  Moisture_target = value;
  puloadThread.write("SET",Moisture_target);
}

void BlinkManualBtn_callback(const String & state){
  if(state == "on"){
    ManualMode = 1;
    puloadThread.write("MANUAL",255);
    BlinkManualBtn.color("#7CFC00");
    BlinkManualBtn.print("on");
  }
  if(state == "off"){
    ManualMode = 0;
    puloadThread.write("MANUAL",0);
    BlinkManualBtn.color("#808080");
    BlinkManualBtn.print("off");
  }

}

void blinker_heartbeat(){
  BlinkEnvirNumberTemp.print(Envir_Temp);
  BlinkEnvirNumberHumi.print(Envir_Humi);
  BlinkPlantNumberTemp.print(Plant_Temp);
  BlinkPlantNumberMois.print(Plant_Mois);

  BlinkPlantSliderTarget.print(Moisture_target);
  puloadThread.write("SET",Moisture_target);

  if(PumpStatus == 0){
    BlinkStatusBtn.color("#808080");
    BlinkStatusBtn.print("off");
  }
  else{
    BlinkStatusBtn.color("#66CCFF"); 
    BlinkStatusBtn.print("on"); 
  }

  if(ManualMode == 1){
    BlinkManualBtn.color("#7CFC00");
    BlinkManualBtn.print("on");
  }
  else{
    BlinkManualBtn.color("#808080");
    BlinkManualBtn.print("off");
  }
}


//Server-Chan WeChat Notification Function
void sc_wechat(String message){ 
  WiFiClient serverchan_client;
  HTTPClient server_chan;
  server_chan.begin(serverchan_client,"http://sctapi.ftqq.com/"+ sc_key + ".send?text=%E8%87%AA%E5%8A%A8%E6%B5%87%E8%8A%B1%E7%B3%BB%E7%BB%9F%E9%80%9A%E7%9F%A5&desp=" + message);
  server_chan.GET();
  server_chan.end();
  Blinker.log("ServerChan Message Sended");
}


//Function to Get Sensor data and Water Pump status from IPC
void GET_IPC_SENSOR_DATA(){
  if(puloadThread.available()){
      if(puloadThread.equals("SENSOR")){
        Envir_Temp = puloadThread.read();
        Envir_Humi = puloadThread.read();
        Plant_Mois = puloadThread.read();
        Plant_Temp = puloadThread.read();
        PumpStatus = puloadThread.read();
      }
      if(puloadThread.equals("PUMP")){ // Receive Broadcast that Water Pump is watering
        if(puloadThread.read() == 255){
          PumpStatus = 1;
          }
        else{
          PumpStatus = 0;
          }
      }
    }
}

void setup() {
  puloadThread.begin(115200); //Serial Broadcast start with 115200
  Blinker.begin(auth, ssid, pswd);  //start to connect wifi and Blinker IoT Platform
  BlinkManualBtn.attach(BlinkManualBtn_callback); //register callback function of BlinkManualBtn
  Blinker.attachHeartbeat(blinker_heartbeat); //attach customize heartbeat function
  BlinkPlantSliderTarget.attach(BlinkPlantSliderTarget_callback); //register callback function of BlinkPlantSliderTarget
  BlinkStatusBtn.attach(BlinkStatusBtn_Callback);
  ManuallyRefresh.attach(ManuallyRefreshCallback);
  BlinkerMIOT.attachQuery(MI_IoT_Query); //register callback function of Mi-Mijia
  //BLINKER_DEBUG.stream(Serial);
  //BLINKER_DEBUG.debugAll();
}

void loop() {
  Blinker.run();
  GET_IPC_SENSOR_DATA();
}
