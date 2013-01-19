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

// Local Network Settings
byte mac[] = { 0xD4, 0x28, 0xB2, 0xFF, 0xA0, 0xA1 }; // Must be unique on local network

// ThingSpeak Settings
char thingSpeakAddress[] = "api.thingspeak.com";
String thingtweetAPIKey = "21I4R9UJQZJSC60Y";

// Variable Setup
long lastConnectionTime = 0;
boolean lastConnected = false;
int failedCounter = 0;

// Initialize Arduino Ethernet Client
EthernetClient client;

// Specific variables
// Status
int hsopen =-1;
int ethernetstatus = -1;

//debug options
boolean debug = false;
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
  startEthernet();
  
  delay(1000);
  
  setRoom(hsopen); 
  // Update Twitter via ThingTweet
}

void loop()
{
  if((digitalRead(topen)==LOW)&&(hsopen!=1)){
    setRoom(2);
    if(updateTwitterStatus("Hackerspace @ Freies Theater: OPENING ---")==0){
      hsopen=1;
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
    if(updateTwitterStatus("Hackerspace @ Freies Theater: CLOSING ---")==0){
      hsopen=0;
    }
    else
    {
      hsopen=-1;
    }
    delay(3000);
    setRoom(hsopen);  
  }
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
  }
    
  // Check if Arduino Ethernet needs to be restarted
  if (failedCounter > 3 ) {startEthernet();}
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
  Serial.println("Setting room ");
  Serial.println(statuss);
  }
  digitalWrite(ropen,((statuss==1)?HIGH:LOW));
  digitalWrite(rwait,((statuss==2)?HIGH:LOW));
  digitalWrite(rclosed,((statuss==0)?HIGH:LOW));
  digitalWrite(runknown,((statuss==-1)?HIGH:LOW));
}
