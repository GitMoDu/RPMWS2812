// LedSection.h

#ifndef _LEDSECTION_h
#define _LEDSECTION_h

#define USE_HSV
#include <Arduino.h>
#include "FastColour.h"


struct LedSection {

	uint8_t Begin = 0;
	uint8_t End = 0;

	uint16_t BeginRPM = 0;
	uint16_t EndRPM = 0;

	uint8_t OverflowRangeBackground, OverflowRangeMarker;

	cHSV FillColour, BackgroundColour, MarkerColour;
	void Set(const uint16_t beginRPM, const  uint16_t endRPM,
		const cHSV fillColour,
		const cHSV backgroundColour = COLOUR_NONE,
		const cHSV markerColour = COLOUR_NONE) 
	{
			BeginRPM = beginRPM;
			EndRPM = endRPM;
		
			FillColour = fillColour;
			BackgroundColour = backgroundColour;
			MarkerColour = markerColour;
	}
};


#endif

