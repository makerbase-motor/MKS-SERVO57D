/**************** MKS SERVOxxD Close Loop Step Motor***************
******************** Arduino Example 2 ********************
**	Instance Name：Read motor real-time position information
**  Instance Purpose：Read the real-time position information of the motor through the serial port
**  Instance Phenomenon：After the program runs, it can be observed that：
** 1. The LED light flashes once per second, that is, the serial port sends a command to read the real-time position;
** 2. The serial port monitor can be observed to output a position value value = xxxxx every second;
** 3. Turn the motor shaft by hand, and you can see the position value change (increase or decrease).
**  Precautions：
** 1. The serial port and the USB download port share the serial port (0,1). When uploading the program via USB, unplug the serial port cable first to avoid program upload failure;
** 2. Set the motor working mode to CR_vFOC, do not enable the motor (the motor shaft can be rotated by hand);
** 3. After the program is downloaded, open the serial monitor to observe the output results (Tools->Serial monitor, select 38400 baud rate);
** 4. Every time the motor shaft rotates one circle (360 degrees), the position value changes (addition/subtraction) by 16384;
** 5. If the program upload fails, you can try: press and hold the RESET button of UNO, and then click upload, and when the arduino displays "uploading", quickly release the button;
**********************************************************/
 
uint8_t txBuffer[20];      //Send data array
uint8_t rxBuffer[20];      //Receive data array
uint8_t rxCnt=0;          //Receive data count

uint8_t getCheckSum(uint8_t *buffer,uint8_t len);
void readRealTimeLocation(uint8_t slaveAddr);
bool waitingForACK(uint8_t len);
void printToMonitor(uint8_t *value);

void setup() {
  // Set the LED light port as output
  pinMode(LED_BUILTIN, OUTPUT);
  // Start the serial port, set the rate to 38400
  Serial.begin(38400);
  //Wait for the serial port initialization to complete
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  // The power-on delay is 3000 milliseconds, waiting for the motor to be initialized
  delay(3000);
}

void loop() {

  bool ackStatus;
  
  digitalWrite(LED_BUILTIN, HIGH); //light up
  readRealTimeLocation(1); //Slave address = 1, issue a query position information command

  ackStatus = waitingForACK(10);      //Wait for the motor to answer

  if(ackStatus == true)        //Received location information
  {
    printToMonitor(&rxBuffer[5]); // The lower 32 bits output the real-time position value to the serial monitor
    digitalWrite(LED_BUILTIN, LOW); //Lights off
    
  }
  else                      //Failed to receive location information (1. Check the connection of the serial cable; 2. Check whether the motor is powered on; 3. Check the slave address and baud rate)
  {
    while(1)                //Flashing light quickly, indicating that the operation failed
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
Function: read real-time location information
Input: slaveAddr slave address
output: none
 */
void readRealTimeLocation(uint8_t slaveAddr)
{
 
  txBuffer[0] = 0xFA;       //frame header
  txBuffer[1] = slaveAddr;  //slave address
  txBuffer[2] = 0x31;       //function code
  txBuffer[3] = getCheckSum(txBuffer,3);  //Calculate checksum
  Serial.write(txBuffer,4);   //The serial port issues a command to read the real-time position
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
Function: wait for the slave to answer, set the timeout to 3000ms
Input: len Length of the response frame
output:
   run successfully true
   failed to run false
   timeout no response false
*/
bool waitingForACK(uint8_t len)
{
  bool retVal;       //return value
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
        retVal = true;   //checksum correct
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
      retVal = false;
      break;                    //timeout, exit while(1)
    }
  }
  return(retVal);
}

/*
Function: Output the lower 32-bit result of the real-time position to the serial monitor
       1. Tools -> Serial Monitor
       2. Baud rate selection 38400
       3. It can be observed that a position value is output every second value = xxxxx
       4. Turn the motor shaft by hand, you can see the position value change, the motor shaft rotates a circle, the position value changes (increase or decrease) 16384
Input: *value The lower 32-bit starting address of the position value
output: none
*/
void printToMonitor(uint8_t *value)
{
  int32_t iValue;
  String  tStr;
  iValue = (int32_t)(
                      ((uint32_t)value[0] << 24)    |
                      ((uint32_t)value[1] << 16)    |
                      ((uint32_t)value[2] << 8)     |
                      ((uint32_t)value[3] << 0)
                    );

  
  tStr = String(iValue);
  Serial.print("  location = ");
  Serial.println(tStr);
/*
  int32_t iValue;
  iValue = value[0]<<24 | value[1]<<16 | value[2]<<8 | value[3]<<0;
  
  Serial.print("value = ");
  Serial.println(iValue);
*/

}
