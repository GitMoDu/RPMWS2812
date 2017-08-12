
/// 
///  Created for personal use, use it at your own risk and benefit.
/// 

#include "PinDefinitions.h"
#include "LedDefinitions.h"

#include "FastColour.h"
#include "LedSection.h"
#include "RPMWS2812.h"
//#include <NeoSerial.h>
//#include <SoftwareSerial.h>

#define SERIAL_BAUD_RATE 115200
#define STARTING_BRIGHTNESS 150 //Out of 255

#define ANIMATION_STEPS 6000
#define DEMO_RPM_RANGE 19000

#define ANIMATION_DURATION 12000
#define ANIMATION_UPDATE_INTERVAL 5
#define DEMO_RPM_UPDATE_INTERVAL  5
#define DEMO_LED_UPDATE_INTERVAL 100
#define DEMO_RPM_LOG_UPDATE_INTERVAL 400

#define BRIGHTNESS_UPDATE_INTERVAL 30
#define SERIAL_POLL_INTERVAL 20

#define POWER_AVERAGE_SAMPLES 10

//Sections data
#define MEDIUM_RPM 9000
#define HIGH_RPM 13000
#define MAX_RPM 15000

#define ALERT_PULSE_MILLIS 110
#define ALERT_PULSE_DUTY_CYCLE 160
#define ALERT_MIN_BRIGHTNESS 60

#define LOW_COLOUR cHSV(240, 255, 255)
#define MEDIUM_COLOUR cHSV(100, 255, 255)
#define HIGH_COLOUR cHSV(120, 255, 255)
#define BACKGROUND_COLOUR cHSV(240, 255, 5)
#define LED_MIN_BRIGHTNESS_VALUE 20



//NeoSerial Serial(0, 1);
//SoftwareSerial Serial(4, 3);
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete


RPMWS2812 RPMDriver;

uint8_t GlobalBrightness = STARTING_BRIGHTNESS;
uint8_t LastGlobalBrightness = 0;

//Mostly demo stuff
uint16_t DemoRPM = 0;
uint16_t AnimationStep = 0;
uint32_t StartTime, RPMStart, RPMEnd;
uint32_t Now;
uint32_t AnimationStart;
uint32_t LastRPMUpdate, LastLEDUpdate, LastAnimationUpdate, LastRPMLogUpdate,
LastPowerUpdate, LastPowerLogUpdate, LastBrightnessUpdate, LastSerialUpdate;

void SetupDemo()
{
	AnimationStep = 0;

	StartTime = millis();

	LastLEDUpdate = StartTime;
	LastRPMUpdate = StartTime;
	LastLEDUpdate = StartTime;
	LastAnimationUpdate = StartTime;
	AnimationStart = StartTime;
	LastPowerUpdate = StartTime;
	LastPowerLogUpdate = StartTime;
	LastRPMLogUpdate = StartTime;
	LastBrightnessUpdate = StartTime;
	LastSerialUpdate = StartTime;
}

void Demo(uint32_t now)
{
	if (now - LastAnimationUpdate > ANIMATION_UPDATE_INTERVAL - 1)
	{
		LastAnimationUpdate += ANIMATION_UPDATE_INTERVAL;

		AnimationStep = ((ANIMATION_STEPS*(now - AnimationStart)) / ANIMATION_DURATION);

		if (AnimationStep < ANIMATION_STEPS / 2)
		{
			DemoRPM = AnimationStep * 2 * ((uint32_t)DEMO_RPM_RANGE / (uint32_t)ANIMATION_STEPS);
		}
		else
		{
			DemoRPM = (ANIMATION_STEPS - AnimationStep) * 2 * ((uint32_t)DEMO_RPM_RANGE / ANIMATION_STEPS);
		}

		if (now - AnimationStart > ANIMATION_DURATION - 1)
		{
			AnimationStep = 0;
			Serial.print(F("Animation end, took "));
			Serial.print((int32_t)(now - AnimationStart), DEC);
			Serial.println(F(" ms"));
			AnimationStart = now;
		}
	}
}


void serialEvent() {
	while (Serial.available()) {
		// get the new byte:
		char inChar = (char)Serial.read();
		// add it to the inputString:
		inputString += inChar;
		// if the incoming character is a newline, set a flag
		// so the main loop can do something about it:
		if (inChar == '\n') {
			stringComplete = true;
		}
	}
}

void UpdateSerial()
{
	int NewBrightness = inputString.toInt();

	GlobalBrightness = constrain(NewBrightness, 0, 255);

	inputString = "";
	stringComplete = false;

}

void UpdateBrightness()
{
	if (GlobalBrightness != LastGlobalBrightness)
	{
		RPMDriver.SetBrightness(GlobalBrightness);
		//RPMDriver.SetExtendedOverflowRange(constrain(GlobalBrightness + 5, 0, 255));
		//Serial.print("GlobalBrightness: ");
		//Serial.println(GlobalBrightness);

		LastGlobalBrightness = GlobalBrightness;
	}

}

void setup()
{

	Serial.begin(SERIAL_BAUD_RATE);
	inputString.reserve(200);// reserve bytes for the inputString:
	delay(500);

	Serial.println();
	Serial.println();

	Serial.println(F("RPM setup..."));
	if (!SetupRPMDriver())
	{
		Serial.println(F(" failed"));
		Halt();
	}
	Serial.println(F(" complete"));

	SetupDemo();
}



bool SetupRPMDriver()
{
	bool Success = false;
	//RPMDriver.AddLogger(&Serial);
	Success = RPMDriver.Begin();
	RPMDriver.SetDesignModel(SUB_PIXEL_ENABLED | BACKGROUND_ENABLED);// | SUB_PIXEL_HIGH_RANGE_ENABLED | BACKGROUND_ENABLED);
	RPMDriver.SetBrightness(GlobalBrightness);
	RPMDriver.SetRangeRPM(600, 15000);
	RPMDriver.ClearSections();

	RPMDriver.SetAlertBlink(HIGH_COLOUR, ALERT_PULSE_MILLIS, ALERT_PULSE_DUTY_CYCLE);

	//RPMDriver.SetExtendedOverflowRange(constrain(GlobalBrightness + 50, 0, 255));

	RPMDriver.SetSection(0, MEDIUM_RPM / 2, LOW_COLOUR, BACKGROUND_COLOUR);
	RPMDriver.SetSection(MEDIUM_RPM / 2, MEDIUM_RPM, cHSV(300, 254, 254), BACKGROUND_COLOUR);
	RPMDriver.SetSection(MEDIUM_RPM, HIGH_RPM, MEDIUM_COLOUR, BACKGROUND_COLOUR);
	RPMDriver.SetSection(HIGH_RPM, MAX_RPM, HIGH_COLOUR, BACKGROUND_COLOUR);//, { 120,255,1 }
	RPMDriver.BootAnimation(LOW_COLOUR);

}

void loop()
{
	Now = millis();

	if (Now - LastLEDUpdate > DEMO_LED_UPDATE_INTERVAL - 1)
	{
		LastLEDUpdate += DEMO_LED_UPDATE_INTERVAL;
	}

	if (Now - LastRPMLogUpdate > DEMO_RPM_LOG_UPDATE_INTERVAL - 1)
	{
		LastRPMLogUpdate += DEMO_RPM_LOG_UPDATE_INTERVAL;
		Serial.print("RPM: ");
		Serial.println(DemoRPM);
	}

	if (Now - LastRPMUpdate > DEMO_RPM_UPDATE_INTERVAL - 1)
	{
		LastRPMUpdate += DEMO_RPM_UPDATE_INTERVAL;



		//RPMStart = micros();
		RPMDriver.UpdateRPM(DemoRPM, Now);
		//Serial.println(RPMDriver.Debug());
		//RPMEnd = micros();
		//Serial.print("Update RPM took: ");
		//Serial.print((RPMEnd - RPMStart));
		//Serial.println(" us");
	}

	if (Now - LastBrightnessUpdate > BRIGHTNESS_UPDATE_INTERVAL - 1)
	{

		LastBrightnessUpdate += BRIGHTNESS_UPDATE_INTERVAL;

		UpdateBrightness();
		/*Serial.print("GlobalBrightness: ");
		Serial.println(GlobalBrightness);*/
	}

	Demo(Now);

	if (Now - LastSerialUpdate > SERIAL_POLL_INTERVAL - 1)
	{
		if (stringComplete)
		{
			UpdateSerial();
		}
		LastSerialUpdate += SERIAL_POLL_INTERVAL;


		//Serial.print("GlobalBrightness: ");
		//Serial.println(GlobalBrightness);
	}
}

void Halt()
{
	while (true);;
}