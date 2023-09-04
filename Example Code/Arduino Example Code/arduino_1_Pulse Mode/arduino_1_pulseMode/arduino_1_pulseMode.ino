/**************** MKS SERVOxxD Close Loop Step Motor ***************
******************** arduino Example 1 ********************
**	Instance Name：Pulses Control Mode
**  Instance Purpose：arduino UNO board sends a pulse signal to control the motor to rotate forward and backward in a cycle
**  Instance Phenomenon：After the program runs, you can observe that:
** 1. The LED light is on, and the motor is turning a circle;
** 2. The LED lights off, the motor reverses a circle;
** 3. cycle like this
** Precautions：The motor working mode is set to CR_vFOC or CR_CLOSE   
** Source：
**  CSDN Blog：https://blog.csdn.net/gjy_skyblue
**  Bilibili Vedio：https://space.bilibili.com/393688975/channel/series
**  Baidu Cloud：https://pan.baidu.com/s/1BjrK9SC8pWnDoU32F8jHqA?pwd=mks2
**	QQ Group：948665794							
**********************************************************/

int EN_PIN = 4;     //Define the enable signal port
int STP_PIN = 3;    //Define the stop signal port
int DIR_PIN = 2;    //Define the diration signal port

int i;
// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
    
  pinMode(EN_PIN, OUTPUT);    //Set the enable signal port to output mode
  pinMode(STP_PIN, OUTPUT);   //Set the stop signal port to output mode
  pinMode(DIR_PIN, OUTPUT);   //Set the diration signal port to output mode

  digitalWrite(EN_PIN, HIGH);   //Set enable signal high level
  digitalWrite(STP_PIN, HIGH);  //Set stop signal high level
  digitalWrite(DIR_PIN, HIGH);  //Set diration signal high level
  
  delay(1000);                  //delay 1000ms
}

// the loop function runs over and over again forever
void loop() {
/*Output pulse signal to control the motor to rotate forward or reverse for 1 circle (16 subdivisions)
 *The higher the pulse frequency, the faster the motor speed
 *The pulse frequency is changed by the delay function delayMicroseconds()*/
    digitalWrite(EN_PIN, LOW);          //The enable signal outputs a low level, and the motor locks the shaft
    digitalWrite(DIR_PIN, LOW);         //The direction signal outputs low level, and the motor rotates in the forward or reverse direction
    digitalWrite(LED_BUILTIN, HIGH);    //Light up the LED
    for(i=0;i<3200;i++)                 //Output 3200 pulse signals
    {
      digitalWrite(STP_PIN, HIGH);      //Pulse signal output high level 
      delayMicroseconds(100);           //delay 100μs
      digitalWrite(STP_PIN, LOW);       //Pulse signal output low level
      delayMicroseconds(100);           //delay 100μs
    }
    digitalWrite(EN_PIN, HIGH);         //The enable signal outputs a high level, and the motor looses the shaft
    delay(1000);                        //delay 1000ms

/*Output pulse signal to control the motor to rotate forward or reverse for 1 circle (16 subdivisions)
 *The higher the pulse frequency, the faster the motor speed
 *The pulse frequency is changed by the delay function delayMicroseconds()*/
    digitalWrite(EN_PIN, LOW);          //The enable signal outputs a low level, and the motor locks the shaft
    digitalWrite(DIR_PIN, HIGH);        //The direction signal outputs a high level, and the motor rotates in the forward or reverse direction
    digitalWrite(LED_BUILTIN, LOW);     //Turn off the LED lights
    for(i=0;i<3200;i++)                 //Output 3200 pulse signals
    {
      digitalWrite(STP_PIN, HIGH);      //Pulse signal output high level
      delayMicroseconds(100);           //delay 100μs
      digitalWrite(STP_PIN, LOW);       //Pulse signal output low level
      delayMicroseconds(100);           //delay 100μs
    }
    digitalWrite(EN_PIN, HIGH);         //The enable signal outputs a high level, and the motor looses the shaft
    delay(1000);                        //delay 1000ms
}
