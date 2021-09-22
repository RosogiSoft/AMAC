#include "Display_SSD1331.h"

Display_SSD1331::Display_SSD1331(int dc, int cs, int sclk, int mosi, int rst)
{
	bus = new Arduino_SWSPI(dc, cs, sclk, mosi, -1 /* MISO */);
	gfx = new Arduino_SSD1331(bus, rst, ScreenOrientation::Portrait);
	screen_res_h = 64;
	screen_res_v = 98;
	sreen_orientation = ScreenOrientation::Portrait;
}

void Display_SSD1331::SetResolution(int screen_res_h, int screen_res_v) {
	this->screen_res_h = screen_res_h;
	this->screen_res_v = screen_res_v;
}

void Display_SSD1331::SetScreenOrientation(ScreenOrientation sreen_orientation) {
	this->sreen_orientation = sreen_orientation;
}

int Display_SSD1331::GetScreenResV(){
	return screen_res_v;
}

int Display_SSD1331::GetScreenResH() {
	return screen_res_h;
}

ScreenOrientation Display_SSD1331::GetScreenOrientation() {
	return sreen_orientation;
}

Arduino_GFX* Display_SSD1331::GetGFXPointer() {
	return gfx;
}

