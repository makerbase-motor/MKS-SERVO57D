/**************** MKS SERVOxxD Close Loop Step Motor ***************
******************** arduino Example 14 ********************
**	Example Name：Position mode 3_run according to absolute coordinates
**  Example Purpose：Control the motor through the serial port to run in absolute coordinates
**  Example Phenomenon：After the program runs, it can be observed that
** 1. The motor runs to the absolute coordinate (absoluteAxis=0), and stops for 2000ms
** 2. The motor runs to the absolute coordinate (absoluteAxis=163840), and stops for 2000ms
** 3. Non-stop cycle operation
** 4. If the operation fails, the LED light will flash quickly
**  Precautions：
** 1. The serial port and the USB download port share the serial port (0,1). When uploading the program via USB, unplug the serial cable first to avoid program upload failure
** 2. Set the motor working mode to SR_vFOC
** 3. If the program upload fails, you can try: press and hold the RESET button of UNO, and then click upload, and when the arduino displays "uploading", quickly release the button
**********************************************************/

uint8_t txBuffer[20];      //send data array
uint8_t rxBuffer[20];      //Receive data array
uint8_t rxCnt=0;          //Receive data count

int32_t absoluteAxis = 163840;           //absolute coordinates 

uint8_t getCheckSum(uint8_t *buffer,uint8_t len);
void positionMode3Run(uint8_t slaveAddr,uint16_t speed,uint8_t acc,int32_t absoluteAxis);
uint8_t waitingForACK(uint32_t delayTime);

void setup() {
  // Set the LED light port as output
  pinMode(LED_BUILTIN, OUTPUT);
  // Start the serial port, set the rate to 38400
  Serial.begin(38400);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  delay(3000);
}

void loop() {

  uint8_t ackStatus;
  
  positionMode3Run(1,100,200,absoluteAxis); //Slave address=1, speed=100RPM, acceleration=200, absolute coordinates

  ackStatus = waitingForACK(3000);      //Wait for the position control to start answering
  if(ackStatus == 1)                    //Position control starts
  {
    ackStatus = waitingForACK(0);     //Wait for the position control to complete the response
    if(ackStatus == 2)                //Receipt of position control complete response
    {
      if(absoluteAxis == 0) absoluteAxis = 163840;    //Set absolute coordinates
      else absoluteAxis = 0;
      
    }
    else                        //Location complete reply not received 
    {
      while(1)                //The flashing light indicates failure
      {
        digitalWrite(LED_BUILTIN, HIGH);     delay(500);
        digitalWrite(LED_BUILTIN, LOW);      delay(500);
      }
    }
  }
  else                      //Position control failed
  {
    while(1)                //The flashing light indicates failure
    {
      digitalWrite(LED_BUILTIN, HIGH);     delay(200);
      digitalWrite(LED_BUILTIN, LOW);      delay(200);
    }
  }

  delay(2000);    //Delay 2000 milliseconds
}

/*
Function: Serial port sends position mode 3 running command
Input: slaveAddr slave address
       speed running speed
       acc acceleration
       absAxis absolute coordinates
*/
void positionMode3Run(uint8_t slaveAddr,uint16_t speed,uint8_t acc,int32_t absAxis)
{
  int i;
  uint16_t checkSum = 0;

  txBuffer[0] = 0xFA;       //frame header
  txBuffer[1] = slaveAddr;  //slave address
  txBuffer[2] = 0xF5;       //function code
  txBuffer[3] = (speed>>8)&0x00FF; //8 bit higher speed
  txBuffer[4] = speed&0x00FF;     //8 bits lower
  txBuffer[5] = acc;            //acceleration
  txBuffer[6] = (absAxis >> 24)&0xFF;  //Absolute coordinates bit31 - bit24
  txBuffer[7] = (absAxis >> 16)&0xFF;  //Absolute coordinates bit23 - bit16
  txBuffer[8] = (absAxis >> 8)&0xFF;   //Absolute coordinates bit15 - bit8
  txBuffer[9] = (absAxis >> 0)&0xFF;   //Absolute coordinates bit7 - bit0
  txBuffer[10] = getCheckSum(txBuffer,10);  //Calculate checksum

  Serial.write(txBuffer,11);
}

/*
Function: Calculate the checksum of a set of data
Input: buffer data to be verified
        size The number of data to be verified
output: checksum
*/
uint8_t getCheckSum(uint8_t *buffer,uint8_t size)
{
  uint8_t i;
  uint16_t sum=0;
  for(i=0;i<size;i++)
    {
      sum += buffer[i];  //Calculate accumulated value
    }
  return(sum&0xFF);     //return checksum
}

/*
Function: Wait for the response from the lower computer, set the timeout time to 3000ms
enter:
   delayTime waiting time (ms),
   delayTime = 0 , wait indefinitely
output:
   Position mode 2 control start 1
   Position mode 2 control completed 2
   Position mode 2 control failure 0
   timeout no reply 0
*/
uint8_t waitingForACK(uint32_t delayTime)
{
  uint8_t retVal;       //return value
  unsigned long sTime;  //timing start time
  unsigned long time;  //current moment
  uint8_t rxByte;      

  sTime = millis();    //get the current moment
  rxCnt = 0;           //Receive count value set to 0
  while(1)
  {
    if (Serial.available() > 0)     //The serial port receives data
    {
      rxByte = Serial.read();       //read 1 byte data
      if(rxCnt != 0)
      {
        rxBuffer[rxCnt++] = rxByte; //Storing data
      }
      else if(rxByte == 0xFB)       //Determine whether the frame header
      {
        rxBuffer[rxCnt++] = rxByte;   //store frame header
      }
    }

    if(rxCnt == 5)    //Receive complete
    {
      if(rxBuffer[4] == getCheckSum(rxBuffer,4))
      {
        retVal = rxBuffer[3];   //checksum correct
        break;                  //exit while(1)
      }
      else
      {
        rxCnt = 0;  //Verification error, re-receive the response
      }
    }

    time = millis();
    if((delayTime != 0) && ((time - sTime) > delayTime))   //Judging whether to time out
    {
      retVal = 0;
      break;                    //timeout, exit while(1)
    }
  }
  return(retVal);
}
