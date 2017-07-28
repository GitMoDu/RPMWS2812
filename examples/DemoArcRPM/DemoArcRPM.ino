
/// 
///  Created for personal use, use it at your own risk and benefit.
/// 

#include "PinDefinitions.h"
#include "LedDefinitions.h"

#include "FastColour.h"
#include "LedSection.h"
#include "RPMWS2812.h"
#include <NeoSWSerial.h>
//#include <SoftwareSerial.h>

#define SERIAL_BAUD_RATE 19200
#define BRIGHTNESS 40 //Out of 254
#define DEMO_REFRESH_PERIOD_MILLIS 30

#define DEMO_RPM_RANGE 16000

#define ANIMATION_DURATION 7600
#define ANIMATION_UPDATE_INTERVAL 10
#define DEMO_RPM_UPDATE_INTERVAL 20
#define DEMO_LED_UPDATE_INTERVAL 20
#define DEMO_RPM_LOG_UPDATE_INTERVAL 500
#define ANIMATION_STEPS 5000


//Sections data
#define MEDIUM_RPM 9000
#define HIGH_RPM 13000
#define MAX_RPM 15000

#define LOW_COLOUR cHSV(240, 254, 254)
#define MEDIUM_COLOUR cHSV(100, 254, 254)
#define HIGH_COLOUR cHSV(120, 254, 254)

NeoSWSerial SWSerial(0, 1);
//SoftwareSerial SWSerial(4, 3);
RPMWS2812 RPMDriver;

void Demo()
{

	uint16_t DemoRPM, AnimationStep = 0;
	uint32_t StartTime, RPMStart, RPMEnd;
	uint32_t Elapsed;
	uint32_t AnimationStart;
	uint32_t LastRPMUpdate, LastLEDUpdate, LastAnimationUpdate, LastRPMLogUpdate;

	StartTime = millis();
	LastLEDUpdate = StartTime;


	StartTime = millis();
	LastRPMUpdate = StartTime;
	LastLEDUpdate = StartTime;
	LastAnimationUpdate = StartTime;
	AnimationStep = 0;
	AnimationStart = StartTime;

	while (true)
	{
		Elapsed = millis();

		if (Elapsed - LastLEDUpdate > DEMO_LED_UPDATE_INTERVAL - 1)
		{
			LastLEDUpdate += DEMO_LED_UPDATE_INTERVAL;
			RPMDriver.Show();
		}

		if (Elapsed - LastRPMLogUpdate > DEMO_RPM_LOG_UPDATE_INTERVAL - 1)
		{
			LastRPMLogUpdate += DEMO_RPM_LOG_UPDATE_INTERVAL;
			SWSerial.print("RPM: ");
			SWSerial.println(DemoRPM);
		}

		if (Elapsed - LastRPMUpdate > DEMO_RPM_UPDATE_INTERVAL - 1)
		{
			LastRPMUpdate += DEMO_RPM_UPDATE_INTERVAL;

			if (AnimationStep < ANIMATION_STEPS / 2)
			{
				DemoRPM = AnimationStep * 2 * ((DEMO_RPM_RANGE) / ANIMATION_STEPS);
			}
			else
			{
				DemoRPM = (ANIMATION_STEPS - AnimationStep) * 2 * (DEMO_RPM_RANGE / ANIMATION_STEPS);
			}

			RPMStart = micros();
			RPMDriver.UpdateRPM(DemoRPM, false);
			//SWSerial.println(RPMDriver.Debug());
			RPMEnd = micros();
			SWSerial.print("Update RPM took: ");
			SWSerial.print((RPMEnd - RPMStart));
			SWSerial.println(" us");
		}

		if (Elapsed - LastAnimationUpdate > ANIMATION_UPDATE_INTERVAL - 1)
		{
			LastAnimationUpdate += ANIMATION_UPDATE_INTERVAL;

			AnimationStep = ((ANIMATION_STEPS*(Elapsed - AnimationStart)) / ANIMATION_DURATION);

			if (Elapsed - AnimationStart > ANIMATION_DURATION - 1)
			{
				AnimationStep = 0;
				SWSerial.print(F("Animation end, took "));
				SWSerial.print((int32_t)(Elapsed - AnimationStart), DEC);
				SWSerial.println(F(" ms"));
				AnimationStart = Elapsed;
			}
		}
	}

}

void setup()
{
	SWSerial.begin(SERIAL_BAUD_RATE);
	delay(10);
	SWSerial.println();
	SWSerial.println(F("RPM setup"));

	RPMDriver.AddLogger(&SWSerial);
	RPMDriver.Begin();
	RPMDriver.SetDesignModel(SUB_PIXEL_ENABLED);
	RPMDriver.SetBrightness(BRIGHTNESS);
	RPMDriver.SetRangeRPM(600, 15000);
	RPMDriver.ClearSections();
	RPMDriver.SetSection(0, MEDIUM_RPM /2, LOW_COLOUR);
	RPMDriver.SetSection(MEDIUM_RPM / 2, MEDIUM_RPM, cHSV( 300, 254, 254));
	RPMDriver.SetSection(MEDIUM_RPM, HIGH_RPM, MEDIUM_COLOUR);
	RPMDriver.SetSection(HIGH_RPM, MAX_RPM, HIGH_COLOUR);//, { 120,255,1 }
	RPMDriver.BootAnimation(LOW_COLOUR);
	SWSerial.println(F("RPM setup complete"));


	Demo();
}


void loop() {
}
