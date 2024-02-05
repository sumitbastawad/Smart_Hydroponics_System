/*NAME: Sumit A BASTAWAD*/
////////////BLYNK//////////
#define BLYNK_TEMPLATE_ID           "TMPL3I35r5Eyd" //blynk tamplate id
#define BLYNK_TEMPLATE_NAME         "Quickstart Template" //blynk tamplate name
#define BLYNK_AUTH_TOKEN            "kE5Eu1FfYbxMjDWXXBkdCQVtSHxiaTfE" //blynk authontication token
#define BLYNK_PRINT Serial
#include <WiFi.h>                  //Add the WiFi library
#include <WiFiClient.h>            //Add the WiFiClint library
#include <BlynkSimpleEsp32.h>      //Add the BlynkSimpleEsp32 library
char ssid[] = "sumit";            //Add the wifi name
char pass[] = "sumit123";         //Add the wifi password

///////////TIME/////////////
#include <NTPClient.h>       //Add NTPclint hedder file 
#include <WiFiUdp.h>         //Add the WiFiUdp hedder file
#define lightpin 4           //set the light controll pin
const char* ntpServer = "pool.ntp.org"; //NTP server
const int timeZone = 0; // Set the time zone offset in hours 
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, timeZone);
int hour; 
int minute;
int second;

////////////TDS/////////////////
#include <EEPROM.h>
#include "GravityTDS.h" // Add the gravityTDS hedder file 
#define TdsSensorPin 39 //initialise the TDS sensor pin
#define TdsVCCPin 2     //initialise the TDSVccpin for contolling the TDS senor to base of NPN trasistor 2N2222
GravityTDS gravityTds; 
float temperature = 25,tdsValue = 0;
float EC_ideal = 1100; // set the ideal TDS level of the water 
float EC_low = 1000;   // set the minimum TDS level of water


///////////PH/////////
#define phpin 36 //initialise the pH sensor pin
#define PhVCCPin 15 //initialise the PhVccpin for contolling the pH senor to base of NPN trasistor bc337
float calibration_value = 19;
int phval = 0; 
unsigned long int avgval; 
int buffer_arr[10],tempph;
float volt;
float ph_act;  
unsigned long int avgValue;  
float b;
int buf[10],temp;
float PH_high = 6.5; //set the miximum pH level of the water
float PH_ideal = 6.0;//set the ideal pH level of the water
float PH_low = 5;//set the minimum pH level of the water

////////////PUMP//////////////
#define sub_pump 19 //initialise the pin for submersible pump 
#define nuta_pump 18 //initialise the pin for nutrient A pump
#define nutb_pump 5 //initialise the pin for nutrient B pump
#define phup_pump 17 //initialise the pin for pH up  pump
#define phdown_pump 16 //initialise the pin for pH doen pump

///////////FLOW SENSOR/////////
const int flowSensorPin =14; //inialise the pin for flowsensor
volatile int flowCount;
float flowRate;  //initialise the veriablre 

///////////DHT11///////////
#include <Adafruit_Sensor.h>
#include <DHT.h>
#define DHTPIN 27         // pin where the DHT11 sensor is connected
#define DHTTYPE DHT11     // type of the DHT11 sensor
DHT dht(DHTPIN, DHTTYPE); // initialize the DHT11 sensor
float h;
float t;
#define cooler 0

///////////LCD//////////////
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup(){
  Serial.begin(115200);//set boad rate
  //blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  //TDS SENSOR
  gravityTds.setPin(TdsSensorPin); 
  pinMode(TdsVCCPin,OUTPUT);
  //PH
  pinMode(phpin,INPUT);
  pinMode(PhVCCPin,OUTPUT);
  //FLOWSENSOR
  pinMode(flowSensorPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(flowSensorPin), pulseCounter, RISING);
  //DHT11
  dht.begin();
  pinMode(cooler,OUTPUT);
  
  /////////////PUMP/////////////
  pinMode(sub_pump,OUTPUT);
  digitalWrite(sub_pump, HIGH);
  pinMode(nuta_pump,OUTPUT);
  digitalWrite(nuta_pump, HIGH);
  pinMode(nutb_pump,OUTPUT);
  digitalWrite(nutb_pump, HIGH);
  pinMode(phup_pump,OUTPUT);
  digitalWrite(phup_pump,HIGH);
  pinMode(phdown_pump,OUTPUT);
  digitalWrite(phdown_pump,HIGH); 
   
  ////////////////TIME////////////
  timeClient.begin();
  timeClient.update(); // Get the initial time value
  pinMode(lightpin, OUTPUT);
  digitalWrite(lightpin,HIGH);

  ///////////LCD//////////////
  lcd.init(); // initialize the lcd
  lcd.backlight();
  lcd.init(); 
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("   Welcome to    ");
  for(int i=15;i>=-13;i--)
  {
  lcd.setCursor(i,1);
  lcd.print("smart hydrophonics  ");
  delay(200);
  }
}

void loop(){
  Blynk.run(); //
  light_cont();//call the light controlling funtion
  dht_sensor();//call the dht11 sensor funtion 
  cooler_cont();//call the cooling controlling  funtion 
  time_call();//call the timing funtion
  subpump_cont();//call the submersable pump conotrolling funtion

 
////////////////SERIAL MONOTORING/////////////
 //humudity 
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  //temprature
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C");
  //tds value
  Serial.print("TDS Value:");
  Serial.print(tds_sensor(),0);
  Serial.println("ppm");
  //ph sensor
  Serial.print("    pH:");  
  Serial.print(ph_sensor());
  Serial.println(" ");
  //flow sensor
  Serial.print("Flow rate: ");
  Serial.print(flowRate);
  Serial.println(" L/min");
  //time
  Serial.print("Current time: ");
  Serial.print(hour);
  Serial.print(":");
  Serial.print(minute);
  Serial.print(":");
  Serial.println(second);
    lcd_call();
    data_sent();
}
/////////////////////////////////////////////////////////////////////////////////sensors//////////////////////////////////////////////////////////////////////////////////////////

//////////////////TDS SENSOR///////////////////

float tds_sensor()                                    // TDS sensor funtion 
      {
      digitalWrite(PhVCCPin,LOW);                     // turn off the pH sensor 
      digitalWrite(TdsVCCPin,HIGH);                   //turn on the TDS sensor 
      delay(250);
      gravityTds.setTemperature(temperature);  
      gravityTds.update();  
      tdsValue = gravityTds.getTdsValue();            //read the TDS vallue
      return tdsValue;                                // return the read TDS value
      }

///////////////PH SENSOR///////////////////

float ph_sensor()                                     //pH sensor funtion
   { 
   digitalWrite(PhVCCPin,HIGH);//turn on the pH sensor 
   digitalWrite(TdsVCCPin,LOW);//Turn off the TDS sensor
   delay(250);
   for(int i=0;i<10;i++) 
    { 
    buffer_arr[i]=analogRead(phpin);
    delay(30);
    }
   for(int i=0;i<9;i++)
    {
     for(int j=i+1;j<10;j++)
     {
      if(buffer_arr[i]>buffer_arr[j])
      {
      temp=buffer_arr[i];
      buffer_arr[i]=buffer_arr[j];
      buffer_arr[j]=temp;
      }
     }
    }
   avgval=0;
   for(int i=2;i<8;i++)
   avgval+=buffer_arr[i]; 
   volt = (float) avgval*3.3/4096/6;
   ph_act = -5.70 * volt + calibration_value;
   return ph_act; 
   }

///////////////DHT11////////////

void dht_sensor() //dht11 sensor funtion
  { 
   h = dht.readHumidity();      // read the humidity value from the DHT11 sensor
   t = dht.readTemperature();   // read the temperature value from the DHT11 sensor
  }

////////FLOW SENSOR////////////
float flow_sensor()//flow sensor funtion
  {
  flowCount = 0;
  delay(1000);
  flowRate = ((float)flowCount / 7.5);     //calculate the flowrate
  return flowRate;                         //return the flow rate
  }
void pulseCounter()
  {
   flowCount++;
  }
////////////////////TIME//////////////
void time_call() // time funtion
  {
   timeClient.update(); // Update the time from the NTP server
   hour = timeClient.getHours();//get hour from the NTP server
   minute = timeClient.getMinutes();//get minute from the NTP server
   second = timeClient.getSeconds();//get second from the NTP server
  }
////////////////////////////////////////////////////////////////////////controlling//////////////////////////////////////////////////////////////////////////////////////////////////////  
///////////////////////TDS CONTROL///////////////
void tds_cont() //TDS controlling funtion 
     {  
      if (tds_sensor() < EC_low) 
      { 
       digitalWrite(PhVCCPin,LOW);
       digitalWrite(TdsVCCPin,HIGH);
       delay(500);
       Serial.print("LOW TDS level: ");
       Serial.println(tds_sensor());
       digitalWrite(sub_pump, HIGH);
       while (tds_sensor() < EC_ideal)
       { 
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("FIXING  SOLUTION");
        lcd.setCursor(0, 1);
        lcd.print(tds_sensor());
        digitalWrite(sub_pump, LOW);
        raiseConductivity();
        Serial.println("Mixing solution..."); 
        delay(60000); //Wait one minute for solution to mix
        digitalWrite(sub_pump, HIGH);
        Serial.println("Mixing complete.");
        Serial.print("New conductity level: ");
        Serial.println(tds_sensor());
       }
      digitalWrite(sub_pump, HIGH);
      Serial.println("tds levels fixed.");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("SOLUTION  FIXED!");
      lcd.setCursor(0, 1);
      lcd.print(tds_sensor());
      }         
     }

///////////////RAISE PH/////////////

void rise_ph()//rising the pH of the water funtion 
   {
    if (ph_sensor() < PH_low) 
     { 
      digitalWrite(PhVCCPin,HIGH);
      digitalWrite(TdsVCCPin,LOW);
      delay(500);
      Serial.print("LOW pH level: ");
      Serial.println(ph_sensor());
      digitalWrite(sub_pump, HIGH);
      while (ph_sensor() < PH_ideal)
        { 
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("   FIXING  PH   ");
        lcd.setCursor(0, 1);
        lcd.print(ph_sensor());
        digitalWrite(sub_pump, LOW); 
        raisePH();
        Serial.println("Mixing solution...");
        delay(60000); //Wait one minute for solution to mix
        digitalWrite(sub_pump, HIGH); 
        Serial.println("Mixing complete.");
        Serial.print("New pH level: ");
        Serial.println(ph_sensor());
        }
        digitalWrite(sub_pump, HIGH);
        Serial.println("pH levels fixed.");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("   PH FIXED!   ");
        lcd.setCursor(0, 1);
        lcd.print(ph_sensor());
    }
   }
////////////LOWER PH//////////////////////////
 void lower_ph(){ //lowering the pH of the water funtion 
    if (ph_sensor() > PH_high) {
      digitalWrite(PhVCCPin,HIGH);
      digitalWrite(TdsVCCPin,LOW); 
      delay(500);
      Serial.print("HIGH pH level: ");
      Serial.println(ph_sensor());
      digitalWrite(sub_pump, HIGH);
      while (ph_sensor() > PH_ideal)
      { 
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("   FIXING  PH   ");
        lcd.setCursor(0, 1);
        lcd.print(ph_sensor());
        digitalWrite(sub_pump, LOW); 
        lowerPH(); 
        Serial.println("Mixing solution..."); 
        delay(60000); //Wait one minute for solution to mix
        digitalWrite(sub_pump, HIGH);
        Serial.println("Mixing complete.");
        Serial.print("New pH level: ");
        Serial.println(ph_sensor());
      }
      digitalWrite(sub_pump, HIGH);
      Serial.println("pH levels fixed.");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("   PH FIXED!   ");
      lcd.setCursor(0, 1);
      lcd.print(ph_sensor());
     }
    }
 
/////////////PUMP CONTOL////////////
void raiseConductivity(){
  digitalWrite(nuta_pump, LOW);
  digitalWrite(nutb_pump, LOW);
  delay(10000); 
  digitalWrite(nuta_pump, HIGH);
  digitalWrite(nutb_pump, HIGH);
}
void raisePH(){
  digitalWrite(phup_pump,LOW);
  delay(10000); 
  digitalWrite(phup_pump,HIGH);
}
void lowerPH(){
  digitalWrite(phdown_pump,LOW);
  delay(10000); 
  digitalWrite(phdown_pump,HIGH);
}

//////////////////light controll///////////
void light_cont(){      
      //if(hour>=1)
  //{
    digitalWrite(lightpin,LOW);
  //}
  //if(hour<=24)
  //{
    digitalWrite(lightpin,HIGH);
  //}
  }   
/////////////subpump controll/////////////  
void subpump_cont()//funtion to turn on the submersible pump for 10 min every hours
   {  
    if(minute<=10)//if minute is less or equal to 10 minute 
     {
     digitalWrite(sub_pump,LOW);//turn on the submersible pump 
     flow_sensor();// call the flow sensor funtion
     }
      if(minute>=11)// if the minute is grater then the 
     {
     digitalWrite(sub_pump,HIGH);//turn off the submersible pump
     tds_sensor(); //call the tds sensor funtion to get TDS level of the water
     ph_sensor();  //call the tds sensor funtion to get pH level of the water
     tds_cont();   //call the TDS controlling the funtion
     /rise_ph();    //call the pH rising funtion 
     //lower_ph();   // call the pH lowering funtion 
     }
   }   
///////////////////COOOLER///////////////
void cooler_cont()//coller contolling funtion 
  {
   if(t>=30) //if temprature is grater then the 30C 
   {
   digitalWrite(cooler,LOW); //turn on the cooler
   }
   if(t<=29) //if temperature is less then 30c
   {
    digitalWrite(cooler,HIGH); //turn off the cooler
   }
  }
  
 ///////////////LCD/////////////
 void lcd_call()//funtion to display data on LCD
     {     
      delay(500);
      lcd.clear();// clear the LCD
      lcd.setCursor(0, 0);//set the cursor 
      lcd.print("pH  Value: ");
      lcd.setCursor(11, 0);
      lcd.print(ph_sensor());//
      lcd.setCursor(0, 1);
      lcd.print("TDS Value: ");
      lcd.print(tds_sensor()); 
      delay(1000);
      lcd.setCursor(0, 0);
      lcd.print("temprature: ");
      lcd.setCursor(11, 0);
      lcd.print(t);
      lcd.setCursor(0, 1);
      lcd.print("humidity  : ");
      lcd.print(h);
     }
 
/////////BLYNK////////////
void data_sent()            //funtion to send the sensor data to blynk cloude
 {
  Blynk.virtualWrite(V1, h);//send the humudity value to the vertual pin v1 of the blynk server
  Blynk.virtualWrite(V0, t);//send the temperature value to the vertual pin v2 of the blynk server
  Blynk.virtualWrite(V2, ph_act);//send the ph value value to the vertual pin v3 of the blynk server
  Blynk.virtualWrite(V3, tdsValue);//send the tds value value to the vertual pin v4 of the blynk server
  Blynk.virtualWrite(V4, flowRate);//send the water flow rate to the vertual pin v5 of the blynk server
 }
