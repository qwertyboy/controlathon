#include <Wire.h>
#include <Zumo32U4.h>

Zumo32U4LCD lcd;
Zumo32U4LineSensors lineSensors;
Zumo32U4ProximitySensors proxSensors;
Zumo32U4Motors motors;

uint16_t lineSensorValues[5] = { 0, 0, 0, 0, 0 };

bool proxLeftActive;
bool proxFrontActive;
bool proxRightActive;

void setup() {
	lineSensors.initThreeSensors();
	proxSensors.initThreeSensors();
}

void loop() {
	static uint16_t lastSampleTime = 0;

	if ((uint16_t)(millis() - lastSampleTime) >= 100) {
		lastSampleTime = millis();

		// Send IR pulses and read the proximity sensors.
		proxSensors.read();

		// Just read the proximity sensors without sending pulses.
		proxLeftActive = proxSensors.readBasicLeft();
		proxFrontActive = proxSensors.readBasicFront();
		proxRightActive = proxSensors.readBasicRight();

	drive();
	}
}

void drive() {
	int left = proxSensors.countsLeftWithLeftLeds();
	int right = proxSensors.countsRightWithRightLeds();

	motors.setSpeeds((left * left * 10) + 40, (right * right * 10 + (right * 4)) + 40);
}
