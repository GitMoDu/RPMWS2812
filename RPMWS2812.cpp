///  RPM indicator based on WS2812 LEDs
///
///  Created for personal use, use it at your own risk and benefit.
///  https://github.com/GitMoDu/RPMWS2812
///  Based on the excellent work by Kasper Kamperman(https://www.kasperkamperman.com)
///  Depends on Light_WS2812 https://github.com/cpldcpu/light_ws2812
/// 

#include "RPMWS2812.h"

bool RPMWS2812::Setup(uint8_t ledCount, uint8_t ledDataPin)
{
	if (ledCount <= MAX_LED_COUNT)
	{
		LedCount = ledCount;
		LedDataPin = ledDataPin;
		Leds.setOutput(LedDataPin);
		Leds.setColorOrderRGB();
		Clear();
		return true;
	}
	else
	{
		return false;
	}
}

void RPMWS2812::BootAnimation(cHSV colourBoot, uint8_t animationBrightness)
{
	uint16_t BootDelayStep = ARC_BOOT_DURATION / 4;

	cHSV ColourBootStart, ColourBootMiddle, ColourBootPeak;

	ColourBootStart = cHSV(colourBoot.h, colourBoot.s, 0);
	ColourBootMiddle = cHSV(colourBoot.h, colourBoot.s, ((animationBrightness / 4)*colourBoot.v) / 255);
	ColourBootPeak = cHSV(colourBoot.h, colourBoot.s, ((animationBrightness)*colourBoot.v) / 255);

	Clear();
	SetAllHSV(ColourBootStart);
	Show();
	delay(BootDelayStep / 4);
	DirectAnimate(ColourBootStart, ColourBootPeak, BootDelayStep);
	DirectAnimate(ColourBootPeak, ColourBootMiddle, BootDelayStep);
	DirectAnimate(ColourBootMiddle, ColourBootStart, BootDelayStep);
	delay((BootDelayStep * 3) / 4);
}


void RPMWS2812::BootAnimation(cHSV colourBoot)
{
	BootAnimation(colourBoot, GlobalBrightness);
}

void RPMWS2812::Show(const bool forceShow)
{
	if (forceShow || NeedsRefresh)
	{
		Leds.sync();
		NeedsRefresh = false;
	}
	
}

cRGB RPMWS2812::Scale(const cRGB colour, const byte scale)
{
	cRGB Result = colour;
	Result.r = (((uint16_t)colour.r * (uint16_t)scale)) / 255;
	Result.g = (((uint16_t)colour.g * (uint16_t)scale)) / 255;
	Result.b = (((uint16_t)colour.b * (uint16_t)scale)) / 255;

	return Result;
}

cHSV RPMWS2812::CrossfadedHSB(cHSV colour1, cHSV colour2, byte progress)
{
	cHSV Result;
	byte progress1 = (254 - progress);
	byte progress2 = progress;

	Result.h = (uint8_t)(((uint16_t)colour1.h * (uint16_t)progress1 + (uint16_t)colour2.h * (uint16_t)progress2) / (uint16_t)254);
	Result.s = (uint8_t)(((uint16_t)colour1.s * (uint16_t)progress1 + (uint16_t)colour2.s *(uint16_t)progress2) / (uint16_t)254);
	Result.v = (uint8_t)((((uint16_t)colour1.v * (uint16_t)progress1 + (uint16_t)colour2.v * (uint16_t)progress2) + 1) / (uint16_t)255);

	return Result;
}

void RPMWS2812::DirectAnimate(const cHSV colourStart, const cHSV colourEnd, const uint16_t durationMillis)
{
	uint16_t Step = durationMillis / 255;
	cHSV AnimatedHSV;
	cRGB AnimatedRGB;
	for (uint8_t i = 0; i < 255; i++)
	{
		AnimatedHSV = CrossfadedHSB(colourStart, colourEnd, i);
		AnimatedRGB.SetHSV(AnimatedHSV.h, AnimatedHSV.s, AnimatedHSV.v);
		SetAll(AnimatedRGB);
		Show();
		delay(Step);
	}
}

void RPMWS2812::Pulse(const cHSV colour, const uint16_t durationMillis)
{
	DirectAnimate(COLOUR_NONE, colour, durationMillis);
	DirectAnimate(colour, COLOUR_NONE, durationMillis);
}

void RPMWS2812::SetBrightness(const uint8_t brightness)
{
	if (GlobalBrightness != brightness)
	{
		GlobalBrightness = brightness;
		UpdateSectionsDynamic();
	}
}

void RPMWS2812::Clear()
{
	SetAllHSV(ColourClear);
	Show();
}

void RPMWS2812::SetAll(cRGB colour)
{
	Set(colour, 0, LedCount);
}

void RPMWS2812::SetAllHSV(cHSV colour)
{
	RGBValueHelper.SetHSV(colour.h, colour.s, colour.v);
	Set(RGBValueHelper, 0, LedCount);
}

void RPMWS2812::Set(const cRGB colour, const  uint8_t startIndex, const uint8_t endIndex)
{
	for (uint8_t i = startIndex; i < endIndex; i++)
	{
		Leds.set_crgb_at(i, colour);
	}
}

void RPMWS2812::UpdateSectionsConstant()//Elements that do not change with brightness
{
	RPM_Per_Led = RPM_Alert / LedCount;
	RPM_AliveInt = RPM_Alive / RPM_Per_Led;
	RPM_AlertInt = RPM_Alert / RPM_Per_Led;

	for (uint8_t i = 0; i < SectionCount; i++)
	{
		Sections[i].Begin = Sections[i].BeginRPM / RPM_Per_Led;
		Sections[i].End = Sections[i].EndRPM / RPM_Per_Led;
	}
}

uint8_t RPMWS2812::BrightnessAdjust(uint8_t value)
{
	return map(value * GlobalBrightness, 0, UINT16_MAXVALUE, 0, UINT8_MAXVALUE);
}

void RPMWS2812::UpdateSectionsDynamic()
{
	for (uint8_t i = 0; i < SectionCount; i++)
	{
		//Fill in brightness adjusted colours. 
		//This prevents lots of calculations while brightness is constant.
		Sections[i].BackgroundColourBrightness = BrightnessAdjust(Sections[i].BackgroundColour.v);
		Sections[i].MarkerColourBrightness = max(Sections[i].BackgroundColourBrightness, BrightnessAdjust(Sections[i].MarkerColour.v));
		Sections[i].FillColourBrightness = max(Sections[i].MarkerColourBrightness, BrightnessAdjust(Sections[i].FillColour.v));
	}

	NeedsRefresh = true;
}

void RPMWS2812::UpdateRPM(const uint16_t rpm, const uint32_t timeStamp, const bool autoRefresh)
{	
	if (RPM_Latest != min(rpm, RPM_Alert))
	{
		NeedsRefresh = true;
		RPM_Latest = min(rpm, RPM_Alert);
		RPM_Int = RPM_Latest / RPM_Per_Led;

		if (RPM_Int >= RPM_AlertInt)
		{
			BlinkAlertUpdate(timeStamp);
		}
		else if (RPM_Latest < RPM_Alive)
		{
			BlinkDeadUpdate(timeStamp);
		}
		else
		{
			RPM_BlinkTimeStamp = 0; //Resets the blinking animations.
			FillUpdate1();
		}
	}
	else
	{
		if (RPM_Int >= RPM_AlertInt)
		{
			BlinkAlertUpdate(timeStamp);
			NeedsRefresh = true;
		}
		else if (RPM_Latest < RPM_Alive)
		{
			BlinkDeadUpdate(timeStamp);
			NeedsRefresh = true;
		}
	}

	if (autoRefresh)
	{
		Show();
	}
}

void RPMWS2812::SetSection(const uint16_t beginRPM, const uint16_t endRPM,
	const cHSV fillColour, const  cHSV backgroundColour, const  cHSV markerColour)
{
	if (SectionCount < MAX_SECTION_COUNT)
	{
		LedSection NewSection;
		NewSection.Set(beginRPM, endRPM, fillColour, backgroundColour, markerColour);
		Sections[SectionCount++] = NewSection;
		UpdateSectionsConstant();
		UpdateSectionsDynamic();
	}
}

void RPMWS2812::ClearSections()
{
	SectionCount = 0;
}

void RPMWS2812::SetDesignModel(byte designModel)
{
	DesignModel = designModel;
	UpdateSectionsConstant();
	UpdateSectionsDynamic();
}


uint8_t RPMWS2812::FlashingCurve(const uint8_t scale, const uint8_t value)
{
#define ARC_ALERT_BLINK_STEP_MARGIN 40 //Prevents aliasing at low periods by increasing the time spent on max brightness
#define ARC_ALERT_BLINK_STEP_DEAD 160 //Makes the flashes even more prominent.

	return BrightnessAdjust(map(value *
		map(min(max(0, (int16_t)scale - ARC_ALERT_BLINK_STEP_DEAD), UINT8_MAXVALUE - ARC_ALERT_BLINK_STEP_MARGIN - ARC_ALERT_BLINK_STEP_DEAD),
			0, UINT8_MAXVALUE - ARC_ALERT_BLINK_STEP_MARGIN - ARC_ALERT_BLINK_STEP_DEAD, 0, UINT8_MAXVALUE),
		0, UINT16_MAXVALUE, 0, UINT8_MAXVALUE));
}

uint8_t RPMWS2812::WarningCurve(const uint8_t scale, const uint8_t value)
{
	if (scale < INT8_MAXVALUE)
	{
		return BrightnessAdjust(map(value * map(scale, 0, INT8_MAXVALUE, 0, UINT8_MAXVALUE), 0, UINT16_MAXVALUE, 0, UINT8_MAXVALUE));
	}
	else
	{
		return BrightnessAdjust(map(value * map(UINT8_MAXVALUE - scale, 0, INT8_MAXVALUE, 0, UINT8_MAXVALUE), 0, UINT16_MAXVALUE, 0, UINT8_MAXVALUE));
	}
}

void RPMWS2812::BlinkAlertUpdate(const uint32_t timeStamp)
{
	if (RPM_BlinkTimeStamp == 0)//New cycle, time to start fresh;
	{
		RPM_BlinkTimeStamp = timeStamp;
		SetAllHSV(ColourClear);
	}
	else
	{
		ProgressHelper = map(timeStamp - RPM_BlinkTimeStamp, 0, AlertBlinkDuration, 0, UINT8_MAXVALUE);
		if (ProgressHelper == UINT8_MAXVALUE)
		{
			RPM_BlinkTimeStamp = 0;//End of the flash cycle, start again.
		}

		SetAllHSV({ ColourAlertBlink.h , ColourAlertBlink.s, FlashingCurve(ProgressHelper, ColourAlertBlink.v) });
	}
}

void RPMWS2812::BlinkDeadUpdate(const uint32_t timeStamp)
{
	if (RPM_BlinkTimeStamp == 0)//New cycle, time to start fresh;
	{
		RPM_BlinkTimeStamp = timeStamp;
		SetAllHSV(ColourClear);
	}
	else
	{
		ProgressHelper = map(timeStamp - RPM_BlinkTimeStamp, 0, DeadBlinkDuration, 0, UINT8_MAXVALUE);
		if (ProgressHelper == UINT8_MAXVALUE)
		{
			RPM_BlinkTimeStamp = 0;//End of the flash cycle, start again.
		}

		SetAllHSV({ ColourDeadBlink.h , ColourDeadBlink.s, WarningCurve(ProgressHelper,ColourDeadBlink.v) });
	}
}

void RPMWS2812::FillUpdate1()
{
	for (uint8_t j = 0; j < SectionCount; j++)
	{
		for (uint8_t i = Sections[j].Begin; i < Sections[j].End; i++)
		{
			if (i < RPM_Int)//Filled pixel
			{
				RGBValueHelper.SetHSV(Sections[j].FillColour.h, Sections[j].FillColour.s, Sections[j].FillColourBrightness);
			}
			else if (i > RPM_Int)//Background pixel
			{
				if (DesignModel & MARKERS_ENABLED && i == Sections[j].Begin)
				{
					RGBValueHelper.SetHSV(Sections[j].MarkerColour.h, Sections[j].MarkerColour.s, Sections[j].MarkerColourBrightness);
				}
				else if (DesignModel & BACKGROUND_ENABLED)
				{
					RGBValueHelper.SetHSV(Sections[j].BackgroundColour.h, Sections[j].BackgroundColour.s, Sections[j].BackgroundColourBrightness);
				}
				else
				{
					RGBValueHelper.SetHSV(ColourClear.h, ColourClear.s, ColourClear.v);
				}
			}
			else //Overflow pixel
			{
				ProgressHelper = map(RPM_Latest - (RPM_Int * RPM_Per_Led), 0, RPM_Per_Led, 0, UINT8_MAXVALUE);
				if (DesignModel & SUB_PIXEL_ENABLED)
				{
					if (DesignModel & MARKERS_ENABLED && i == Sections[j].Begin)//Overflow on marker pixel, slightly different maths.
					{
						RGBValueHelper.SetHSV(Sections[j].FillColour.h, Sections[j].FillColour.s, map(ProgressHelper,
							0, UINT8_MAXVALUE,
							Sections[j].MarkerColourBrightness, Sections[j].FillColourBrightness));
					}
					else
					{
						RGBValueHelper.SetHSV(Sections[j].FillColour.h, Sections[j].FillColour.s, map(ProgressHelper,
							0, UINT8_MAXVALUE,
							Sections[j].BackgroundColourBrightness, Sections[j].FillColourBrightness));
					}
				}
				else
				{
					RGBValueHelper.SetHSV(Sections[j].FillColour.h, Sections[j].FillColour.s, Sections[j].FillColourBrightness);
				}
			}

			Leds.set_crgb_at(i, RGBValueHelper);
		}
	}
}

void RPMWS2812::SetRangeRPM(const uint16_t aliveRPM, const uint16_t maxRPM)
{
	RPM_Alive = aliveRPM;
	RPM_Alert = maxRPM;
	UpdateSectionsConstant();
}

void RPMWS2812::SetDeadBlink(uint16_t deadBlinkDuration)
{
	DeadBlinkDuration = deadBlinkDuration;
}

void RPMWS2812::SetDeadBlink(cHSV blinkColour, uint16_t deadBlinkDuration)
{
	ColourDeadBlink = blinkColour;
	DeadBlinkDuration = deadBlinkDuration;
}

void RPMWS2812::SetDeadBlink(cHSV blinkColour)
{
	ColourAlertBlink = blinkColour;
}

void RPMWS2812::SetAlertBlink(uint16_t alertBlinkDuration)
{
	AlertBlinkDuration = alertBlinkDuration;
}

void RPMWS2812::SetAlertBlink(cHSV blinkColour, uint16_t alertBlinkDuration)
{
	ColourAlertBlink = blinkColour;
	AlertBlinkDuration = alertBlinkDuration;
}

void RPMWS2812::SetAlertBlink(cHSV blinkColour)
{
	ColourAlertBlink = blinkColour;
}

void RPMWS2812::SetExtendedOverflowRange(uint8_t range)
{
	ExtendedOverflowRange = range;
}

String RPMWS2812::Debug()
{
	String Debug = "DebugRPM- RPM_Latest: ";

	Debug += " RPM_PixelOverflow: ";
	Debug += String(RPM_PixelOverflow, DEC);

	Debug += String(RPM_Latest, DEC)
		+ " RPM_Int: "
		+ String(RPM_Int, DEC)
		+ " RPM_Per_Led: "
		+ String(RPM_Per_Led, DEC)
		+ " RPM_AliveInt: "
		+ String(RPM_AliveInt, DEC)
		+ " RPM_Alert: "
		+ String(RPM_Alert, DEC)
		+ " LED_COUNT: "
		+ String(LedCount, DEC)
		+ " Section count; "
		+ String(SectionCount, DEC);

	for (uint8_t i = 0; i < SectionCount; i++)
	{
		Debug += " Section ";
		Debug += String(i, DEC);
		Debug += " BeginRPM: ";
		Debug += String(Sections[i].BeginRPM, DEC);
		Debug += " EndRPM: ";
		Debug += String(Sections[i].EndRPM, DEC);
		Debug += " BeginRPM: ";
		Debug += String(Sections[i].Begin, DEC);
		Debug += " EndRPM: ";
		Debug += String(Sections[i].End, DEC);
	}

	return Debug;
}