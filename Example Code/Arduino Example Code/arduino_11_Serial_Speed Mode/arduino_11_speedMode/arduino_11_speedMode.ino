/**************** MKS SERVOxxD Close Loop Step Motor ***************
******************** arduino Example 11 ********************
**	Example Name：Motor speed mode operation
**  Example Purpose：Control the motor to run in speed mode through the serial port
**  Example Phenomena：After the program runs, it can be observed that
** 1. The LED light turns on/off once, the motor changes speed once, and the cycle runs continuously
** 2. If the operation fails, the LED light will flash quickly
**  Precautions：
** 1. The serial port and the USB download port share the serial port (0,1). When uploading the program via USB, unplug the serial cable first to avoid program upload failure
** 2. Set the motor working mode to SR_vFOC
** 3. If the program upload fails, you can try: press and hold the RESET button of UNO, and then click upload, and when the arduino displays "uploading", quickly release the button
**********************************************************/

uint8_t txBuffer[20];      //Sends data array
uint8_t rxBuffer[20];      //Receive data array
uint8_t rxCnt=0;          //Receive data count

uint16_t runSpeed=100;    //motor running speed
uint8_t runDir  = 1;      //Motor running direction

uint8_t getCheckSum(uint8_t *buffer,uint8_t len);
void speedModeRun(uint8_t slaveAddr,uint8_t dir,uint16_t speed,uint8_t acc);
uint8_t waitingForACK(uint8_t len);

void setup() {
  // Set the LED light port as output
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  // Start the serial port, set the rate to 38400
  Serial.begin(38400);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  delay(3000);
}

void loop() {

  uint8_t ackStatus;
  
  speedModeRun(1,runDir,runSpeed,2); //Slave address=1, acceleration=2
  digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN)^1);
  ackStatus = waitingForACK(5);      //Wait for the motor to answer

  if(ackStatus == 1)        //run successfully
  {
    runSpeed += 100;        //Speed increased by 100RPM
    if(runSpeed > 300)     //change speed and direction
    {
      runSpeed = 0;         //speed set to 0
      runDir ^= 1;          //change direction
    }
  }
  else                      //failed to run
  {
    while(1)                //The flashing light indicates that the operation failed
    {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(200);
      digitalWrite(LED_BUILTIN, LOW);
      delay(200);
    }
  }

  delay(3000);    //Delay 3000 milliseconds
}

/*
Function: Serial port sends speed mode operation command
Input: slaveAddr slave address
       dir running direction
       speed running speed
       acc acceleration
*/
void speedModeRun(uint8_t slaveAddr,uint8_t dir,uint16_t speed,uint8_t acc)
{
  int i;
  uint16_t checkSum = 0;

  txBuffer[0] = 0xFA;       //frame header
  txBuffer[1] = slaveAddr;  //slave address
  txBuffer[2] = 0xF6;       //function code
  txBuffer[3] = (dir<<7) | ((speed>>8)&0x0F); //High 4 bits for direction and speed
  txBuffer[4] = speed&0x00FF;   //8 bits lower
  txBuffer[5] = acc;            //acceleration
  txBuffer[6] = getCheckSum(txBuffer,6);  //Calculate checksum

  Serial.write(txBuffer,7);
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
Input: len Length of the response frame
output:
   Run successfully 1
   failed to run 0
   timeout no reply 0
*/
uint8_t waitingForACK(uint8_t len)
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

    if(rxCnt == len)    //Receive complete
    {
      if(rxBuffer[len-1] == getCheckSum(rxBuffer,len-1))
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
    if((time - sTime) > 3000)   //Judging whether to time out
    {
      retVal = 0;
      break;                    //timeout, exit while(1)
    }
  }
  return(retVal);
}
