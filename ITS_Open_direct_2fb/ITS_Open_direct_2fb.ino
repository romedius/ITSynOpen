/*
IT-Syndikat Open device Firmware for the wooden Device with two fat buttons

Network Requirements:

* Ethernet port on Router
* DHCP enabled on Router
* Unique MAC Address for Arduino

Additional Credits:
Example sketches from Arduino team, Ethernet by Adrian McEwen
Based on the Ethernet to Thingspeak exaple  by Hans Scharler
*/

#include <SPI.h>
#include <Ethernet.h>

#include "APIKey.h"
#include "TimerOne.h"

byte mac[] = { 0xD4, 0xBA, 0xD9, 0x9A, 0x7C, 0x95 }; // Must be unique on local network


// Initialize Arduino Ethernet Client
EthernetClient client;

// Twitter response variables
char serverName[] = "it-syndikat.org";  // URL
String currentLine = "";            // string to hold the text from server

// Specific variables
// Status
int hsopen;
int ethernetstatus;

int ledtimer =0;
const int ledspan =100;

int pingtimer =-1;
const int pingspan =250;

int checktimer = 0;
const int checkspan =1000;

int update = 0;

//debug options
boolean debug = true;

//LED and Switch pins
const int glight = 5;
const int rlight = 6;

const int topen = 2;
const int tclose = 3;

// Status LED pins

void setup()
{
  
  pinMode(glight, OUTPUT);      
  pinMode(rlight, OUTPUT);      
  
  // Buttons with pullup resistor
  pinMode(topen, INPUT);      
  pinMode(tclose, INPUT);      
  digitalWrite(topen,HIGH);
  digitalWrite(tclose,HIGH);

  // Start Serial for debugging on the Serial Monitor
  if(debug){
    Serial.begin(9600);
  }
  // Start Ethernet on Arduino
  setEth(-1);
  setRoom(2); 
  
  Timer1.initialize(20000); // 50 mal die Sekunde
  Timer1.attachInterrupt(setLeds);
  
  startEthernet();

  delay(1000);
  
  // Get current Status
  RequestState();
}

void loop()
{
  readButtons();
  launchUpdate();
}


void readButtons(){
  int ret=0;
  if((digitalRead(topen)==LOW)&&(hsopen!=1)){
    //startEthernet();
    setRoom(2);
    TriggerServerUpdate(true);
  }
  if((digitalRead(tclose)==LOW)&&(hsopen!=0)){
    //startEthernet();
    setRoom(2);
    TriggerServerUpdate(false);
  }
}

void launchUpdate(){
  if(update==1){
    RequestPing();
    RequestState();
    update = 0;
  }
}

void startEthernet()
{
  client.stop();
  if(debug){
    Serial.println("Connecting Arduino to network...");
    Serial.println();
  }
  setEth(-1);
  delay(1000);
  
  // Connect to network amd obtain an IP address using DHCP
  if (Ethernet.begin(mac) == 0)
  {
    if(debug){
      Serial.println("DHCP Failed, reset Arduino to try again");
      Serial.println();
    }
    setEth(0);
  }
  else
  {
    if(debug){
      Serial.println("Arduino connected to network using DHCP");
      Serial.println();
    }
    setEth(1);
  }  
  if(debug){
    Serial.println("DONE");
    Serial.println();
  }
  delay(1000);
}

void setLeds(){
  if (pingtimer>=0)
  {
    // blink like crazy
    if((pingtimer/2)%2 ==1){
      analogWrite(rlight, 255);
      analogWrite(glight, 255);
    }else{
      analogWrite(rlight, 0);
      analogWrite(glight, 0);
    }
    pingtimer--;
  }else{
    switch(ethernetstatus){
      case 0:{ // ethernet fail
        //blink red
        if(ledtimer < (ledspan/2)){
          analogWrite(rlight, 255);
        }else{
          analogWrite(rlight, 0);
        }
        analogWrite(glight, 0);
        break;
      }
      case -1:{ // ethernet unknown
        // fade green
        if(ledtimer < (ledspan/2)){
          analogWrite(glight, (255/(ledspan/2)*ledtimer));//fade up
        }else{
          analogWrite(glight, 255-(255/(ledspan/2)*(ledtimer-(ledspan/2))));//fade down
        }    
        analogWrite(rlight, 0);
        break;
      }
      case 1:{ // ethernet ok
        switch(hsopen){
          case 0:{ // closed
            //red
            analogWrite(rlight, 255);
            analogWrite(glight, 0);
            break;
          }
          case 1:{ // open
            // green
            analogWrite(rlight, 0);
            analogWrite(glight, 255);
            break;
          }
          case 2:{ // wait
            // fade red and green
            if(ledtimer < (ledspan/2)){
              int tmp=(255/(ledspan/2)*ledtimer); 
              analogWrite(glight, tmp);//fade up
              analogWrite(rlight, tmp);//fade up
              
            }else{
              int tmp = 255-(255/(ledspan/2)*(ledtimer-(ledspan/2)));
              analogWrite(glight, tmp);//fade down
              analogWrite(rlight, tmp);//fade down
            }
            break;
          }
          case -1:{ // unknown 
            // fade red
            if(ledtimer < (ledspan/2)){
              analogWrite(rlight, (255/(ledspan/2)*ledtimer));//fade up
            }else{
              analogWrite(rlight, 255-(255/(ledspan/2)*(ledtimer-(ledspan/2))));//fade down
            }    
            analogWrite(glight, 0);      
            break;
          }
        }      
        break;
      }
    }
    ledtimer++;
    if ( ledtimer >= ledspan ) {
      ledtimer = 0;
    }
  }
    checktimer++;
    if ( checktimer >= checkspan ) {
      checktimer = 0;
      update=1;
    }  
}

void setEth(int statuss){
  ethernetstatus = statuss;
  if(debug){
    Serial.println("Setting eth ");
    Serial.println(statuss);
  }
  setLeds();
}

void setRoom(int statuss){
  hsopen= statuss;
  if(debug){
    Serial.print("Setting room ");
    Serial.println(statuss);
  }
  setLeds();
}


void RequestState() {
  TriggerServerReq("/api/status-s.php",0);
}


void RequestPing() {
  TriggerServerReq("/api/ping-get.php?apikey="+pingAPIKey,1);
}

void TriggerServerReq(String s, int mode) {
  // attempt to connect, and wait a millisecond:
  if(debug){Serial.println("connecting to server... Status req");}
  if (client.connect(serverName, 80)) {
    if(debug){Serial.println("making HTTP request...");}
    // make HTTP GET request to server:
    client.println("GET "+ s +" HTTP/1.1");
    client.println("HOST: it-syndikat.org");
    client.println("Connection: close");
    client.println();
    readServerStatus(mode);
  }else{
    if(debug){Serial.println("Not connected...");}
  }
  // note the time of this connect attempt:
}

void TriggerServerUpdate(boolean stat) {
  // attempt to connect, and wait a millisecond:
  if(debug){Serial.println("connecting to server... Update Req");}
  if (client.connect(serverName, 80)) {
    if(debug){Serial.println("making HTTP request...");}
    // make HTTP GET request to server:
    String s =(stat?"true":"false");
    client.println("GET /api/update.php?open=" + s + "&apikey="+serverAPIKey+" HTTP/1.1");
    client.println("HOST: it-syndikat.org");
    client.println("Connection: close");
    client.println();
    readServerStatus(0);    
  }else{
    if(debug){Serial.println("Not connected...");}
  }
  // note the time of this connect attempt:
}

//reads out the status returned by the server and sets the LED's appropriately.
//the mode is defied by the intended call: 
int readServerStatus(int mode) {
  char lastsign='0';
  boolean readStatus = false;
  while(client.connected()) {
    if (client.available()) {
      // read incoming bytes:
      char inChar = client.read();

      // add incoming byte to end of line:
      currentLine += inChar;
      
      if(debug){Serial.print(inChar);}

      // if you get a newline, clear the line:
      if (inChar == '\n') {
        currentLine = "";
        if(lastsign == '\n'){ // /r/n /r/n is the end of a header
          readStatus= true; //start to parse the content of the line
          if(debug){Serial.println("##END OF HEADER##");}
        }
      }
      
      if (readStatus) {
        if(currentLine.startsWith("true", 0)){
          if(debug){Serial.println("");}
          switch(mode){
            case 0:
               setRoom(1);
               break;
            case 1:
               if(debug){Serial.println("Wink Wink");}
               pingtimer = pingspan;
               break;
          }
          // close the connection to the server:
          client.stop();
          return 0;
        }
        if(currentLine.startsWith("false", 0)){
          if(debug){Serial.println("");}
          switch(mode){
            case 0:
               setRoom(0);
               break;
            case 1:
               if(debug){Serial.println("No Wink");}
               break;
          }
          // close the connection to the server:
          client.stop();
          return 0;
        }                
      }
      if (inChar != '\r') {// removes /r so we dan test if the header end with two newlines, hacky but works.
        lastsign = inChar;
      }      
    }
  }
  setRoom(-1);
  client.stop();
}


//This just reads the server return and logs it to the serial
// @deprecated
void readServerReturn() {  
  //if(debug){Serial.println("readServerReturn ... ");}
   while(client.connected()) {
     if (client.available()) {
       char c = client.read();
       if(debug){Serial.print(c);}
     }
   }
   if(debug){Serial.println();}
   if(debug){Serial.println("disconnecting.");}
   client.stop();
}

