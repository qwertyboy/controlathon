#include <Wire.h>
#include <Zumo32U4.h>

// This is the maximum speed the motors will be allowed to turn.
// A maxSpeed of 400 lets the motors go at top speed.	Decrease
// this value to impose a speed limit.
const uint16_t maxSpeed = 100;

Zumo32U4Buzzer buzzer;
Zumo32U4Motors motors;
Zumo32U4ButtonA buttonA;
Zumo32U4LCD lcd;
Zumo32U4ProximitySensors proxSensors;
L3G gyro;

bool proxLeftActive;
bool proxFrontActive;
bool proxRightActive;

int16_t gyroOffset;
uint32_t gyroLastUpdate;
void gyroSetup(){
	gyro.init();

	// 800 Hz output data rate,
	// low-pass filter cutoff 100 Hz
	gyro.writeReg(L3G::CTRL1, 0b11111111);

	// 2000 dps full scale
	gyro.writeReg(L3G::CTRL4, 0b00100000);

	// High-pass filter disabled
	gyro.writeReg(L3G::CTRL5, 0b00000000);

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
	gyroOffset = total / 1024;
}

// This should be called to set the starting point for measuring
// a turn.  After calling this, turnAngle will be 0.
void turnSensorReset(){
	gyroLastUpdate = micros();
	turnAngle = 0;
}

// Displays a bar graph of sensor readings on the LCD.
// Returns after the user presses A.
void showReadings()
{
	lcd.clear();

	while(!buttonA.getSingleDebouncedPress())
	{
		lineSensors.readCalibrated(lineSensorValues);

		lcd.gotoXY(0, 0);
		for (uint8_t i = 0; i < NUM_SENSORS; i++)
		{
			uint8_t barHeight = map(lineSensorValues[i], 0, 1000, 0, 8);
			printBar(barHeight);
		}
	}
}

void setup()
{
	// Uncomment if necessary to correct motor directions:
	//motors.flipLeftMotor(true);
	//motors.flipRightMotor(true);

	lineSensors.initFiveSensors();
	proxSensors.initThreeSensors();

	loadCustomCharacters();

	// Play a little welcome song
	buzzer.play(">g32>>c32");

	// Wait for button A to be pressed and released.
	lcd.clear();
	lcd.print(F("Press A"));
	lcd.gotoXY(0, 1);
	lcd.print(F("to calib"));
	buttonA.waitForButton();

	calibrateSensors();

	showReadings();

	// Play music and wait for it to finish before we start driving.
	lcd.clear();
	lcd.print(F("Go!"));
	buzzer.play("L16 cdegreg4");
	while(buzzer.isPlaying());
}

void loop()
{
	// Get the position of the line.	Note that we *must* provide
	// the "lineSensorValues" argument to readLine() here, even
	// though we are not interested in the individual sensor
	// readings.
	int16_t position = lineSensors.readLine(lineSensorValues);

	// Our "error" is how far we are away from the center of the
	// line, which corresponds to position 2000.
	int16_t error = position - 2000;

	// Get motor speed difference using proportional and derivative
	// PID terms (the integral term is generally not very useful
	// for line following).	Here we are using a proportional
	// constant of 1/4 and a derivative constant of 6, which should
	// work decently for many Zumo motor choices.	You probably
	// want to use trial and error to tune these constants for your
	// particular Zumo and line course.
	int16_t speedDifference = error / 4 + 6 * (error - lastError);

	lastError = error;

	// Get individual motor speeds.	The sign of speedDifference
	// determines if the robot turns left or right.
	int16_t leftSpeed = (int16_t)maxSpeed + speedDifference;
	int16_t rightSpeed = (int16_t)maxSpeed - speedDifference;

	// Constrain our motor speeds to be between 0 and maxSpeed.
	// One motor will always be turning at maxSpeed, and the other
	// will be at maxSpeed-|speedDifference| if that is positive,
	// else it will be stationary.	For some applications, you
	// might want to allow the motor speed to go negative so that
	// it can spin in reverse.
	leftSpeed = constrain(leftSpeed, 0, (int16_t)maxSpeed);
	rightSpeed = constrain(rightSpeed, 0, (int16_t)maxSpeed);

	// Send IR pulses and read the proximity sensors.
	proxSensors.read();

	// Just read the proximity sensors without sending pulses.
	proxLeftActive = proxSensors.readBasicLeft();
	proxFrontActive = proxSensors.readBasicFront();
	proxRightActive = proxSensors.readBasicRight();

	int left = proxSensors.countsLeftWithLeftLeds();
	int right = proxSensors.countsRightWithRightLeds();
	int front = proxSensors.countsFrontWithLeftLeds();


	if (left >= 4 && position == 0) {
		motors.setSpeeds(100,50);
		delay(200);
	} else if (right >= 4 && position == 0){
		motors.setSpeeds(50,100);
		delay(200);
	}

	static char buffer[80];
	sprintf(buffer, "%d %d %d %d\n", left, right, front, position);
	Serial.print(buffer);

	motors.setSpeeds(leftSpeed, rightSpeed);
}
