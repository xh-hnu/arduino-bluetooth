/* 
TSL2561:          HC_06:                OLED:                 DHT11
3V3 to 3.3V       5v to 5v              5v to 5v              +  to  5v
GND to GND        GND to GND            GND to GND            -  to  GND
SDA to A4         TXD to arduino Pin 4  SDA to A4             OUT  to  arduino 2
SCL to A5         RXD to arduino Pin 5  SCL to A5
TSL2561 ADDR                            OLED  SSD1306 ADDR
0x29 or 0x39 or 0x49                    0x3c  or  0x3d
*/
#include <SparkFunTSL2561.h>
#include <Wire.h>
#include "U8glib.h"
#include "const.h"
#include "string.h"
#include <SoftwareSerial.h>
#include "SDHT.h"

#define HC_06_TX 4
#define HC_06_RX 5

SFE_TSL2561 light;

// Global variables:

boolean gain;     // Gain setting, 0 = X1, 1 = X16;
unsigned int ms;  // Integration ("shutter") time in milliseconds
int8_t notice;
char lux_str[6];
char temp_str[6];
char hum_str[6];
String send_data;

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);  // I2C / TWI 
SoftwareSerial mySerial(HC_06_TX,HC_06_RX);//RX TX
SDHT dht;

//显示函数
void draw(char *lux, char *temp, char *hum){
  u8g_uint_t d;
  //中文字符显示   无线气象信息
  u8g.drawXBMP(0, 0, 16, 16, wu); 
  u8g.drawXBMP(18, 0, 16, 16, xian); 
  u8g.drawXBMP(36, 0, 16, 16, qi); 
  u8g.drawXBMP(54, 0, 16, 16, xiang); 
  u8g.drawXBMP(72, 0, 16, 16, xin); 
  u8g.drawXBMP(90, 0, 16, 16, xi); 
  //中文字符显示  光强 温度 湿度
  u8g.drawXBMP(0, 20, 16, 16, guang); 
  u8g.drawXBMP(18, 20, 16, 16, qiang); 
  u8g.drawXBMP(43, 20, 16, 16, wen); 
  u8g.drawXBMP(61, 20, 16, 16, du);
  u8g.drawXBMP(83, 20, 16, 16, shi);
  u8g.drawXBMP(101, 20, 16, 16, du);
  u8g.setFont(u8g_font_9x15);
  u8g.setFontRefHeightText();
  u8g.setFontPosTop();
  u8g.drawStr(0,40,lux);
  u8g.drawStr(0,53,"Lux");
  u8g.drawStr(43,46,temp);
  d = 43 + u8g.getStrWidth(temp);
  u8g.drawXBMP(d, 43, 16, 16, c);
  u8g.drawXBMP(d + 4, 44, 16, 16, Celcius);
  u8g.drawStr(83,46,hum);
  d = 83 + u8g.getStrWidth(hum);
  u8g.drawXBMP(d, 44, 16, 16, h);
}


void setup(){
  // Initialize the Serial port:
  mySerial.begin(9600);
  Serial.begin(9600);
  Serial.println("TSL2561 example sketch");

  light.begin();
  
  unsigned char ID;
  
  if (light.getID(ID)){
    Serial.print("Got factory ID: 0X");
    Serial.print(ID,HEX);
    Serial.println(", should be 0X5X");
  }
  else{
    Serial.println("error");
  }
  gain = 0;
  // If time = 0, integration will be 13.7ms
  // If time = 1, integration will be 101ms
  // If time = 2, integration will be 402ms
  // If time = 3, use manual start / stop to perform your own integration
  unsigned char time = 2;
  Serial.println("Set timing...");
  light.setTiming(gain,time,ms);
  Serial.println("Powerup...");
  light.setPowerUp();
}

void loop(){
  delay(ms);
  unsigned int num = 0;
  if ((notice  = dht.broadcast(DHT11, 2)) == SDHT_OK) 
    layout(); 
  //一次性显示缓存数据
  while(mySerial.available())   {     
    char c = mySerial.read(); 
    Serial.print(c);   
    while(c == 'S'){
      while(!mySerial.available()){
      }
      c = mySerial.read(); 
    }
  }

  unsigned int data0, data1;
  if (light.getData(data0,data1)){
    double lux;    // Resulting lux value
    boolean good;  // True if neither sensor is saturated
    // Perform lux calculation:
    light.getLux(gain,ms,data0,data1,lux);
    int lux_int = lux;
    itoa(lux_int, lux_str, 10);//int to string 10进制
    // Print out the results:
    send_data = "lux" + (String)lux_str;
    mySerial.println(send_data);
    Serial.println(send_data);
  }
  else{
    //Serial.println("error");
  }
   u8g.firstPage();  
  do {
      draw(lux_str, temp_str, hum_str);
    } while( u8g.nextPage());
}

void layout() {
  send_data = "hum" + String(dht.humidity, 0);
  mySerial.println(send_data);
  Serial.println(send_data);
  itoa(dht.humidity, hum_str, 10);//int to string 10进制
  send_data = "tem" + String(dht.celsius, 0);
  mySerial.println(send_data);
  Serial.println(send_data);
  itoa(dht.celsius, temp_str, 10);//int to string 10进制
}
