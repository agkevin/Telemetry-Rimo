//COM3 Program for receiver, takes the struct packet send, and prints the sepate values.
//This code is not really neccesary only if you want to use 2 arduinos on both ends. 
#include <SoftwareSerial.h>
#include <LargeSerial.h>//for using simple struct with arduino 
/*
 * TODO: Make sure it receives the same size of struct as send 
 */
SoftwareSerial Xbee(8,9);

struct RECEIVER_PACKET {//struct for handling each value of the packet
  int steeringAngle, current, aSpeed, temp, voltage;
}receiverPacket;

unsigned int objSize = sizeof(receiverPacket);


void setup() {
  Serial.begin(57600);//same baud rate as sender 
    while(!Serial){
    ;//wait until sucessfull serial connection between latop and atmel 
  }
  Xbee.begin(57600);//Same baud rate as serial 
}

void loop() {
  handleTelemetry(&receiverPacket);//main function 
  memset(&receiverPacket,0,objSize);
  delay(15);// give time to read values and fill struct 
}

void handleTelemetry(void *ptr){
   Xbee.listen();//listen to serial ports 8,9 
    if(Xbee.available() >= objSize){

      char data[objSize];//create temporal array 
      memset(data,0,objSize);//initializing
      Xbee.readBytes(data,objSize);//Read number of bytes 
      memcpy(ptr, data, objSize); //Copy byte into the struct

      sprintf(data,"%d,%d,%d,%d,%d",receiverPacket.current,receiverPacket.aSpeed,receiverPacket.temp,receiverPacket.steeringAngle,receiverPacket.voltage);
      Serial.println(data);
         
  }

}

