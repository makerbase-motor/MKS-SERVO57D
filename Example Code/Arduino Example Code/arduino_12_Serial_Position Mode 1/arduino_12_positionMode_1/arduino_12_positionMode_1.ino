/**************** MKS SERVOxxD Close Loop Step Motor ***************
******************** arduino Example 12 ********************
**	Example Name：Motor position mode 1_run by pulse number
**  Example Purpose：Control the motor to run with the number of pulses through the serial port
**  Example Phenomenon：After the program runs, it can be observed that
** 1. The motor runs to the specified position (10 circles) according to the set pulse number, and stops for 2000ms
** 2. Change the direction, then run to the specified position (10 laps), stop for 2000ms
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

uint32_t runPosition=9600;    //Motor running pulse number
uint8_t runDir  = 1;          //Motor running direction

uint8_t getCheckSum(uint8_t *buffer,uint8_t len);
void positionMode1Run(uint8_t slaveAddr,uint8_t dir,uint16_t speed,uint8_t acc,uint32_t pulses);
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
  
  positionMode1Run(1,runDir,100,200,32000); //Slave address=1, speed=100RPM, acceleration=200, pulse number=32000 (10 circles)  

  ackStatus = waitingForACK(3000);      //Wait for the position control to start answering
  if(ackStatus == 1)                    //Position control starts
  {
    ackStatus = waitingForACK(0);     //Wait for the position control to complete the response
    if(ackStatus == 2)                //Receipt of position control complete response
    {
      runDir ^= 1;               //Run in the opposite direction, back to the starting point
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
Function: Serial port sends position mode 1 running command
Input: slaveAddr slave address
       dir running direction
       speed running speed
       acc acceleration
       pulses pulse number
*/
void positionMode1Run(uint8_t slaveAddr,uint8_t dir,uint16_t speed,uint8_t acc,uint32_t pulses)
{
  int i;
  uint16_t checkSum = 0;
 
  txBuffer[0] = 0xFA;       //frame header
  txBuffer[1] = slaveAddr;  //slave address
  txBuffer[2] = 0xFD;       //function code
  txBuffer[3] = (dir<<7) | ((speed>>8)&0x0F); //High 4 bits for direction and speed
  txBuffer[4] = speed&0x00FF;   //8 bits lower
  txBuffer[5] = acc;            //acceleration
  txBuffer[6] = (pulses >> 24)&0xFF;  //Pulse bit31 - bit24
  txBuffer[7] = (pulses >> 16)&0xFF;  //Pulse bit23 - bit16
  txBuffer[8] = (pulses >> 8)&0xFF;   //Pulse bit15 - bit8
  txBuffer[9] = (pulses >> 0)&0xFF;   //Pulse bit7 - bit0
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
   Position mode 1 control start 1
   Position mode 1 control completed 2
   Position mode 1 control failure 0
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
        break;                  //Exit while(1)
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
