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

int16_t leftSpeed;
int16_t rightSpeed;

void setup()
{
	proxSensors.initThreeSensors();

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
	// Send IR pulses and read the proximity sensors.
	proxSensors.read();

	// Just read the proximity sensors without sending pulses.
	proxLeftActive = proxSensors.readBasicLeft();
	proxFrontActive = proxSensors.readBasicFront();
	proxRightActive = proxSensors.readBasicRight();

	int left = proxSensors.countsLeftWithLeftLeds();
	int right = proxSensors.countsRightWithRightLeds();
	int frontLeft = proxSensors.countsFrontWithLeftLeds();
	int frontRight = proxSensors.countsFrontWithRightLeds();

	if (frontLeft > 4 && frontRight > 4) {
		if (left > 4) {
			turnLeft();
		} else if (right > 4) {
			turnRight();
		}
	}

	static char buffer[80];
	sprintf(buffer, "%d %d %d %d\n", left, right, frontLeft, frontRight);
	Serial.print(buffer);

	// determines if the robot turns left or right.
	leftSpeed = 200;
	rightSpeed = 200;

	motors.setSpeeds(leftSpeed, rightSpeed);
}

void turnLeft(){
	// determines if the robot turns left or right.
	leftSpeed = 300;
	rightSpeed = 100;

	motors.setSpeeds(leftSpeed, rightSpeed);
}

void turnRight(){
	// determines if the robot turns left or right.
	leftSpeed = 300;
	rightSpeed = 100;

	motors.setSpeeds(leftSpeed, rightSpeed);
}
