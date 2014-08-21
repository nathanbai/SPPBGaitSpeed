/*
 Gait Speed Measurement
 The circuit:
 * start detection analog pin attached to analog input 0
 * stop detection analog pin attached to analog input 1
 * LED anode (long leg) attached to digital output 13
 * TX pin connected to Bluetooth module RX pin
 * RX pin connected to RFID module TX pin for user identification
 Created by Yicheng Bai
 
 If want to use both the softwareserial port to receive data, Listen() function of SoftwareSerial class 
 should be called to switch the SoftwareSerial ports to receive data
 
 May 23rd 2013
 */
 #include "Wire.h"
 #include <SoftwareSerial.h>
 #include <String.h>
 #include <floattostring.h>

SoftwareSerial mySerial(2, 3); // RX, TX
SoftwareSerial WiFlySerial(5,4); // RX, TX

int sensorPin_start = A0;    // select the input pin for the start detection
int sensorPin_stop = A1;    // select the input pin for the stop detection
int ledPin = 8;      // select the pin for the LED
int buttonPin = 6 ;  //select the pin for the push button
int buzzerPin = 11;
int sensorValue_start = 0;  // variable to store the value coming from the start sensor
int sensorValue_stop = 0;  // variable to store the value coming from the stop sensor
boolean start_flag = false;   // flag for start
boolean flag_start_done = false;

unsigned long time_start = 0; // start time in millisecond
unsigned long time_stop = 0; // stop time in millisecond
float gait_speed = 0;
float time_scale = 1000.0;
int time1, time2, testtime;
int score;

String end_string = String("&&&");
String user_name = "";
char buffer_gait_speed[25];

char tag_Mickey[13] = "66006BFB8076"; // Registered RFID tag named Mickey
char tag_Minny[13] = "66006C00B0BA"; // Registered RFID tag named Minny
boolean flag_checked = false;
boolean flag_second_check = false;
int index = 0;

int threshold_offset = 60;
int threshold_offset_return = 35;
int detect_threshold_start = 0;
int detect_threshold_stop = 0;
boolean start_ind = true;
boolean stop_ind = false;

void WiFiSend(String data_send)
{
  WiFlySerial.print(data_send);
  WiFlySerial.println("");
  WiFlySerial.println("");
}

void WiFiSend(unsigned int data_send)
{ 
  WiFlySerial.print(data_send);
  WiFlySerial.println("");
  WiFlySerial.println("");
}

void calculate_speed(unsigned long time_interval) {
  Serial.println("Calc 1");
  gait_speed = 4.0 / (time_interval / time_scale);
  String test_string = "Gait speed of " + user_name + "is ";
  time1 = time_interval;
  //Serial.print(test_string);
  //Serial.println(gait_speed);
  
//  floatToString(buffer_gait_speed,gait_speed,2);
//  
//  String data_send = buffer_gait_speed + end_string;
//  WiFiSend(data_send);
}

void calculate_speed_2(unsigned long time_interval) {
  Serial.println("Calc 2");
  gait_speed = 4.0 / (time_interval / time_scale);
  String test_string = "Gait speed of " + user_name + "is ";
  
  //Serial.print(test_string);
  //Serial.println(gait_speed);
  time2 = time_interval;
  
//  floatToString(buffer_gait_speed,gait_speed,2);
//  String data_send = buffer_gait_speed + end_string;
//  WiFiSend(data_send);
  
  
  if (time1 < time2){testtime = time1;}
  else {testtime = time2;}
  
  
  if (testtime < 4820) {score = 4;}
  else if (testtime >=4281 && testtime <6200) {score = 3;}
  else if (testtime >=6201 && testtime <8700) {score = 2;}
  else if (testtime >=8701) {score = 1;}
  
  String Title = "GaitScore";
  String myScore = String(score);
  String stringscore = String(Title + myScore);
  WiFiSend(stringscore);
  delay(500);
  String Forward = "GaitTimeForward";
  String firstTime = String(Forward + time1);
  WiFiSend(firstTime);
  delay(500);
  String Back = "GaitTimeReturn";
  String lastTime = String(Back + time2);
  WiFiSend(lastTime);
  delay(500);

}

boolean checkTag(char tag[]){
///////////////////////////////////
//Check the read tag against known tags
///////////////////////////////////
  if(strlen(tag) == 0) return false; //empty, no need to contunue

  if(compareTag(tag, tag_Mickey)){ // if matched tag1, do this
    user_name = "Mickey";
    String data_send = user_name + end_string;
    WiFiSend(data_send);
    return true;
  }else if(compareTag(tag, tag_Minny)){ //if matched tag2, do this
    user_name = "Minny";
    String data_send = user_name + end_string;
    WiFiSend(data_send);
    return true;
  }else{
    //Serial.println(tag); //read out any unknown tag
    return false;
  }

}

void clearTag(char one[]){
///////////////////////////////////
//clear the char array by filling with null - ASCII 0
//Will think same tag has been read otherwise
///////////////////////////////////
  for(int i = 0; i < strlen(one); i++){
    one[i] = 0;
  }
}

boolean compareTag(char one[], char two[]){
///////////////////////////////////
//compare two value to see if same,
//strcmp not working 100% so we do this
///////////////////////////////////

  if(strlen(one) == 0) return false; //empty

  for(int i = 0; i < 12; i++){
    if(one[i] != two[i]) return false;
  }

  return true; //no mismatches
}

void setup() {
  // We start the serial library to output our messages.
  Wire.begin();
  Serial.begin(9600);
  WiFlySerial.begin(9600);
  mySerial.begin(9600);
  // declare the ledPin as an OUTPUT:
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin,INPUT);
  digitalWrite(ledPin,LOW);
  pinMode(buzzerPin,OUTPUT);
  digitalWrite(buzzerPin,0);
  sensorValue_start = analogRead(sensorPin_start);
  sensorValue_stop = analogRead(sensorPin_stop); 
  for (int i = 0;i<20;i++)
  {
    sensorValue_start =( sensorValue_start + analogRead(sensorPin_start))/2;
    sensorValue_stop =( sensorValue_stop + analogRead(sensorPin_stop))/2;
  }
  detect_threshold_start = sensorValue_start + threshold_offset;
  detect_threshold_stop = sensorValue_stop + threshold_offset_return;
  
  Serial.print(detect_threshold_start);
  Serial.print(',');
  Serial.println(detect_threshold_stop);
}

boolean rfidTagRecognition()
{
  char tagString[15];
  boolean reading = false;
  
  // detect whether already start the measurement
  
  while (mySerial.available()) {
    Serial.println(mySerial.available());
    int readByte = mySerial.read();
    Serial.write(readByte);
    if (readByte == 2) reading = true;
    if (readByte == 3) reading = false;
      
    if (reading && readByte != 2 && readByte != 10 && readByte != 13) {
      tagString[index] = readByte;
      index ++;
    }      
  }
  //Serial.println(index);
  //&& checkTag(tagString
  if (index == 12 ) {  
    WiFiSend("RFID_Tag");
        delay(1000);
        tagString[12] = 'G';
        tagString[13] = 'T';
        tagString[14] = '\0';
        WiFiSend(tagString);       
        delay(400);
    flag_checked = true;
    digitalWrite(ledPin, HIGH);
    clearTag(tagString);
    index = 0;
    Serial.println("Match found!");
    delay(200);
    digitalWrite(ledPin, LOW);
    delay(1000);
    return true;
  } else {
    return false;
  }
}

boolean checkButton()
{
  int buttonPress = digitalRead(buttonPin);
    
  if(buttonPress){
    delay(300);
    if(buttonPress){
      flag_second_check = true;
      String data_send = "Return Button" + end_string;
      WiFiSend(data_send);
      return true;
    }
    return false;
    //Serial.println("Button Pressed");
  }
  
}

boolean check_value(boolean terminal)
{
  int sensorValue_start = 0;
  int sensorValue_stop = 0;
  sensorValue_start = analogRead(sensorPin_start);
  sensorValue_stop = analogRead(sensorPin_stop);
  Serial.print(sensorValue_start);
  Serial.print(',');
  Serial.println(sensorValue_stop);
  if(terminal){
    sensorValue_start = analogRead(sensorPin_start);
    if (sensorValue_start > detect_threshold_start){
      delay(100);
      sensorValue_start = analogRead(sensorPin_start);
      if (sensorValue_start > detect_threshold_start){
        Serial.println(sensorValue_start);
        return true;
      }
    }
  }else{
    sensorValue_stop = analogRead(sensorPin_stop); 
    if (sensorValue_stop > detect_threshold_stop){
      delay(100);
      sensorValue_stop = analogRead(sensorPin_stop);
      if (sensorValue_stop > detect_threshold_stop){
        Serial.println(sensorValue_stop);
        return true;
      }
    }
  }
  return false;
}

void loop() {
  
  // if the rfid is not scanned, read the rfid to set the flag
  if (!flag_checked) {
    while(!rfidTagRecognition());
  }
  
  if (flag_checked) {
    if(!flag_second_check) {
      if (!flag_start_done) {
        if (start_flag) {
          if (check_value(stop_ind)) {
            Serial.println("Loop1");
            time_stop = millis();
            digitalWrite(buzzerPin, HIGH);
          delay(500);
          digitalWrite(buzzerPin,0);
            calculate_speed(time_stop - time_start);
            flag_start_done = true;
            start_flag = false;
            while(!checkButton());
            delay(2000);
          }
        } else {
          if (check_value(start_ind)) {
            Serial.println("Loop2");
            time_start = millis();
             digitalWrite(buzzerPin, HIGH);
          delay(500);
          digitalWrite(buzzerPin,0);
            start_flag = true;
            delay(1000);
          }
        }
      }
    } else {
      if (start_flag) {
        if (check_value(start_ind)) {
          Serial.println("Loop3");
          time_stop = millis();
          digitalWrite(buzzerPin, HIGH);
          delay(500);
          digitalWrite(buzzerPin,0);
          calculate_speed_2(time_stop - time_start);
          
          flag_checked = false;
          flag_second_check = false;
          flag_start_done = false;
          start_flag = false;
          delay(1000);
        }
      } else {
        if (check_value(stop_ind)) {
          Serial.println("Loop4");
          time_start = millis();
           digitalWrite(buzzerPin, HIGH);
          delay(500);
          digitalWrite(buzzerPin,0);
          digitalWrite(ledPin, LOW);
          start_flag = true;
          delay(1000);
        }
      }
    }
  }
}
