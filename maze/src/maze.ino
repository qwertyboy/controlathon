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
uint32_t turnAngle = 0;
int16_t turnRate;
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

// Read the gyro and update the angle.  This should be called as
// frequently as possible while using the gyro to do turns.
void turnSensorUpdate()
{
	// Read the measurements from the gyro.
	gyro.read();
	turnRate = gyro.g.z - gyroOffset;

	// Figure out how much time has passed since the last update (dt)
	uint16_t m = micros();
	uint16_t dt = m - gyroLastUpdate;
	gyroLastUpdate = m;

	// Multiply dt by turnRate in order to get an estimation of how
	// much the robot has turned since the last update.
	// (angular change = angular velocity * time)
	int32_t d = (int32_t)turnRate * dt;

	// The units of d are gyro digits times microseconds.  We need
	// to convert those to the units of turnAngle, where 2^29 units
	// represents 45 degrees.  The conversion from gyro digits to
	// degrees per second (dps) is determined by the sensitivity of
	// the gyro: 0.07 degrees per second per digit.
	//
	// (0.07 dps/digit) * (1/1000000 s/us) * (2^29/45 unit/degree)
	// = 14680064/17578125 unit/(digit*us)
	turnAngle += (int64_t)d * 14680064 / 17578125;
}


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

	if (frontLeft >= 5 && frontRight >= 5) {
		if (left > right) {
			Serial.print("left");
			turnLeft();
		} else {
			Serial.print("right");
			turnRight();
		}
	}

	if (frontLeft < 1 && frontRight < 1) {
		motors.setSpeeds(-100, -100);
		delay(400);
		motors.setSpeeds(200, -200);
		delay(300);
	}

	static char buffer[80];
	sprintf(buffer, "%d %d %d %d\n", left, frontLeft, frontRight, right);
	Serial.print(buffer);

	// determines if the robot turns left or right.
	leftSpeed = 150;
	rightSpeed = 150;

	if (left < 4 && frontLeft <= 3 && frontRight <= 3) {
		rightSpeed += 50;
		Serial.print("right");
	} else if (right < 4 && frontLeft <= 3 && frontRight <= 3) {
		leftSpeed += 50;
		Serial.print("left");
	}

	motors.setSpeeds(leftSpeed, rightSpeed);
}

// turning functions
int16_t turnSpeed = 300;
const int32_t turnAngle45 = 0x20000000;
int16_t turnDelay = 200;
void turnLeft(){
	motors.setSpeeds(turnSpeed, -turnSpeed);
	delay(turnDelay);
	motors.setSpeeds(0, 0);
}

void turnRight(){
	motors.setSpeeds(-turnSpeed, turnSpeed);
	delay(turnDelay);
	motors.setSpeeds(0, 0);
}
