/**************** MKS SERVOxxD Close Loop Step Motor ***************
******************** arduino Example 6 ********************
**	Example Name：Read IO port status
**  Example Purpose：Read IO port status through serial port
**  Example phenomenon：After the program runs, it can be observed that
** 1. The LED light flashes once per second, that is, the serial port sends a command to read the number of pulses
** 2. The serial port monitor can be observed to output one IO status per second IN_1=x IN_2=x OUT_1=x OUT_2=x
** 3. Change the state of the input port (for example, the IN_1 port is grounded), and it can be observed that the state of the output IO will also change accordingly.
**  Precautions:
** 1. The serial port and the USB download port share the serial port (0,1). When uploading the program via USB, unplug the serial cable first to avoid program upload failure;
** 2. Motor working mode is set to CR_vFOC
** 3. After the program is downloaded, open the serial monitor to observe the output results (Tools->Serial monitor, select 38400 baud rate)
** 4. If the program upload fails, you can try: press and hold the RESET button of UNO, and then click upload, and when the arduino displays "uploading", quickly release the button
**********************************************************/

uint8_t txBuffer[20];      //send data array
uint8_t rxBuffer[20];      //Receive data array
uint8_t rxCnt=0;          //Receive data count

uint8_t getCheckSum(uint8_t *buffer,uint8_t len);
void readIOStatus(uint8_t slaveAddr);
bool waitingForACK(uint8_t len);
void printToMonitor(uint8_t *value);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);// Set the LED light port as output

  // Start the serial port, set the rate to 38400
  Serial.begin(38400);
  //Wait for the serial port initialization to complete
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

// Power-on delay of 5000 milliseconds, waiting for the motor to be initialized
  delay(5000);
  }

void loop() {

  bool ackStatus;
  
  digitalWrite(LED_BUILTIN, HIGH); //light up
  readIOStatus(1); //Slave address = 1, issue a command to query the status of the IO port

  ackStatus = waitingForACK(5);      //Wait for the motor to answer

  if(ackStatus == true)        //Received IO port status
  {
    printToMonitor(&rxBuffer[3]); // IO port status output to serial monitor
    digitalWrite(LED_BUILTIN, LOW); //Lights off
    
  }
  else                      //Failed to receive IO port status information (1. Check the connection of the serial cable; 2. Check whether the motor is powered on; 3. Check the slave address and baud rate)
  {
    while(1)                //Flashing light quickly, indicating that the operation failed
    {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(200);
      digitalWrite(LED_BUILTIN, LOW);
      delay(200);
    }
  }

  delay(1000);    //Delay 1000 milliseconds
}

/*
Function: read the number of input pulses
Input: slaveAddr slave address
output: none
 */
void readIOStatus(uint8_t slaveAddr)
{
 
  txBuffer[0] = 0xFA;       //frame header
  txBuffer[1] = slaveAddr;  //slave address
  txBuffer[2] = 0x34;       //function code
  txBuffer[3] = getCheckSum(txBuffer,3);  //Calculate checksum
  Serial.write(txBuffer,4);   //The serial port issues a command to read the input pulse number
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
        break;                  //Exit while(1)
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
Function: Output the IO port status results to the serial monitor
       1. Tools -> Serial Monitor
       2. Baud rate selection 38400
       3. It can be observed that one IO port status is output per second
Input: *value IO port status starting address
output: none
*/
void printToMonitor(uint8_t *value)
{
  uint8_t iValue;
  iValue = value[0]&1;   
  Serial.print("  IN_1 = ");
  Serial.print(iValue);

  iValue = (value[0]&2)>>1;   
  Serial.print("  IN_2 = ");
  Serial.print(iValue);

  iValue = (value[0]&4)>>2;   
  Serial.print("  OUT_1 = ");
  Serial.print(iValue);

  iValue = (value[0]&8)>>3;   
  Serial.print("  OUT_2 = ");
  Serial.println(iValue);
 }
