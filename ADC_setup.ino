//Set up and configeration for the ADC
//v0.3
//Tianqi, 10-29-2015

//Continued by Mohan, Hamza, Sachin, 1-Feb-2017


//can bus command for set up & configer
#include <SPI.h>
#include "mcp_can.h"

const INT8U frequency=CAN_500KBPS;
const int SPI_CS_PIN = 53;
const int dly=100;
const int test=1000;
const int halt=10000000000;

//Min and max decimal accelerator pedal values
const int MinPedal = 0;
const int MaxPedal = 105;

// Variable Declaration
int sum = 0;


//Can cmd
unsigned char ini[2] = {01, 01};
unsigned char wCom2[8] = {47, 01, 24, 02, 255, 00, 00, 00};
unsigned char wCom3[8] = {47, 02, 24, 02, 01, 00, 00, 00};
unsigned char wHBeat[8] = {43, 23, 16, 00, 00, 00, 00, 00};
unsigned char wGItru[8] = {47, 35, 100, 00, 01, 00, 00, 00};

//Parameter of Seneor
//Data collected by testing sensors
unsigned int te[3][2]={{11200,24000},{11200,24000},{11200,24000}};
unsigned int channel[2]={0,3};

//Global Value
unsigned char len = 0;
unsigned char buf[8];
unsigned int canId;

MCP_CAN CAN(SPI_CS_PIN); 

void setup() {
  Serial.begin(115200);
  check();
  configure();
}

void check() {
 START_INIT:
  if(CAN_OK == CAN.begin(frequency))                   // init can bus : baudrate = 500k
  {
    Serial.println("CAN BUS Shield init ok! ");
  }
  else
  {
    Serial.println("CAN BUS Shield initialization failed! ");
    Serial.println("CAN BUS Shield re-initializing... ");
    delay(dly);
    goto START_INIT;
   }
}
void configure() {
  Serial.println("========================");
  Serial.print("Init... ");
  CAN.sendMsgBuf(0x00, 0, 2,ini);
  Serial.println("OK");
  delay(test); 
    
  Serial.print("Config 1801... ");
  CAN.sendMsgBuf(0x601, 0, 8, wCom2);
  Serial.println("OK");
  delay(test); 

  Serial.print("Config 1802... ");
  CAN.sendMsgBuf(0x601, 0, 8, wCom3);
  Serial.println("OK");
  delay(test); 
  
  Serial.print("Heart Beat... ");
  CAN.sendMsgBuf(0x601, 0, 8, wHBeat);
  Serial.println("OK");
  delay(test); 
    
  Serial.print("Message... ");
  CAN.sendMsgBuf(0x601, 0, 8, wGItru);
  Serial.println("OK");
  delay(dly); 
  Serial.println("========================");
}

void loop() {
    int result[3];
    rec();
    
    if (canId == 641)
    {
      for(int i=channel[0]; i<channel[1]; i++)
      {
        result[i] = ConvertToPercentage(i);
        //For every read, 1)evaluate to percentage value.
        //2) convert percentage value to motor controller data.
        //3) output data to motor controller.
        Serial.print(result[i]);
        Serial.print(" ");
      }
      int alpha = CalculateOutputSignal(result, 3);
      Serial.print("  ");
      Serial.print(alpha);
    }
    Serial.println();
    delay(dly);
}

void rec() {
    if(CAN_MSGAVAIL == CAN.checkReceive())
    {
      CAN.readMsgBuf(&len, buf);
      canId = CAN.getCanId();
    }
}

//Convert data from ADC to decimal values.
int ConvertToPercentage(int n){
    int i = 2 * n;
    int dec = buf[i+1] * 256 + buf[i];
    int delta = te[n][1] - te[n][0] + 5;
    double dbl = (double)(dec - te[n][0]) / delta;
    double result = 100 * dbl;

    //if the data goes over the maxium acceptable value (acquired from experiments and pre-race calibration), return -1, the error value.
    if( result > MaxPedal) return (-1);

    int returnvalue =  (int)(result * 100 / MaxPedal);
    if(returnvalue <= 5) returnvalue = 0;
    /*
    Serial.print(n);
    Serial.print(" ");
    Serial.print(buf[i+1]);
    Serial.print(" ");
    Serial.print(buf[i]);
    Serial.print(" ");
    Serial.print(dec);
    Serial.print(" ");
    Serial.print(te[n][0]);
    Serial.print(" ");
    Serial.print(te[n][1]);
    Serial.print(" ");
    Serial.println(dbl);
    */
    return returnvalue;
    }


    
  int CalculateOutputSignal(int x[], int xsize)
  {
    int returnvalue = 0;   
    int sum = 0;
    int count = 0;
    for (int i = 0; i < xsize; i ++)
    {
      if ((x[i] >= 0) && (x[i] < 100))
      {
        count++;
        sum += x[i];
      }
    }
    if (count <= 1) { return 0; }
    else
    {
      returnvalue = sum / count;
    }
    return returnvalue;
  }
