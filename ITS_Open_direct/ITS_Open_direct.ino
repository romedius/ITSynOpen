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
//#include <EthernetUdp.h>

#include "APIKey.h"


char ITSAddress[] = "it-syndikat.org";

// Local Network Settings
byte mac[] = { 0xD4, 0x28, 0xB2, 0xFF, 0xA0, 0xA1 }; // Must be unique on local network

unsigned int localPort = 8888;      // local port to listen for UDP packets

// Variable Setup - check if these Variables are needed
long lastConnectionTime = 0;
boolean lastConnected = true;
int failedCounter = 0;

// NTP stuff
//IPAddress timeServer(132, 163, 4, 101); // time-a.timefreq.bldrdoc.gov NTP server
IPAddress timeServer(193,170,62,252); // ana austrian one
// IPAddress timeServer(132, 163, 4, 102); // time-b.timefreq.bldrdoc.gov NTP server
// IPAddress timeServer(132, 163, 4, 103); // time-c.timefreq.bldrdoc.gov NTP server

const int NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// Initialize Arduino Ethernet Client
EthernetClient client;

// Twitter response variables
char serverName[] = "it-syndikat.org";  // twitter URL
String currentLine = "";            // string to hold the text from server
String tweet = "";                  // string to hold the tweet
boolean readingTweet = false;       // if you're currently reading the tweet


// Specific variables
// Status
int hsopen =2;
int ethernetstatus = -1;
/*
0=not set
1=read thingspeak response
2=read tweet
*/
int readStatus=0;

// Twitter message, intermediate part
String tmsg = " Hackerspace at ";

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
  switch(readStatus){
    case 0:{
      readStatus = readButtons();
      break;
    }
    case 1:{
      readStatus = readServerReturn();
      if(debug){
        Serial.print("got it ... status: ");
        Serial.println(readStatus);    
      }
      break;
    }
    case 2:{
      readStatus = readServerStatus();
      break;
    }
  }
  
  // Check if Arduino Ethernet needs to be restarted

  lastConnected = client.connected();
}


int readButtons(){
  int ret=0;
  if((digitalRead(topen)==LOW)&&(hsopen!=1)){
    startEthernet();
    setRoom(2);
    TriggerServerUpdate(true);
    hsopen=1;
    ret=1;
    setRoom(hsopen);
  }
  if((digitalRead(tclose)==LOW)&&(hsopen!=0)){
    startEthernet();
    setRoom(2);
    TriggerServerUpdate(false);
    hsopen=0;
    ret=1;
    setRoom(hsopen);
  }
  return ret;
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
    client.println();
  }else{
    if(debug){Serial.println("Not connected...");}
  }
  // note the time of this connect attempt:
  readStatus=2;
}

void TriggerServerUpdate(boolean stat) {
  // attempt to connect, and wait a millisecond:
  if(debug){Serial.println("connecting to server... Update Req");}
  if (client.connect(serverName, 80)) {
    if(debug){Serial.println("making HTTP request...");}
    // make HTTP GET request to server:
    String s =(stat?"true":"false");
    client.println("GET /update.php?open=" + s + " HTTP/1.1");
    client.println("HOST: it-syndikat.org");
    client.println();
  }else{
    if(debug){Serial.println("Not connected...");}
  }
  // note the time of this connect attempt:
  readStatus=1;
}

//clean this mess up
int readServerStatus() {  
   if (client.connected()) {
    if (client.available()) {
      // read incoming bytes:
      char inChar = client.read();

      // add incoming byte to end of line:
      currentLine += inChar;

      // if you get a newline, clear the line:
      if (inChar == '\n') {
        currentLine = "";
      }
      
      // if you're currently reading the bytes of a tweet,
      // add them to the tweet String:
      if (readingTweet) {
        if (inChar != '<') {
          tweet += inChar;
        }
        else {
          // if you got a "<" character,
          // you've reached the end of the tweet:
          readingTweet = false;
          if(debug){Serial.println("Message:");}  
          if(debug){Serial.println(tweet);}
          if(debug){Serial.println(tweet.startsWith("true", 0));}          
          if(debug){Serial.println(tweet.startsWith("false", 0));}
          if(tweet.startsWith("true", 0)){hsopen=1;}
          else {if(tweet.startsWith("false", 0)){hsopen=0;}
          else {hsopen=-1;}}  
          setRoom(hsopen);
          // close the connection to the server:
          client.stop();
          return 0;
        }
      }
      // if the current line ends with <text>, it will
      // be followed by the tweet:
      if ( currentLine.endsWith("<status>")) {
        // tweet is beginning. Clear the tweet string:
        readingTweet = true;
        tweet = "";
      }
    }  
  }
  return 2;
}


//clean this mess up
int readServerReturn() {  
  //if(debug){Serial.println("readServerReturn ... ");}
   if (client.connected()) {
     if (client.available()) {
       char c = client.read();
       Serial.print(c);
       return 1;
     }
     return 1;
   }else{
     Serial.println();
     Serial.println("disconnecting.");
     client.stop();
     return 0;
  }
}

