// BackgroundDraw.h

#ifndef _BACKGROUNDDRAW_h
#define _BACKGROUNDDRAW_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include <src/Display_SSD1331.h>

class BackgroundDraw{
public:
	BackgroundDraw(Display_SSD1331* display_properties, int color);
	BackgroundDraw(Display_SSD1331* display_properties, int color, int volume);
	void SetVolume(int volume);
	void SetColor(int color);
	void RefreshfullBG();

private:
	Display_SSD1331* display_propeties;
	int color;
	int volume;
	
	void FillRowAndAvoidIcon(int draw_height, int color);
};

#endif

