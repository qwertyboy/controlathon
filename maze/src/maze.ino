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

bool proxLeftActive;
bool proxFrontActive;
bool proxRightActive;

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
