#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h> //提供UDP相关功能
unsigned int UDPremotePort = 14580;
static unsigned char UDPremoteIP[4] = {202,141,*,*} ; //UDP服务器地址
WiFiUDP Udp;
TinyGPSPlus gps;  // The TinyGPS++ object
//SoftwareSerial ss(4, 5); // The serial connection to the GPS device
SoftwareSerial ss(12,14); // The serial connection to the GPS device
const char* ssid = "xm";
const char* password = "12345678";
float latitude , longitude;
int year , month , date, hour , minute , second;
String date_str , time_str , lat_str , lng_str;


int pm;
WiFiServer server(80);
void setup()
{
  Serial.begin(115200);
  ss.begin(38400);
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
}
void loop()
{  
   
  while (ss.available() > 0)
    if (gps.encode(ss.read()))
    {
      if (gps.location.isValid())
      {
        latitude = gps.location.lat();
        lat_str = String(latitude , 6);

        float Aprs_lat = lat_str.substring(0,2).toFloat()*100+lat_str.substring(3,10).toFloat()/1000000*60 ;
        char b[]=""; 
        String str_lat;
        dtostrf( Aprs_lat,3,2,b);
        String n = "N";
        str_lat = b+n;
        Serial.println( str_lat);
        
       
        longitude = gps.location.lng();
        lng_str = String(longitude , 6);
        float Aprs_lng = lng_str.substring(0,3).toFloat()*100+lng_str.substring(4,10).toFloat()/1000000*60 ;
        char a[]=""; 
        String str_lng;
        dtostrf( Aprs_lng,4,2,a);
        String e = "E";
        str_lng = a+e;
        Serial.println(str_lng);
        
        String Aprs_string="BG4WLG-1>ES66:="+str_lat+"/"+str_lng+"="+"\r\n"+"\r\n";
        Serial.println(Aprs_string);
        
 //string 转char
           char Aprs_char[Aprs_string.length() + 1];
           Aprs_string.toCharArray(Aprs_char,Aprs_string.length() + 1);
           Serial.print("Aprs_char");
           Serial.println(Aprs_char);
 

 //以下udp
        Udp.beginPacket(UDPremoteIP,UDPremotePort);
        Udp.write(Aprs_char);
        Udp.endPacket();
    delay(60000);
      }

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
        time_str += String(minute);

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
