//Code for Arduino on board the E-kart with a CAN-BUS Shield. For COM10
#include <mcp_can.h>
#include <SPI.h>
#define MAX_SIZE 64

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 10;

volatile int flagRecv = 0;
int canidint;
unsigned char buf[8];
byte datastr[10];
unsigned char len = 0;

struct CAN_PACKET {// 2 Bytes per value
  int steeringAngle, current, aSpeed, temp, voltage;
} canPacket;


MCP_CAN CAN(SPI_CS_PIN); // Set CS pin

void setup()
{
  Serial.begin(57600);

  while (CAN_OK != CAN.begin(CAN_500KBPS)) {;} // init can bus : baudrate = 500k
  attachInterrupt(0, MCP2515_ISR, FALLING); //Interrupt service for CAN messages

  canPacket.steeringAngle = 0;
  canPacket.current = 0;
  canPacket.aSpeed = 0;
  canPacket.temp = 0;
  canPacket.voltage = 0;
  
  delay(5000); //5 sec delay for start up of e kart, then start sending and reading values 
  pinMode(7,OUTPUT);
  digitalWrite(7,HIGH);//ready to send & receive on the bus

  
}
//Defining CANopen packets 
unsigned char stmpRMS[8] = {0x40, 0x70, 0x20, 0x16, 0x00, 0x00, 0x00, 0x00};//RMS motor current frame request to OD
unsigned char stmpDC[8] = {0x40, 0x30, 0x20, 0x15, 0x00, 0x00, 0x00, 0x00};// DC Bus voltage nominal request frame to OD

void MCP2515_ISR() {
  flagRecv = 1;//flag true
}

void loop()
{
  sendRequest();//send SDO request to OD in node 5 (left motor contoller)
  readBus();//iterate over messages that I´m interested
  delay(50);// wait to read the requested data 50 miliseconds
  printBus(sizeof(canPacket),&canPacket);//send data through serial
}

void sendRequest() {//sending CANopen packets
  CAN.sendMsgBuf(0x605, 0, 8, stmpRMS);//read request RMS motor current left, left motor controller; SDO channel 1
  CAN.sendMsgBuf(0x625, 0, 8, stmpDC);//read request DC bus nominal, left motor controller; SDO channel 2
}

void readBus() {
  if (flagRecv) {
    flagRecv = 0;//clear flag
    while (CAN_MSGAVAIL == CAN.checkReceive()) {
      CAN.readMsgBuf(&len, buf);

      canidint = int(CAN.getCanId());//cast to int 

      switch (canidint) {
        case 1413: { //0x585 response from OD on node 5
            datastr[0] = buf[5];// RMS torque current 16 bits intel format
            datastr[1] = buf[4];// RMS
            break;
          }
        case 389: { //0x185 motor and temp
            datastr[2] = buf[3];//actual speed 2 Bytes
            datastr[3] = buf[2];
            datastr[4] = buf[5];// motor temp 2 Bytes
            datastr[5] = buf[4];
            break;
          }
        case 192:{//0xC0 steering angle intel
            datastr[6] = buf[1];
            datastr[7] = buf[0];
            break;
        }
        case 1445:{//0x5A5 DC bus nominal
            datastr[8] = buf[5];
            datastr[9] = buf[4];
            break;
        }
      }
    }
  }
}

void printBus(unsigned int objSize, void *ptr) {
  //(highByte << 8) | (lowByte)
  canPacket.current = (datastr[0] << 8) | (datastr[1] & 0xFF);  // (in ARMS) signed int 
  canPacket.aSpeed = (datastr[2] << 8) | (datastr[3] & 0xFF);//signed int (in RPM)
  canPacket.temp = (datastr[4] << 8) | (datastr[5] & 0xFF);//signed int (in celsius)
  canPacket.steeringAngle = (datastr[6] << 8) | (datastr[7] & 0xFF);//signed int (Raw value)
  canPacket.voltage = (datastr[8] << 8) | datastr[9];//unsigned int (in centiV)

  if(MAX_SIZE >= objSize){//make sure we don't send more bytes than limit
    byte * b = (byte *)ptr;//
    for(unsigned int i = 0; i<objSize; i++){
      Serial.write(b[i]);//Serial print raw
    }
  }
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
