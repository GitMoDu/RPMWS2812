// FastColour.h

#ifndef _FASTCOLOUR_h
#define _FASTCOLOUR_h



#include <Arduino.h>
#include <WS2812.h>

#define COLOUR_NONE {0,0,0}

struct cHSV {
	int h;
	uint8_t s;
	uint8_t v;

	cHSV()
	{
		h = 0;
		s = 0;
		v = 0;
	}
	cHSV(int _h, uint8_t _s, uint8_t _v)
	{
		h = _h;
		s = _s;
		v = _v;
	}
};

#endif

