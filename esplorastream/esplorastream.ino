/*********************************************************************
 *
 * Esplora Controller sensor data streaming program
 *
 * 
 *
 * @file  espctl.ino
 * @author Greg Crawford 
 * @license Creative Commons Attribution-ShareAlike 3.0 Unported License. http://creativecommons.org/licenses/by-sa/3.0/deed.en_US
 * @see http://arduino.cc/en/Main/ArduinoBoardEsplora
 *
 */ 

#include <Esplora.h>

int slider;           // Position of slider 0-1023
int xAxis;            // Acceleration along X axis
int yAxis;            // Acceleration along Y axis
int zAxis;            // Acceleration along Z axis
int joystickX;        // Position of Joystick along X axis
int joystickY;        // Position of Joystick alony Y axis
int lightSensor;      // Light level as reported by light sensor
int microphone;       // Value of microphone sensor
int temperature;      // Temperature read by temperature sensor

char outBfr[64];      // Used to hold string of data to output

int userVariable;     // Extra value sent in data string

int counter;          // Counter for update interval
int limit;            // Counter limit for determinging when to update
int running;          // Indicator of 'running' status.

long holdMillis;      // Number of milliseconds a button is pressed


/**********************************************************/
/*
 * @brief  Sounds a brief tone on the piezo element
 */
/**********************************************************/

void ConfirmationTone() {
  Esplora.tone(5000, 20);
  delay(100);
  Esplora.tone(5000, 20);
  delay(100);
  Esplora.tone(5000, 20);
}

/*******************************************************************/
/*
 * @brief Returns the number of milliseconds a button is pressed
 *
 * @param switchId integer value indicating which button
 */
/******************************************************************/
long ButtonHold(int switchId) {
  long startMillis;
  
  startMillis = millis();      // Initial millis value
  
  // Wait until button is released
  while(Esplora.readButton(switchId) == LOW) {
    delay(1);
  }

  // Return the total # of milliseconds button was pressed  
  return  millis() - startMillis;
} 

/*******************************************************************/
/*
 * @brief Arduino setup function
 */
/*******************************************************************/
 
void setup() {
  while(!Serial);       // needed for Leonardo-based board like Esplora
  Serial.begin(57600); 
  
  counter = 0;
  limit = 1000;         // Initialize update rate to roughly once every 1,000 milliseconds
  running = 0;          // At start don't output data stream
  
  userVariable = 0;
  
  Esplora.writeRGB(255, 0, 0);   // Red LED color indicates that data data stream is not being output
}


/*******************************************************************/
/*
 * @brief Arduino main program loop
 */
/*******************************************************************/

void loop() {
  // If SWITCH_2 is pressed, then start sending data stream at intervals defined by limit
  if (Esplora.readButton(SWITCH_2) == LOW) {
    running = 1;
    // Brief tone to give some feedback that switch is pressed
    Esplora.tone(2000, 5);
  
    // If SWITCH_2 held down for more than 400 milliseconds, toggle the value of the userVariable
    if (ButtonHold(SWITCH_2) > 400) {
      if (userVariable == 1)
        userVariable = 0;
      else
        userVariable = 1;
        
       // Sound an obvious tone to indicate toggle occured
      ConfirmationTone();
    }
    
    // Set the color of the RGB led based on the value of the userVariable
    if (userVariable == 1)
      Esplora.writeRGB(0, 20, 0);
    else
      Esplora.writeRGB(0, 0, 50);
  }
  
  // If SWITCH_4 is pressed, disable sending of the data stream
  if (Esplora.readButton(SWITCH_4) == LOW) {
    running = 0;
    // Brief tone to provide feedback that switch was pressed
    Esplora.tone(2000, 5);

    holdMillis = ButtonHold(SWITCH_4);
    
    // If SWITCH_4 pressed for more than 400 milliseconds, set userVariable to 0
    if (holdMillis > 400) {
      userVariable = 0;

      // Sound an obvious tone to indicate action occured      
      ConfirmationTone();
    }
    
    // If SWITCH_4 is pressed for more than 1 second, set userVariable to 99
    if (holdMillis > 1000) {
      userVariable = 99;
      // Sound an obvious tone to indicate action occured      
      ConfirmationTone();
    }
    
    // Set the color of the RGB led based on the value of the userVariable
    if (userVariable == 1)
      Esplora.writeRGB(255, 205, 0);
    else
      Esplora.writeRGB(255, 0, 0);    
  }

  // If SWITCH_1 pressed, decrease limit by 100 (lower bound is 100)
  if (Esplora.readButton(SWITCH_1) == LOW) {
    if (limit > 100) {
      limit -= 100;
      counter = 0;  // Reset the counter to restart interval
      // Tone to indicate button was pressed
      Esplora.tone(5000, 5);
    }
    else {
      // Different tone indicates that we've reached lower bound of limit
      Esplora.tone(880, 20);
    }

    // If SWITCH_1 pressed for more than 400 milliseconds, reset limit to default of 1,000 milliseconds
    if (ButtonHold(SWITCH_1) > 400) {
      limit = 1000;
      // Sound an obvious tone to indicate action occured      
      ConfirmationTone();
    }    
  }

  // If SWITCH_3 pressed, increase limit by 500 (upper bound is 30,000)  
  if (Esplora.readButton(SWITCH_3) == LOW) {
    if (limit < 30000) {
      limit += 500;
      counter = 0;  // Reset the counter to restart interval
      // Tone to indicate button was pressed      
      Esplora.tone(5000, 5);
    }
    else {
     // Different tone indicates that we've reached upper bound of limit
      Esplora.tone(880, 20);
    }
    
    // Wait until SWITCH_3 is released
    ButtonHold(SWITCH_3);
  }

  // Count until we reach limit, or until userVariable equals 99, and then take action.
  if ((counter >= limit) || (userVariable == 99)) {      
    counter = 0;  // Reset counter
    
    // Read the various sensors
    slider = Esplora.readSlider();
    xAxis = Esplora.readAccelerometer(X_AXIS);
    yAxis = Esplora.readAccelerometer(Y_AXIS);
    zAxis = Esplora.readAccelerometer(Z_AXIS);      
    joystickX = Esplora.readJoystickX();
    joystickY = Esplora.readJoystickY();
    lightSensor = Esplora.readLightSensor();
    microphone = Esplora.readMicrophone();
    temperature = Esplora.readTemperature(DEGREES_F);
    
    // "print" the sensor data to a string.
    sprintf(outBfr, "s:%d,x:%d,y:%d,z:%d,j:%d,k:%d,l:%d,m:%d,t:%d,u:%d", slider, xAxis, yAxis, zAxis,
             joystickX, joystickY, lightSensor, microphone, temperature, userVariable);
   
/*
 *  Note if you are unfamiliar with sprintf, it is essentially doing the same as the following lines
 *  of code except that it "prints" to a string.  The resulting string is then sent to the serial
 *  port using Serial.print. 
 *
   
    Serial.print("s:");
    Serial.print(slider);
    Serial.print(",x:");
    Serial.print(xAxis);
    Serial.print(",y:");
    Serial.print(yAxis);
    Serial.print(",z:");
    Serial.print(zAxis);
    Serial.print(",j:");
    Serial.print(joystickX);
    Serial.print(",k:");
    Serial.print(joystickY);
    Serial.print(",l:");
    Serial.print(lightSensor);
    Serial.print(",m:");
    Serial.print(microphone);
    Serial.print(",t:");
    Serial.print(temperature);
    Serial.print(",u:");
    Serial.print(userVariable);
    Serial.println("");
 */
 
    // "Print" the data string to the serial port
    Serial.println(outBfr);
    delay(10);
 
    // When user variable is set to 99, it means to stop sending the data stream   
    if (userVariable == 99) {
      running = 0;
      userVariable = 0;
      counter = 0;
      limit = 1000;
      Esplora.writeRGB(255, 0, 0);
    }
    else {
      if (userVariable == 1)  // Turn RGB led to green 
        Esplora.writeRGB(0, 20, 0);
      else
        Esplora.writeRGB(0, 0, 50);  // Turn RGB led to blue
    }
  }
  
  delay(1);
  // Only increment the counter if our status is "running"
  if (running == 1)
    counter++;
}


