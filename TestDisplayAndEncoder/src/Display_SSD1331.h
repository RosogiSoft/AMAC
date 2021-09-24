// Display_SSD1331.h

#ifndef _DISPLAY_SSD1331_h
#define _DISPLAY_SSD1331_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include  "Arduino_GFX_Library.h"

enum ScreenOrientation {
	Landscape,
	Portrait,
	Landscape_Inverted,
	Portrait_Inverted
};

class Display_SSD1331
{
public:

	Display_SSD1331(int dc, int cs, int sclk, int mosi, int rst);
	void SetResolution(int screen_res_h, int screen_res_v);
	void SetScreenOrientation(ScreenOrientation sreen_orientation);
	ScreenOrientation GetScreenOrientation();
	int GetScreenResV();
	int GetScreenResH();
	Arduino_GFX* GetGFXPointer();

private:
	Arduino_DataBus* bus;
	Arduino_GFX* gfx;
	int screen_res_v;
	int screen_res_h;
	ScreenOrientation sreen_orientation;
};

#endif

