/** This example shows how to read from the five line sensors and
three proximity sensors on the Zumo32U4 Front Sensor Array and
documents the different options available for configuring those
sensors.
The sensor readings are displayed on the LCD and also printed to
the serial monitor. */

#include <Wire.h>
#include <Zumo32U4.h>
#include "TurnSensor.h"

Zumo32U4LCD lcd;
Zumo32U4LineSensors lineSensors;
Zumo32U4ProximitySensors proxSensors;
Zumo32U4Motors motors;
L3G gyro;
Zumo32U4ButtonA buttonA;


uint16_t lineSensorValues[5] = { 0, 0, 0, 0, 0 };

bool proxLeftActive;
bool proxFrontActive;
bool proxRightActive;

void setup()
{
  // The code below shows several ways to configure the line and
  // proximity sensors.  If you don't want to use confiuration 1,
  // then comment it out and uncomment one of the other
  // configurations.

  /* Configuration 1:
   * - 3 line sensors (1, 3, 5)
   * - 3 proximity sensors (left, front, right)
   *
   * For this configuration to work, jumpers on the front sensor
   * array must be installed in order to connect pin 4 to RGT and
   * connect pin 20 to LFT.  This is a good configuration for a
   * sumo robot. */
  lineSensors.initThreeSensors();
  proxSensors.initThreeSensors();

  /* Configuration 2:
   * - 5 line sensors (1, 2, 3, 4, 5)
   * - 1 proximity sensor (front)
   *
   * For this configuration to work, jumpers on the front sensor
   * array must be installed in order to connect pin 4 to DN4 and
   * pin 20 to DN2.  This is a good configuration for a line
   * follower or maze solver. */
  //lineSensors.initFiveSensors();
  //proxSensors.initFrontSensor();

  /* Configuration 3:
   * - 4 line sensors (1, 3, 4, 5)
   * - 2 proximity sensors (left, front)
   *
   * For this configuration to work, jumpers on the front sensor
   * array must be installed in order to connect pin 4 to DN4 and
   * pin 20 to LFT. */
  //uint8_t lineSensorPins[] = { SENSOR_DOWN1, SENSOR_DOWN3, SENSOR_DOWN4, SENSOR_DOWN5 };
  //lineSensors.init(lineSensorPins, sizeof(lineSensorPins));
  //uint8_t proxSensorPins[] = { SENSOR_LEFT, SENSOR_FRONT };
  //proxSensors.init(proxSensorPins, sizeof(proxSensorPins));

  /* Configuration 4:
   * - 4 line sensors (1, 2, 3, 5)
   * - 2 proximity sensors (front, right)
   *
   * For this configuration to work, jumpers on the front sensor
   * array must be installed in order to connect pin 4 to RGT and
   * pin 20 to DN2. */
  //uint8_t lineSensorPins[] = { SENSOR_DOWN1, SENSOR_DOWN2, SENSOR_DOWN3, SENSOR_DOWN5 };
  //lineSensors.init(lineSensorPins, sizeof(lineSensorPins));
  //uint8_t proxSensorPins[] = { SENSOR_FRONT, SENSOR_RIGHT };
  //proxSensors.init(proxSensorPins, sizeof(proxSensorPins));

  /* Configuration 5:
   * This is the same as configuration 1 except that all the pins
   * and parameters being used have been written explicitly, so
   * this is a good starting point if you want to do something
   * customized.
   *
   * If you use custom pins for your sensors, then the helpers
   * used in this example like countsRightWithLeftLeds() will no
   * longer know which sensor to use.  Instead, you should use
   * countsWithLeftLeds(sensorNumber) and
   * coutnsWithRightLeds(sensorNumber).
   *
   * In the code below, 2000 is timeout value (in milliseconds)
   * for the line sensors, and SENSOR_LEDON is the pin number for
   * the pin that controls the line sensor emitters. */
  //uint8_t lineSensorPins[] = { SENSOR_DOWN1, SENSOR_DOWN3, SENSOR_DOWN5 };
  //lineSensors.init(lineSensorPins, sizeof(lineSensorPins), 2000, SENSOR_LEDON);
  //uint8_t proxSensorPins[] = { SENSOR_LEFT, SENSOR_FRONT, SENSOR_RIGHT };
  //proxSensors.init(proxSensorPins, sizeof(proxSensorPins), SENSOR_LEDON);

  /* After setting up the proximity sensors with one of the
   * methods above, you can also customize their operation: */
  //proxSensors.setPeriod(420);
  //proxSensors.setPulseOnTimeUs(421);
  //proxSensors.setPulseOffTimeUs(578);
  //uint16_t levels[] = { 4, 15, 32, 55, 85, 120 };
  //proxSensors.setBrightnessLevels(levels, sizeof(levels)/2);

  loadCustomCharacters();
  findFarWall();
  calibrateLineSensors();
}

// This function calibrates the line sensors for about 10
// seconds.  During this time, you should move the robot around
// manually so that each of its line sensors sees a full black
// surface and a full white surface.  For the best calibration
// results, you should also avoid exposing the sensors to
// abnormal conditions during this time.
void calibrateLineSensors()
{
  // To indicate we are in calibration mode, turn on the yellow LED
  // and print "Line cal" on the LCD.
  ledYellow(1);
  lcd.clear();
  lcd.print(F("Line cal"));

  for (uint16_t i = 0; i < 400; i++)
  {
    lcd.gotoXY(0, 1);
    lcd.print(i);
    lineSensors.calibrate();
  }

  ledYellow(0);
  lcd.clear();
}

// This is the average reading obtained from the gyro's Z axis
// during calibration.
int16_t gyroOffsett;

// This variable helps us keep track of how much time has passed
// between readings of the gyro.
uint16_t gyroLastUpdatee = 0;

void findFarWall(){
    //turnSensorSetup();
    // calibrate gyro
    Wire.begin();
    gyro.init();

    // 800 Hz output data rate,
    // low-pass filter cutoff 100 Hz
    gyro.writeReg(L3G::CTRL1, 0b11111111);

    // 2000 dps full scale
    gyro.writeReg(L3G::CTRL4, 0b00100000);

    // High-pass filter disabled
    gyro.writeReg(L3G::CTRL5, 0b00000000);

    lcd.clear();
    lcd.print(F("Gyro cal"));

    // Turn on the yellow LED in case the LCD is not available.
    ledYellow(1);

    // Delay to give the user time to remove their finger.
    delay(500);

    // Calibrate the gyro.
    int32_t total = 0;
    for (uint16_t i = 0; i < 1024; i++)
    {
        // Wait for new data to be available, then read it.
        while(!gyro.readReg(L3G::STATUS_REG) & 0x08);
        gyro.read();

        // Add the Z axis reading to the total.
        total += gyro.g.z;
    }
    ledYellow(0);
    gyroOffsett = total / 1024;

    turnSensorReset();

    uint8_t maximized = 0;
    while(!maximized){
        motors.setSpeeds(-100, 100);

        // read sensors
        proxSensors.read();
        uint8_t leftValue = proxSensors.countsFrontWithLeftLeds();
        uint8_t rightValue = proxSensors.countsFrontWithRightLeds();

        while((int32_t)turnAngle < turnAngle45 * 2){
            turnSensorUpdate();
        }

        motors.setSpeeds(100, -100);
        while((int32_t)turnAngle > 0){
            turnSensorSetup();
        }
        motors.setSpeeds(0, 0);
        maximized = 1;
    }
}

void loadCustomCharacters()
{
  static const char levels[] PROGMEM = {
    0, 0, 0, 0, 0, 0, 0, 63, 63, 63, 63, 63, 63, 63
  };
  lcd.loadCustomCharacter(levels + 0, 0);  // 1 bar
  lcd.loadCustomCharacter(levels + 1, 1);  // 2 bars
  lcd.loadCustomCharacter(levels + 2, 2);  // 3 bars
  lcd.loadCustomCharacter(levels + 3, 3);  // 4 bars
  lcd.loadCustomCharacter(levels + 4, 4);  // 5 bars
  lcd.loadCustomCharacter(levels + 5, 5);  // 6 bars
  lcd.loadCustomCharacter(levels + 6, 6);  // 7 bars
}

void printBar(uint8_t height)
{
  if (height > 8) { height = 8; }
  const char barChars[] = {' ', 0, 1, 2, 3, 4, 5, 6, 255};
  lcd.print(barChars[height]);
}

// Prints a bar graph showing all the sensor readings on the LCD.
void printReadingsToLCD()
{
  // On the first line of the LCD, display proximity sensor
  // readings.
  lcd.gotoXY(0, 0);
  printBar(proxSensors.countsLeftWithLeftLeds());
  printBar(proxSensors.countsLeftWithRightLeds());
  lcd.print(' ');
  printBar(proxSensors.countsFrontWithLeftLeds());
  printBar(proxSensors.countsFrontWithRightLeds());
  lcd.print(' ');
  printBar(proxSensors.countsRightWithLeftLeds());
  printBar(proxSensors.countsRightWithRightLeds());

  // On the second line of the LCD, display line sensor readings.
  // These calibrated sensor readings are between 0 and 1000, so
  // we use the map function to rescale it to 0 through 8.
  lcd.gotoXY(0, 1);
  for (uint8_t i = 0; i < 5; i++)
  {
    uint8_t barHeight = map(lineSensorValues[i], 0, 1000, 0, 8);
    printBar(barHeight);
  }

  // On the last 3 characters of the second line, display basic
  // readings of the sensors taken without sending IR pulses.
  lcd.gotoXY(5, 1);
  printBar(proxLeftActive);
  printBar(proxFrontActive);
  printBar(proxRightActive);
}

// Prints a line with all the sensor readings to the serial
// monitor.
void printReadingsToSerial()
{
  static char buffer[80];
  sprintf(buffer, "%d %d %d %d %d %d  %d %d %d  %4d %4d %4d %4d %4d\n",
    proxSensors.countsLeftWithLeftLeds(),
    proxSensors.countsLeftWithRightLeds(),
    proxSensors.countsFrontWithLeftLeds(),
    proxSensors.countsFrontWithRightLeds(),
    proxSensors.countsRightWithLeftLeds(),
    proxSensors.countsRightWithRightLeds(),
    proxLeftActive,
    proxFrontActive,
    proxRightActive,
    lineSensorValues[0],
    lineSensorValues[1],
    lineSensorValues[2],
    lineSensorValues[3],
    lineSensorValues[4]
  );
  Serial.print(buffer);
}

void loop()
{
  static uint16_t lastSampleTime = 0;

  if ((uint16_t)(millis() - lastSampleTime) >= 100)
  {
    lastSampleTime = millis();

    // Send IR pulses and read the proximity sensors.
    proxSensors.read();

    // Just read the proximity sensors without sending pulses.
    proxLeftActive = proxSensors.readBasicLeft();
    proxFrontActive = proxSensors.readBasicFront();
    proxRightActive = proxSensors.readBasicRight();

    // Read the line sensors.
    lineSensors.readCalibrated(lineSensorValues);

    // Send the results to the LCD and to the serial monitor.
    printReadingsToLCD();
    printReadingsToSerial();

	drive();
  }
}

void drive(){
	int left = proxSensors.countsLeftWithLeftLeds();
	int right = proxSensors.countsRightWithRightLeds();

	motors.setSpeeds((left * 30) + 40, (right * 30) + 40);
}
