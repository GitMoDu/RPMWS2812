/// 
///  Created for personal use, use it at your own risk and benefit.
///  Based on the excellent work by Kasper Kamperman(https://www.kasperkamperman.com)
///  Depends on Light_WS2812 https://github.com/cpldcpu/light_ws2812
/// 

#ifndef _RPMWS2812_h
#define _RPMWS2812_h

#include <Arduino.h>
#include <WS2812.h>
#include <NeoSWSerial.h>

#include "LedDefinitions.h"
#include "PinDefinitions.h"
#include "LedSection.h"

#define MAX_LED_COUNT 254

#define SUB_PIXEL_ENABLED B00000001
#define MARKERS_ENABLED B00000010

class RPMWS2812
{

private:
#define ARC_DEAD_BLINK_DURATION_DEFAULT 300
#define ARC_ALERT_BLINK_DURATION_DEFAULT 40
#define ARC_WAKEUP_FADE_DURATION_DEFAULT 1100

#define COLOUR_BOOT_START 240,254,200
#define BOOT_DURATION 2000

#define ARC_BLINK_HUE_DEFAULT 160
#define ARC_ALERT_HUE_DEFAULT 96

#define ARC_ALIVE_RPM_DEFAULT 500
#define ARC_ALERT_RPM_DEFAULT 15000

	WS2812 Leds = WS2812(LED_COUNT);

	uint16_t DeadBlinkDuration = ARC_DEAD_BLINK_DURATION_DEFAULT;
	uint16_t AlertBlinkDuration = ARC_ALERT_BLINK_DURATION_DEFAULT;
	uint16_t WakeupFadeDuration = ARC_WAKEUP_FADE_DURATION_DEFAULT;

	uint8_t GlobalBrightness = 254;

	cHSV ColourClear = cHSV(0, 0, 0);
	cHSV ColourBoot = cHSV(COLOUR_BOOT_START);

	cHSV ColourDeadBlink = cHSV(ARC_BLINK_HUE_DEFAULT, 254, 128);
	cHSV ColourAlertBlink = cHSV(ARC_ALERT_HUE_DEFAULT, 254, 254);

	uint16_t RPM_Alive = ARC_ALIVE_RPM_DEFAULT;
	uint16_t RPM_Alert = ARC_ALERT_RPM_DEFAULT;
	uint16_t RPM_Latest = 0;

	//Updated on set RPM values
	uint16_t RPM_Per_Led;
	uint16_t RPM_AliveInt;
	uint16_t RPM_Int;

	//Runtime helper variables
	uint32_t RPM_FloorScaled, RPM_Overflow;
	uint8_t RPM_PixelOverflow;

	cRGB RGBValue;

	bool RPM_BlinkAlive, RPM_BlinkAlert = false;
	uint64_t RPM_BlinkTimeStamp = 0;

	uint64_t RPM_LastDeadTimeStamp = 0;
	uint64_t RPM_LastAlertTimeStamp = 0;

	LedSection Sections[LED_COUNT];//Up to 1 section per Led
	uint8_t SectionCount;

	byte DesignModel = SUB_PIXEL_ENABLED;

	void UpdateConstants();
	void UpdateRPMConstants();
	void UpdateSections();

	void Set(const cRGB colourRGB, const uint8_t startIndex, const uint8_t endIndex);
	cRGB Scale(const cRGB colourRGB, const byte scale);
	void DirectAnimate(const cHSV colourStart, const cHSV colourEnd, const uint16_t durationMillis);
	cHSV CrossfadedHSB(cHSV colour1, cHSV colour2, byte progress);
	

	NeoSWSerial* Logger = nullptr;
	bool Log = false;

	
public:

	void AddLogger(NeoSWSerial* logger);
	bool Begin();
	void BootAnimation(cHSV colourBoot = cHSV(COLOUR_BOOT_START));
	void SetAllHSV(cHSV colour);
	void SetAll(cRGB colourRGB);
	void SetBrightness(const uint8_t brightness);
	void UpdateRPM(const uint16_t rpm, const bool autoRefresh = true);
	void SetRangeRPM(const uint16_t aliveRPM, const uint16_t maxRPM);
	void SetDesignModel(byte designModel);
	void SetDeadBlink(uint16_t deadBlinkDuration, cHSV blinkColour, uint16_t wakeupFadeDuration = 0);
	void SetAlertBlink(uint16_t alertBlinkDuration, cHSV blinkColour);

	String Debug();
	void Clear();
	void ClearSections();
	void SetSection(const uint16_t beginRPM, const uint16_t endRPM, const cHSV fillColour,
		const cHSV backgroundColour = COLOUR_NONE,
		const cHSV markerColour = COLOUR_NONE);
	void Show();
	void Pulse(const cHSV colour, const uint16_t durationMillis);
};




#endif

