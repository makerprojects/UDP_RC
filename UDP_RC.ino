/*
UDP Remote Control

Please refer to www.pikoder.com for more information

last change: 05.29.16 - initial release

Copyright 2016 Gregor Schlechtriem

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#define DEBUG true

#define cNumChan 2

#define Channel_1 9
#define Channel_2 10

#include <SoftwareSerial.h>
#include <Servo.h>

SoftwareSerial esp8266(11, 12); // RX, TX
Servo myChan1;
Servo myChan2;

void setup() {
  Serial.begin(19200);
  esp8266.begin(19200);
  
  esp8266.setTimeout(5000);
  if (sendCom("AT+RST", "ready")) {
    debug("RESET OK");
  }

  if (configAP()) {
    debug("AP ready");
  }
  if (configUDP()) {
    debug("UDP ready");
  }
  
  //shorter Timeout for faster wrong UPD-Comands handling
    esp8266.setTimeout(1000);
   
    myChan1.attach(Channel_1);
    myChan1.write(90);   // neutral position     

    myChan2.attach(Channel_2);
    myChan2.write(90);   // neutral position     
}

void loop() {
  if (esp8266.available()){
    if (esp8266.find("+IPD,6:")) {
      delay(10);                          // make sure that all chars were received...
      for (int i = 0; i < cNumChan; i++) {    
        int Command = esp8266.read();
        if (Command == 255) { 
          int chanNo = esp8266.read();
          int servoPos = esp8266.read();
          debug("Command = " + String(Command)+ " Channel: " + String(chanNo)+ " ServoPos: " + String(servoPos));
          servoPos = map(servoPos,0,254,45,135);  
          if (chanNo == 0) { 
            myChan1.write(servoPos);   // set to position
          } else {
            myChan2.write(servoPos);   // write actual position       
          }
        }
      }
    }
  }
}

//-----------------------------------------Config ESP8266------------------------------------

boolean configAP()
{
  boolean succes = true;

  succes &= (sendCom("AT+CWMODE=2", "OK"));
  succes &= (sendCom("AT+CWSAP=\"NanoESP\",\"\",5,0", "OK"));

  return succes;
}

boolean configUDP()
{
  boolean succes = true;

  succes &= (sendCom("AT+CIPMODE=0", "OK"));
  succes &= (sendCom("AT+CIPMUX=0", "OK"));
  succes &= sendCom("AT+CIPSTART=\"UDP\",\"192.168.4.255\",90,91", "OK"); //UDP Bidirectional and Broadcast
  return succes;
}

//-----------------------------------------------Controll ESP-----------------------------------------------------

boolean sendUDP(String Msg)
{
  boolean succes = true;

  succes &= sendCom("AT+CIPSEND=" + String(Msg.length() + 2), ">");    //+",\"192.168.4.2\",90", ">");
  if (succes)
  {
    succes &= sendCom(Msg, "OK");
  }
  return succes;
}

boolean sendCom(String command, char respond[])
{
  esp8266.println(command);
  if (esp8266.findUntil(respond, "ERROR"))
  {
    return true;
  }
  else
  {
    debug("ESP SEND ERROR: " + command);
    return false;
  }
}

String sendCom(String command)
{
  esp8266.println(command);
  return esp8266.readString();
}

//-------------------------------------------------Debug Functions------------------------------------------------------
void serialDebug() {
  while (true)
  {
    if (esp8266.available())
      Serial.write(esp8266.read());
    if (Serial.available())
      esp8266.write(Serial.read());
  }
}

void debug(String Msg)
{
  if (DEBUG)
  {
    Serial.println(Msg);
  }
}
