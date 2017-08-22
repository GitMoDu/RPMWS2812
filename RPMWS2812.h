///  RPM indicator based on WS2812 LEDs
///
///  Created for personal use, use it at your own risk and benefit.
///  https://github.com/GitMoDu/RPMWS2812
///  Based on the excellent work by Kasper Kamperman(https://www.kasperkamperman.com)
///  Depends on Light_WS2812 https://github.com/cpldcpu/light_ws2812
/// 

#ifndef _RPMWS2812_h
#define _RPMWS2812_h

#include <Arduino.h>
#define USE_HSV
#include <WS2812.h>
//#include <Doge.h>
#include "LedSection.h"

#define SUB_PIXEL_ENABLED B00000001
#define MARKERS_ENABLED B00000010
#define BACKGROUND_ENABLED B00000100
#define SUB_PIXEL_HIGH_RANGE_ENABLED B00001000

#ifndef MAX_LED_COUNT
#define MAX_LED_COUNT 30
#endif
#ifndef MAX_SECTION_COUNT
#define MAX_SECTION_COUNT 10
#endif


class RPMWS2812
{
private:
#define UINT8_MAXVALUE 255
#define INT8_MAXVALUE 127

#define UINT16_MAXVALUE 65025



#define ARC_DEAD_BLINK_DURATION_DEFAULT 800
#define ARC_ALERT_BLINK_DURATION_DEFAULT 40
#define ARC_WAKEUP_FADE_DURATION_DEFAULT 1100

#define ARC_EXTENDED_OVERFLOW_RANGE_DEFAULT 255

#define ARC_COLOUR_BOOT_START 240,254,200
#define ARC_BOOT_DURATION 2000

#define ARC_BRIGHTNESS_DEFAULT 128

#define ARC_DEAD_HUE_DEFAULT 225
#define ARC_ALERT_HUE_DEFAULT 96

#define ARC_ALIVE_RPM_DEFAULT 500
#define ARC_ALERT_RPM_DEFAULT 15000

	WS2812 Leds = WS2812(MAX_LED_COUNT);

	uint8_t LedCount = MAX_LED_COUNT;
	uint8_t LedDataPin = 255;

	uint16_t DeadBlinkDuration = ARC_DEAD_BLINK_DURATION_DEFAULT;
	uint16_t AlertBlinkDuration = ARC_ALERT_BLINK_DURATION_DEFAULT;
	uint16_t WakeupFadeDuration = ARC_WAKEUP_FADE_DURATION_DEFAULT;

	uint8_t GlobalBrightness = ARC_BRIGHTNESS_DEFAULT;

	cHSV ColourClear = cHSV(0, 0, 0);
	cHSV ColourBoot = cHSV(ARC_COLOUR_BOOT_START);

	cHSV ColourDeadBlink = cHSV(ARC_DEAD_HUE_DEFAULT, 254, 128);
	cHSV ColourAlertBlink = cHSV(ARC_ALERT_HUE_DEFAULT, 254, 254);

	uint16_t RPM_Alive = ARC_ALIVE_RPM_DEFAULT;
	uint16_t RPM_Alert = ARC_ALERT_RPM_DEFAULT;
	uint16_t RPM_Latest = 0;

	//Updated on set RPM values
	uint16_t RPM_Per_Led;
	uint8_t RPM_AliveInt, RPM_AlertInt;
	uint8_t RPM_Int;

	//Runtime helper variables
	uint8_t RPM_PixelOverflow;
	uint8_t ExtendedOverflowRange = ARC_EXTENDED_OVERFLOW_RANGE_DEFAULT;

	cRGB RGBValueHelper;

	bool RPM_BlinkAlive, RPM_BlinkAlert = false;
	uint32_t RPM_BlinkTimeStamp = 0;
	uint8_t ProgressHelper = 0;
	uint8_t FlashingCurve(const uint8_t input, const uint8_t value);
	uint8_t WarningCurve(const uint8_t input, const uint8_t value);

	bool NeedsRefresh = true;

	//Down to 1 LED per Section
	LedSection Sections[MAX_SECTION_COUNT];
	uint8_t SectionCount;

	byte DesignModel = SUB_PIXEL_ENABLED;

	void UpdateSectionsDynamic();
	void UpdateSectionsConstant();
	void UpdateSections(const uint32_t timeStamp);

	void BlinkAlertUpdate(const uint32_t timeStamp);
	void BlinkDeadUpdate(const uint32_t timeStamp);
	void FillUpdate1(const uint32_t timeStamp);

	uint8_t BrightnessAdjust(uint8_t value);

	void Set(const cRGB colourRGB, const uint8_t startIndex, const uint8_t endIndex);
	cRGB Scale(const cRGB colourRGB, const byte scale);
	void DirectAnimate(const cHSV colourStart, const cHSV colourEnd, const uint16_t durationMillis);
	cHSV CrossfadedHSB(cHSV colour1, cHSV colour2, byte progress);

public:

	bool Setup(uint8_t ledCount, uint8_t ledDataPin);
	void BootAnimation(cHSV colourBoot, uint8_t animationBrightness);
	void BootAnimation(cHSV colourBoot = cHSV(ARC_COLOUR_BOOT_START));
	void SetAllHSV(cHSV colour);
	void SetAll(cRGB colourRGB);
	void SetBrightness(const uint8_t brightness);
	void UpdateRPM(const uint16_t rpm, const uint32_t timeStamp, const bool autoRefresh = true);
	void SetRangeRPM(const uint16_t aliveRPM, const uint16_t maxRPM);
	void SetDesignModel(byte designModel);

	void SetAlertBlink(cHSV blinkColour, uint16_t alertBlinkDuration);
	void SetAlertBlink(uint16_t alertBlinkDuration);
	void SetAlertBlink(cHSV blinkColour);
	void SetDeadBlink(cHSV blinkColour, uint16_t deadBlinkDuration);
	void SetDeadBlink(uint16_t deadBlinkDuration);
	void SetDeadBlink(cHSV blinkColour);

	void SetExtendedOverflowRange(uint8_t range);

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

