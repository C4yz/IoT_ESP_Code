/*
 * 34315 - Internet Of Things
 * House Temperature Regulator
 * Group 5
 */

#include <ESP8266WiFi.h>
#include <ThingSpeak.h>

// Define connection pins:
#define motor 4
// Fan speed
#define MAX 255
#define LOUW 127
#define OFF 0

// Replace with your network details
const char* ssid = "Nokia 3310";
const char* password = "Android2300";
WiFiClient client;

// ThingSpeak channel ID and API key
unsigned long channelID = 1710056; //your channel
const char * myWriteAPIKey = "SOR94XAST8J94V4W"; // your WRITE API key
const char * myReadAPIKey = "D5UZ9WBG9IXRLLTD"; // your READ API key
const char* server = "api.thingspeak.com";      // ThingSpeak server

const double VCC = 3.3;             // NodeMCU on board 3.3v vcc
const double adc_resolution = 1023; // 10-bit adc

const double A = 0.001129148;   // thermistor equation parameters
const double B = 0.000234125;
const double C = 0.0000000876741;
float R1 = 10000; // For voltage divider to thermistor
double Vout, Rth, temperature, temperature1, adc_value;

long desiredTemp = 0; // Desired temperature read from thingspeak
long oldDesiredTemp=0; 
int tsCount = 0;
int motorStatus=0;
void setup() {
  // Configure the pins as input or output:
  pinMode(motor, OUTPUT);
  // Begin serial communication at a baud rate of 9600:
  Serial.begin(9600);
  while(!Serial) { }
  // Connecting to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  ThingSpeak.begin(client);
  // Reading desired temperature and motor status from thingspeak
  desiredTemp=ThingSpeak.readLongField(channelID,1,myReadAPIKey);
  motorStatus = ThingSpeak.readLongField(channelID,3,myReadAPIKey);
  
}

void loop() {

  // Printing to terminal for debugging and troubleshooting
  Serial.print("Desired temperature: ");
  Serial.print(desiredTemp);
  Serial.println(" C"); 

  adc_value = analogRead(A0); // Reading the ADC value
  Vout = (adc_value * VCC) / adc_resolution; // Calculating the ADC value to voltage
  Rth = (VCC * R1 / Vout) - R1; 
  temperature = (1 / (A + (B * log(Rth)) + (C * pow((log(Rth)), 3))));  // Temperature in kelvin
  temperature = temperature - 273.15;  // Temperature in degree celsius

  // Printing to terminal for debugging and troubleshooting
  Serial.print("Temperature: "); 
  Serial.print(temperature);
  Serial.println(" C"); 

  // If the user enters a new temperature it goes into auto fan mode (motorStatus=3)
  if(desiredTemp!=oldDesiredTemp){
    motorStatus=3;
    ThingSpeak.setField(3,float(motorStatus));
    ThingSpeak.writeFields(channelID, myWriteAPIKey);
    Serial.println("Resetter til AUTO mode!!");
  }

  
  if(motorStatus == 3){

    // When in auto mode the motor drives at max if the temperature is more than 10C over the desired temp.
    if(temperature>desiredTemp){
      
     if((temperature-desiredTemp)>=10){
      analogWrite(motor,MAX);
      Serial.println("MAX");
      
    // If it is between 0 and 10 degrees over the wanted temp it drives at low speed
    
     }
   else if((temperature-desiredTemp)>=0){
    analogWrite(motor,LOUW);
    Serial.println("LOW");
    
  }
// If the desired temp is reached the motor turns off
  }
  else{
    analogWrite(motor,OFF);
    Serial.println("OFF");
  }
  }
  // If the motor is not in auto mode it gets controlled from the app
  if(motorStatus == 2){
    analogWrite(motor,MAX);
  }
  if(motorStatus == 1){
    analogWrite(motor,LOUW);
  }
  if(motorStatus == 0){
    analogWrite(motor,OFF);
  }

  oldDesiredTemp=desiredTemp;
  delay(2000);
  tsCount++;
  if(tsCount==10){  // tsCount is to delay the reading and writing to thingspeak
  ThingSpeak.begin(client);
  Serial.println("Printing to ThingSpeak!!!");
  ThingSpeak.setField(2,float(temperature));
  ThingSpeak.writeFields(channelID, myWriteAPIKey);
  desiredTemp=ThingSpeak.readLongField(channelID,1,myReadAPIKey);
  motorStatus = ThingSpeak.readLongField(channelID,3,myReadAPIKey);
  tsCount=0;
  client.stop();
  }

}
