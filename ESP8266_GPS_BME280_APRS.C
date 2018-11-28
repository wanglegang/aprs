//2018年11月28日增加速度，高度，卫星数量，精度等
#include <TinyGPS++.h>   //GPS库
#include <SoftwareSerial.h>  //读取PGS软串口库
#include <ESP8266WiFi.h>   //esp8266 wifi
#include <WiFiUdp.h> //提供UDP相关功能
//BME280 
#include <Adafruit_BME280.h>   //温湿度压力传感器库
#include <Wire.h>
#define SDA D3      //BME280 I2C在nodemcu
#define SDC D4      //BME280 I2C在nodemcu
//bme280 初始化
Adafruit_BME280 bme280;   
bool bme280_CanRead = true;

//判断280针脚地址
bool initBME280(char addr) {
  if (bme280.begin(addr)) {
    Serial.println("found");
    return true;
  } else {
    Serial.println("not found");
    return false;
  }
}
//UDP服务器设置
unsigned int UDPremotePort = 14580;  //端口号
static unsigned char UDPremoteIP[4] = {202,141,176,2} ; //UDP服务器地址
WiFiUDP Udp; 
TinyGPSPlus gps;  // The TinyGPS++ object  gps初始化
SoftwareSerial ss(12, 14); // 接收gps的软串口
const char* ssid = "Uolian-2.4";    //路由器ssid
const char* password = "13505311306";  // 密码
float latitude , longitude;  //浮点型定义经纬度变量
int year , month , date, hour , minute , second; //定义年月日
String date_str , time_str , lat_str , lng_str;
int pm;
//上传aprs数据周期
unsigned long uptime = 0;
unsigned long last_send = 0;
const unsigned long sending_interval = 60000;

//定义http服务器地址
WiFiServer server(80);

void setup()
{
  Serial.begin(115200);
  ss.begin(38400);  //gps模块波特率
  //连接无线路由器
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  server.begin();
  Serial.println("Server started");
  // Print the IP address
  Serial.println(WiFi.localIP());

  //BME280 设置
  Wire.pins(SDC, SDA);
  Wire.begin(SDC, SDA);
  // BME280 地址判断
  if (bme280_CanRead) {
    if (!initBME280(0x76) && !initBME280(0x77)) {
      Serial.println("Check BME280 wiring");
      bme280_CanRead = false;
    }
      }
  delay(2500);
}
void loop()
{  
   uptime = millis();
//  if (last_send > uptime){  //时间溢出后，处理计时
//     last_send = 0;}
 if (last_send == 0 || last_send + sending_interval < uptime){
     Serial.print("   last:");
     Serial.println(last_send);
     Serial.print("uptime:");
     Serial.println(uptime);
  //处理gps数据 
//  while (ss.available() > 0)  //成功读取到
//    // Serial.println("while");
//    if (gps.encode(ss.read()))
//    {      Serial.println("Gprs read");
//           
//      if (gps.location.isValid())
//      { Serial.println("Gprs valid");
    //处理lat 经度     
        latitude = gps.location.lat();  //经度
        lat_str = String(latitude , 6); //浮点转字符串
        float Aprs_lat = lat_str.substring(0,2).toFloat()*100+lat_str.substring(3,10).toFloat()/1000000*60 ; //39.851087N，转成aprs：39*100+0.851087*60=3951.06
	  //lat_str.substring(0,2).toFloat()*100 ，截取39.851087中的39*100 
        char char_lat[]=""; 
        dtostrf( Aprs_lat,3,2,char_lat);  //浮点转char,3代表小数点前面取3位，2代表小数点后面取2位，
        String str_lat;
        String N = "N"; 
        str_lat = char_lat+N;  //根据aprs要求拼接+N
    //    Serial.println( str_lat); 
        
    //处理lng 纬度   
        longitude = gps.location.lng(); 
        lng_str = String(longitude , 6);
        float Aprs_lng = lng_str.substring(0,3).toFloat()*100+lng_str.substring(4,10).toFloat()/1000000*60 ; //经度116.326323E，转成aprs：116*100+0.326323*60=11619.58
        char char_lng[]=""; 
        String str_lng;
        dtostrf( Aprs_lng,4,2,char_lng);
        String E = "E";
        str_lng = char_lng+E;
        //Serial.println(str_lng);

      //处理温度，改成aprs标准数据 t华氏度=（摄氏度*9/5）+32 ，处理小于10  小于100  小于1000
         float Temperature=(bme280.readTemperature()*9/5)+32;
         Serial.print("Temperature:");
         Serial.println(Temperature);

   //      if(number<100)
   //   {
           char char_Temperature[]="";
           dtostrf( Temperature,2,0,char_Temperature);
           String T = "0";
           String Aprs_Temperature=T+char_Temperature;
         //  Serial.println(Aprs_Temp);
 
   //处理湿度，处理小于10 
           char Humidty[]="";
           dtostrf( bme280.readHumidity(),2,0,Humidty);
           Serial.print("Aprs_Humidty:");
           Serial.println(Humidty);
           String Aprs_Humidty=Humidty;
 
   //处理大气压，处理小于 
   //      if(number<100)
   //   {
           char Pressure[]="";
           dtostrf(bme280.readPressure()/10,4,0,Pressure);
   //        Serial.print("Aprs_Pressure:");
   //        Serial.println(Pressure);
           String Aprs_Pressure=Pressure;

 //拼接aprs数据格式          
           String Aprs_string="BG4WLG-1>QQ124051517:="+str_lat+"/"+str_lng+"="+"000/000g000t"+Aprs_Temperature+"r000p000h"+Aprs_Humidty+"b"+Aprs_Pressure+"温度:"+bme280.readTemperature()+"°C"+" 湿度:"+bme280.readHumidity()+"%"+" 大气压:"+bme280.readPressure()/100+"mpar"+ " 高度:" + gps.altitude.meters() + "米" +" 速度:" + gps.speed.kmph() + "千米/时" + " 卫星数量:" + gps.satellites.value() + "颗" +" 精度:" + gps.hdop.value() + "米" +"\r\n"+"\r\n";

        //   Serial.print("Aprs_string:");
        //   Serial.println(Aprs_string);
         
 //拼接出aprs后string 转char
           char Aprs_char[Aprs_string.length() + 1];  //定义数组
           Aprs_string.toCharArray(Aprs_char,Aprs_string.length() + 1);
           Serial.print("Aprs_char:");
           Serial.println(Aprs_char);
 

 //以下udp
           Udp.beginPacket(UDPremoteIP,UDPremotePort);
           Udp.write(Aprs_char);
           Udp.endPacket();
        //   delay(3000);   //间隔发送aprs 时间
//       }
//    }
//    
    last_send = uptime;
 }   

/*
//BME280 READ
  if (bme280_CanRead)
  {
    Serial.print("t:");
    Serial.print(bme280.readTemperature());
    Serial.print("  h:");
    Serial.print(bme280.readHumidity());
    Serial.print("  p:");
    Serial.println(bme280.readPressure());
  }
*/

// 读取gps数据
  while (ss.available() > 0)
    if (gps.encode(ss.read()))
    {
      if (gps.location.isValid())
      {
        latitude = gps.location.lat();
        lat_str = String(latitude , 6);
        Serial.print("lat:");
        Serial.print (lat_str);
        longitude = gps.location.lng();
        lng_str = String(longitude , 6);
        Serial.print("  lng:");
        Serial.println(lng_str);
      }

 //HTTP    
      if (gps.date.isValid())
      {
        date_str = "";
        date = gps.date.day();
        month = gps.date.month();
        year = gps.date.year();

        if (date < 10)
          date_str = '0';
        date_str += String(date);

        date_str += " / ";

        if (month < 10)
          date_str += '0';
        date_str += String(month);

        date_str += " / ";

        if (year < 10)
          date_str += '0';
        date_str += String(year);
      }

      if (gps.time.isValid())
      {
        time_str = "";
        hour = gps.time.hour();
        minute = gps.time.minute();
        second = gps.time.second();

        minute = (minute + 30);
        if (minute > 59)
        {
          minute = minute - 60;
          hour = hour + 1;
        }
        hour = (hour + 5) ;
        if (hour > 23)
          hour = hour - 24;

        if (hour >= 12)
          pm = 1;
        else
          pm = 0;

        hour = hour % 12;

        if (hour < 10)
          time_str = '0';
        time_str += String(hour+2);

        time_str += " : ";

        if (minute < 10)
          time_str += '0';
        time_str += String(minute+30);

        time_str += " : ";

        if (second < 10)
          time_str += '0';
        time_str += String(second);

        if (pm == 1)
          time_str += " PM ";
        else
          time_str += " AM ";
          
    }
  }
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client)
  {
    return;
  }

  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n <!DOCTYPE html> <html> <head> <title>GPS Interfacing with NodeMCU</title> <style>";
  s += "a:link {background-color: YELLOW;text-decoration: none;}";
  s += "table, th, td {border: 1px solid black;} </style> </head> <body> <h1  style=";
  s += "font-size:300%;";
  s += " ALIGN=CENTER> GPS Interfacing with NodeMCU</h1>";
  s += "<p ALIGN=CENTER style=""font-size:150%;""";
  s += "> <b>Location Details</b></p> <table ALIGN=CENTER style=";
  s += "width:50%";
  s += "> <tr> <th>Latitude</th>";
  s += "<td ALIGN=CENTER >";
  s += lat_str;
  s += "</td> </tr> <tr> <th>Longitude</th> <td ALIGN=CENTER >";
  s += lng_str;
  s += "</td> </tr> <tr>  <th>Date</th> <td ALIGN=CENTER >";
  s += date_str;
  s += "</td></tr> <tr> <th>Time</th> <td ALIGN=CENTER >";
  s += time_str;
  s += "</td>  </tr> </table> ";
 
  
  if (gps.location.isValid())
  {
     s += "<p align=center><a style=""color:RED;font-size:125%;"" href=""http://maps.google.com/maps?&z=15&mrt=yp&t=k&q=";
    s += lat_str;
    s += "+";
    s += lng_str;
    s += """ target=""_top"">Click here!</a> To check the location in Google maps.</p>";
  }

  s += "</body> </html> \n";

  client.print(s);
  delay(100);
 

}
