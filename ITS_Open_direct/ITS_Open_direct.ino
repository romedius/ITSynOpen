/*

Arduino --> ThingTweet via Ethernet
The ThingTweet sketch is designed for the Arduino + Ethernet Shield.
This sketch updates a Twitter status via the ThingTweet App
(http://community.thingspeak.com/documentation/apps/thingtweet/) using HTTP POST.
ThingTweet is a Twitter proxy web application that handles the OAuth.
Getting Started with ThingSpeak and ThingTweet:
* Sign Up for a New User Account for ThingSpeak - https://www.thingspeak.com/users/new
* Link your Twitter account to the ThingTweet App - Apps / ThingTweet
* Enter the ThingTweet API Key in this sketch under "ThingSpeak Settings"
Arduino Requirements:
* Arduino with Ethernet Shield or Arduino Ethernet
* Arduino 1.0 IDE
* Twitter account linked to your ThingSpeak account
Network Requirements:

* Ethernet port on Router
* DHCP enabled on Router
* Unique MAC Address for Arduino
Created: October 17, 2011 by Hans Scharler (http://www.iamshadowlord.com)
Updated: December 7, 2012 by Hans Scharler (http://www.iamshadowlord.com)
Additional Credits:
Example sketches from Arduino team, Ethernet by Adrian McEwen
*/

#include <SPI.h>
#include <Ethernet.h>

#include "APIKey.h"

// Local Network Settings
//byte mac[] = { 0xD4, 0x28, 0xB2, 0xFF, 0xA0, 0xA1 }; // Must be unique on local network

byte mac[] = { 0xD4, 0xBE, 0xD9, 0x9A, 0x7C, 0x95 }; // Must be unique on local network


// Initialize Arduino Ethernet Client
EthernetClient client;

// Twitter response variables
char serverName[] = "it-syndikat.org";  // twitter URL
String currentLine = "";            // string to hold the text from server

// Specific variables
// Status
int hsopen =2;
int ethernetstatus = -1;

//debug options
boolean debug = true;

//LED and Switch pins
const int ropen=5;
const int rwait=6;
const int rclosed=7;
const int runknown=8;

const int eok=A0;
const int efail=A1;
const int eunknown=A2;

const int topen = 2;
const int tclose = 3;

// Status LED pins

void setup()
{
  
  pinMode(ropen, OUTPUT);      
  pinMode(rwait, OUTPUT);      
  pinMode(rclosed, OUTPUT);      
  pinMode(runknown, OUTPUT);      

  pinMode(eok, OUTPUT);      
  pinMode(efail, OUTPUT);      
  pinMode(eunknown, OUTPUT); 

  // Taster
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
  setRoom(hsopen); 
  startEthernet();

  delay(1000);
  
  // Update Twitter via ThingTweet
  TriggerServerReq();
}

void loop()
{
  readButtons();
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
  delay(1000);
}

void setEth(int statuss){
  if(debug){
    Serial.println("Setting eth ");
    Serial.println(statuss);
  }
  digitalWrite(eok,((statuss==1)?HIGH:LOW));
  digitalWrite(efail,((statuss==0)?HIGH:LOW));
  digitalWrite(eunknown,((statuss==-1)?HIGH:LOW));
}

void setRoom(int statuss){
  if(debug){
    Serial.print("Setting room ");
    Serial.println(statuss);
  }
  digitalWrite(ropen,((statuss==1)?HIGH:LOW));
  digitalWrite(rwait,((statuss==2)?HIGH:LOW));
  digitalWrite(rclosed,((statuss==0)?HIGH:LOW));
  digitalWrite(runknown,((statuss==-1)?HIGH:LOW));
}


void TriggerServerReq() {
  // attempt to connect, and wait a millisecond:
  if(debug){Serial.println("connecting to server... Status req");}
  if (client.connect(serverName, 80)) {
    if(debug){Serial.println("making HTTP request...");}
    // make HTTP GET request to server:
    client.println("GET /status-s.php HTTP/1.1");
    client.println("HOST: it-syndikat.org");
    client.println("Connection: close");
    client.println();
    readServerStatus();
    //readServerReturn(); 
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
    client.println("GET /update.php?open=" + s + "&apikey="+serverAPIKey+" HTTP/1.1");
    client.println("HOST: it-syndikat.org");
    client.println("Connection: close");
    client.println();
    readServerStatus();    
  }else{
    if(debug){Serial.println("Not connected...");}
  }
  // note the time of this connect attempt:
}

//reads out the status returned by the server and sets the LED's appropriately.
int readServerStatus() {
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
        // if you got a "<" character,
        // you've reached the end of the tweet:
        if(currentLine.startsWith("true", 0)){
          if(debug){Serial.println("");}
          hsopen=1;
          setRoom(hsopen);
          // close the connection to the server:
          client.stop();
          return 0;
        }
        if(currentLine.startsWith("false", 0)){
          if(debug){Serial.println("");}
          hsopen=0;
          setRoom(hsopen);
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
  hsopen=-1;
  setRoom(hsopen);
  client.stop();
}


//This just reads the server return and logs it to the serial
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

