/// 
///  Created for personal use, use it at your own risk and benefit.
///  Based on the excellent work by Kasper Kamperman(https://www.kasperkamperman.com)
///  Depends on Light_WS2812 https://github.com/cpldcpu/light_ws2812
/// 

#include "RPMWS2812.h"

bool RPMWS2812::Begin()
{
	Leds.setOutput(LED_DATA_PIN);
	Leds.setColorOrderRGB();

	Clear();

	return true;
}

void RPMWS2812::AddLogger(NeoSWSerial* logger)
{
	Logger = logger;
	Log = true;
}

void RPMWS2812::BootAnimation(cHSV colourBoot)
{
	uint16_t BootDelayStep = BOOT_DURATION / 4;

	cHSV ColourBootStart, ColourBootMiddle, ColourBootPeak;

	ColourBootStart = cHSV(colourBoot.h, colourBoot.s, 0);
	ColourBootMiddle = cHSV(colourBoot.h, colourBoot.s, ((GlobalBrightness / 4)*colourBoot.v) / 255);
	ColourBootPeak = cHSV(colourBoot.h, colourBoot.s, ((GlobalBrightness)*colourBoot.v) / 255);

	Clear();
	SetAllHSV(ColourBootStart);
	Show();
	delay(BootDelayStep / 4);
	DirectAnimate(ColourBootStart, ColourBootPeak, BootDelayStep);
	DirectAnimate(ColourBootPeak, ColourBootMiddle, BootDelayStep);
	DirectAnimate(ColourBootMiddle, ColourBootStart, BootDelayStep);
	delay((BootDelayStep * 3) / 4);
}

void RPMWS2812::Show()
{
	Leds.sync();
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
	GlobalBrightness = brightness;
}

void RPMWS2812::Clear()
{
	SetAllHSV(ColourClear);
	Show();
}

void RPMWS2812::SetAll(cRGB colour)
{
	Set(colour, 0, LED_COUNT);
}

void RPMWS2812::SetAllHSV(cHSV colour)
{
	cRGB ColourRGB;
	ColourRGB.SetHSV(colour.h, colour.s, colour.v);
	Set(ColourRGB, 0, LED_COUNT);
}

void RPMWS2812::Set(const cRGB colour, const  uint8_t startIndex, const uint8_t endIndex)
{
	for (uint8_t i = startIndex; i < endIndex; i++)
	{
		Leds.set_crgb_at(i, colour);
	}
}

void RPMWS2812::SetRangeRPM(const uint16_t aliveRPM, const uint16_t maxRPM)
{
	RPM_Alive = aliveRPM;
	RPM_Alert = maxRPM;
	UpdateRPM(0);
	UpdateRPMConstants();
}

void RPMWS2812::SetDeadBlink(uint16_t deadBlinkDuration, cHSV blinkColour, uint16_t wakeupFadeDuration = 0)
{
	ColourDeadBlink = blinkColour;
	DeadBlinkDuration = deadBlinkDuration;
	WakeupFadeDuration = wakeupFadeDuration;
}

void RPMWS2812::SetAlertBlink(uint16_t alertBlinkDuration, cHSV blinkColour)
{
	ColourAlertBlink = blinkColour;
	AlertBlinkDuration = alertBlinkDuration;
}

void RPMWS2812::UpdateConstants()
{
	UpdateRPMConstants();
}

void RPMWS2812::UpdateRPMConstants()
{
	RPM_Per_Led = RPM_Alert / LED_COUNT;
	RPM_AliveInt = RPM_Alive / RPM_Per_Led;
	for (uint8_t i = 0; i < SectionCount; i++)
	{
		Sections[i].Begin = Sections[i].BeginRPM / RPM_Per_Led;
		Sections[i].End = Sections[i].EndRPM / RPM_Per_Led;

		Sections[i].OverflowRangeBackground = (byte)min(254, max(0, GlobalBrightness - Sections[i].BackgroundColour.v));
		Sections[i].OverflowRangeMarker = (byte)min(254, max(0, GlobalBrightness - Sections[i].MarkerColour.v));
	}
}

void RPMWS2812::UpdateRPM(const uint16_t rpm, const bool autoRefresh = true)
{
	bool Changed = RPM_Latest != rpm;

	RPM_Latest = min(rpm, RPM_Alert);

	RPM_Int = RPM_Latest / RPM_Per_Led;

	UpdateSections();

	if (autoRefresh && Changed)
	{
		Show();
	}
}


void RPMWS2812::SetSection(const uint16_t beginRPM, const uint16_t endRPM,
	const cHSV fillColour, const  cHSV backgroundColour, const  cHSV markerColour)
{
	LedSection NewSection;
	NewSection.Set(beginRPM, endRPM, fillColour, backgroundColour, markerColour);
	Sections[SectionCount++] = NewSection;
	UpdateRPMConstants();
}

void RPMWS2812::ClearSections()
{
	SectionCount = 0;
}

void RPMWS2812::SetDesignModel(byte designModel)
{
	DesignModel = designModel;
	UpdateSections();
}

void RPMWS2812::UpdateSections()
{

	if (DesignModel & SUB_PIXEL_ENABLED)
	{
		RPM_FloorScaled = RPM_Int * RPM_Per_Led;
		RPM_Overflow = RPM_Latest - RPM_FloorScaled;
	}

	for (uint8_t j = 0; j < SectionCount; j++)
	{
		for (uint8_t i = Sections[j].Begin; i < Sections[j].End; i++)
		{
			if (DesignModel & MARKERS_ENABLED && i == Sections[j].Begin && RPM_Int < Sections[j].Begin)
			{
				RGBValue.SetHSV(Sections[j].MarkerColour.h, Sections[j].MarkerColour.s, Sections[j].MarkerColour.v);
			}
			else if (i < RPM_Int)
			{
				RGBValue.SetHSV(Sections[j].FillColour.h, Sections[j].FillColour.s, GlobalBrightness);
			}
			else if (i > RPM_Int)
			{
				RGBValue.SetHSV(Sections[j].BackgroundColour.h, Sections[j].BackgroundColour.s, Sections[j].BackgroundColour.v);
			}
			else //Overflow pixel
			{
				if (DesignModel & SUB_PIXEL_ENABLED)
				{
					if (DesignModel & MARKERS_ENABLED && i == Sections[j].Begin)//Overflow on marker pixel, slightly different maths.
					{
						RPM_PixelOverflow = min(254, max(0, (uint8_t)((RPM_Overflow * Sections[j].OverflowRangeMarker) / RPM_Per_Led)));
						RGBValue.SetHSV(Sections[j].FillColour.h, Sections[j].FillColour.s, Sections[j].MarkerColour.v + RPM_PixelOverflow);
					}
					else
					{
						RPM_PixelOverflow = min(254, max(0, (uint8_t)((RPM_Overflow * Sections[j].OverflowRangeBackground) / RPM_Per_Led)));
						RGBValue.SetHSV(Sections[j].FillColour.h, Sections[j].FillColour.s, Sections[j].BackgroundColour.v + RPM_PixelOverflow);
					}
				}
				else
				{
					RGBValue.SetHSV(Sections[j].FillColour.h, Sections[j].FillColour.s, GlobalBrightness);
				}
			}

			Leds.set_crgb_at(i, RGBValue);
		}
	}
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
		+ String(LED_COUNT, DEC)
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


