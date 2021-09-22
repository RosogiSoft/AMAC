#include "BackgroundDraw.h"


BackgroundDraw::BackgroundDraw(Display_SSD1331* display_properties, int color) {
    this->display_propeties = display_propeties;
	this->color = color;
    volume = 0;
}

BackgroundDraw::BackgroundDraw(Display_SSD1331* display_properties, int color, int volume) {
    this->display_propeties = display_propeties;
    this->color = color;
    this->volume = volume;
}

void BackgroundDraw::SetVolume(int new_volume) {
    Arduino_GFX* gfx = display_propeties->GetGFXPointer();
    int start_draw_height = screen_res_v - volume * screen_res_v / 100; //scale to screen size and invert animation relative to Y axis (bottom to top)
    if (new_volume > volume) {
        for (int i = 0; i < new_volume - volume + 1; i++) {
            FillRowAndAvoidIcon(start_draw_height - i + 1, GREEN);
        }
    }
    else {
        for (int i = 0; i < volume - new_volume + 1; i++) {
            FillRowAndAvoidIcon(start_draw_height + i - 1, ERASE_COLOR);
        }
    }
    gfx->endWrite();

}
void BackgroundDraw::SetColor(int color) {

}
void BackgroundDraw::RefreshfullBG() {

}

void BackgroundDraw::FillRowAndAvoidIcon(int draw_height, int color) {
    if (draw_height > (icon_center_y - icon_radius) && draw_height < (icon_center_y + icon_radius)) {
        int icon_boundary = CalculateIconBoundaryAtY(draw_height);
        gfx->writeFastHLine(1, draw_height, icon_boundary, color);
        gfx->writeFastHLine(screen_res_h - icon_boundary - 1, draw_height, icon_boundary + 1, color);
    }
    else {
        gfx->writeFastHLine(1, draw_height, screen_res_h, color);
    }
}

