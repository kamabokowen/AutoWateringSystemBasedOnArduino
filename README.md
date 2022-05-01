# Auto Watering System Based On Arduino 
# 基于Arduino的智能自动浇花系统
This is my graduation design for my bachelor's degree. 

Это мой дипломный проект для получения степени бакалавра. 

这是我的本科毕业设计程序代码，一个智能自动浇花系统。

It is an automatic flower watering system, the main control is based on Arduino Leonardo, using ESP8266 as a Wi-Fi module. 

Это автоматическая система полива цветов, основное управление основано на Arduino Leonardo, с использованием ESP8266 в качестве Wi-Fi модуля. 

主控板是Arduino Leonardo，使用ESP8266作为串口转Wi-Fi通信模块。

It enables the user to monitor not only the humidity and temperature of the soil in nearly real time, but also the temperature and humidity of the environment. 
The program is based on Blinker cloud platform, also support Mijia Xiaoai, and Server-Chan WeChat push.

Она позволяет пользователю практически в реальном времени отслеживать не только влажность и температуру почвы, но и температуру и влажность окружающей среды. 
Программа основана на облачной платформе Blinker, также поддерживает Mijia Xiaoai, и Server-Chan WeChat push.

这个系统可以实时查看环境温湿度和土壤温湿度，还可以实现简单的自动化浇花功能（通过控制一个潜水泵）。程序依赖Blinker点灯科技云平台，同时支持米家小爱同学语音查询温湿度，以及通过Server酱推送设备信息。


I'd like to give special thanks to DFrobot. although I couldn't use it in the end because of the compatibility problem of FireBeetle, they gave me a lot of inspiration. people from Arduino Chinese community and official community also helped me to solve many problems.

Я хотел бы выразить особую благодарность DFrobot. хотя я не смог использовать его в конечном итоге из-за проблемы совместимости FireBeetle, они дали мне много вдохновения. люди из китайского сообщества Arduino и официального сообщества также помогли мне решить многие проблемы.

I would like to commemorate my four years of college life with this project.

Этим проектом я хотел бы увековечить свои четыре года жизни в колледже.
 
 
 
Changchun University, Jilin, China 

长春大学中俄学院自动化2022届毕业生，羽坂

## List of sensors used 使用到的传感器
* DHT11 Temperature and humidity sensor DHT11温湿度传感器（监测环境温湿度）
* DS18B20 Waterproof Temperature Sensor DS18B20防水温度传感器（检测土壤温度）
* Soil moisture sensor (analog signal) 一个输出模拟信号的土壤湿度传感器
