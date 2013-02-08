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
#include <EthernetUdp.h>

#include "APIKey.h"


char thingSpeakAddress[] = "api.thingspeak.com";

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

// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

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
      readStatus = readThingspeakResponse();
      break;
    }
    case 2:{
      readStatus = readServerStatus();
      break;
    }
  }
  
  
  
  // Check if Arduino Ethernet needs to be restarted
  if (failedCounter > 3 ) {
    startEthernet();
  }
  lastConnected = client.connected();
}

int updateTwitterStatus(String tsData)
{
  if (client.connect(thingSpeakAddress, 80))
  {
    // Create HTTP POST Data
    tsData = "api_key="+thingtweetAPIKey+"&status="+tsData;
            
    client.print("POST /apps/thingtweet/1/statuses/update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(tsData.length());
    client.print("\n\n");

    client.print(tsData);
    
    lastConnectionTime = millis();
    
    if (client.connected())
    {
      if(debug){
        Serial.println("Connecting to ThingSpeak...");
        Serial.println();
      } 
      failedCounter = 0;
      return 0;
    }
    else
    {
      failedCounter++;
      if(debug){
        Serial.println("Connection to ThingSpeak failed ("+String(failedCounter, DEC)+")");
        Serial.println();
      }
      return -1;
    }
  }
  else
  {
    failedCounter++;
    if(debug){
      Serial.println("Connection to ThingSpeak Failed ("+String(failedCounter, DEC)+")");
      Serial.println();
    }
    lastConnectionTime = millis();
    return -1;
  }
}

int readButtons(){
  int ret=0;
  if((digitalRead(topen)==LOW)&&(hsopen!=1)){
    setRoom(2);
    if(updateTwitterStatus("Opening" + tmsg + read_time())==0){
      hsopen=1;
      ret=1;
    }
    else
    {
      hsopen=-1;
    }
    delay(3000);  
    setRoom(hsopen);  
  }
  if((digitalRead(tclose)==LOW)&&(hsopen!=0)){
    setRoom(2);
    if(updateTwitterStatus("Closing" + tmsg + read_time())==0){
      hsopen=0;
      ret=1;
    }
    else
    {
      hsopen=-1;
    }
    delay(3000);
    setRoom(hsopen);
  }
  return ret; 
}

int readThingspeakResponse(){
// Print Update Response to Serial Monitor
  if(client.available())
  {
    char c = client.read();
    if(debug){
      Serial.print(c);
    }
  }

  // Disconnect from ThingSpeak
  if (!client.connected() && lastConnected)
  {
    if(debug){
      Serial.println("...disconnected");
    }
    client.stop();
    return 0;
  }
  return 1;
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
    Udp.begin(localPort);
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
    Serial.println("Setting room ");
    Serial.println(statuss);
  }
  digitalWrite(ropen,((statuss==1)?HIGH:LOW));
  digitalWrite(rwait,((statuss==2)?HIGH:LOW));
  digitalWrite(rclosed,((statuss==0)?HIGH:LOW));
  digitalWrite(runknown,((statuss==-1)?HIGH:LOW));
}

//NTP Functions: Read the Time from NTP Server
String read_time(){
  String time= "";
  if(debug){
    Serial.println("readTime");
  }
  sendNTPpacket(timeServer); // send an NTP packet to a time server

    // wait to see if a reply is available
  for(int i = 50; i>0;i--){
    if(debug){
      Serial.print("iteration ");
      Serial.println(i);
    }
    delay(50);  
    if ( Udp.parsePacket() ) {  
      // We've received a packet, read the data from it
      Udp.read(packetBuffer,NTP_PACKET_SIZE);  // read the packet into the buffer
  
      //the timestamp starts at byte 40 of the received packet and is four bytes,
      // or two words, long. First, esxtract the two words:
  
      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);  
      // combine the four bytes (two words) into a long integer
      // this is NTP time (seconds since Jan 1 1900):
      unsigned long secsSince1900 = highWord << 16 | lowWord;  
      if(debug){
        Serial.print("Seconds since Jan 1 1900 = " );
        Serial.println(secsSince1900);              
    
        // now convert NTP time into everyday time:
        Serial.print("Unix time = ");
      }
      // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
      const unsigned long seventyYears = 2208988800UL;    
      // subtract seventy years:
      unsigned long epoch = secsSince1900 - seventyYears;  
      // print Unix time:
      if(debug){
        Serial.println(epoch);                              
      }   
      //time +="The UTC time is ";       // UTC is the time at Greenwich Meridian (GMT)
      time +=((epoch + 3600)  % 86400L) / 3600; // print the hour (86400 equals secs per day)
      time +=':';
               
      if ( ((epoch % 3600) / 60) < 10 ) {
        // In the first 10 minutes of each hour, we'll want a leading '0'
        time +="0";          
      }
      time +=((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
      time +=':';
      
      if ( (epoch % 60) < 10 ) {
        // In the first 10 seconds of each minute, we'll want a leading '0'
        time +="0";
      }
      time+=epoch %60;
      if(debug){Serial.println(time);}     
      return(time);
    }
  }
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:         
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer,NTP_PACKET_SIZE);
  Udp.endPacket();
}

/**
* http://arduino.cc/en/Tutorial/TwitterClient
*
*
*/
void TriggerServerReq() {
  // attempt to connect, and wait a millisecond:
  if(debug){Serial.println("connecting to server...");}
  if (client.connect(serverName, 80)) {
    if(debug){Serial.println("making HTTP request...");}
    // make HTTP GET request to server:
    client.println("GET /status-s.php HTTP/1.1");
    client.println("HOST: it-syndikat.org");
    client.println();
  }
  // note the time of this connect attempt:
  readStatus=2;
}

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
      // if the current line ends with <text>, it will
      // be followed by the tweet:
      if ( currentLine.endsWith("<status>")) {
        // tweet is beginning. Clear the tweet string:
        readingTweet = true;
        tweet = "";
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
          if(debug){Serial.println(tweet.startsWith("Opening", 1));}          
          if(debug){Serial.println(tweet.startsWith("Closing", 1));}
          if(tweet.startsWith("Opening", 1)){hsopen=1;}
          else if(tweet.startsWith("Closing", 1)){hsopen=0;}
          else {hsopen=-1;}
          setRoom(hsopen);
          // close the connection to the server:
          client.stop();
          return 0;
        }
      }
    }  
  }
  return 2;
}

