//#include <Arduino_GFX_Library.h>
//#include "src\Display_SSD1331.h"
//#include "src\BackgroundDraw.h"
#include <SPI.h>
#include <math.h>

//Encoder connection pin-out
#define encoder1_clk 3
#define encoder1_dt 4
#define encoder1_sw 2

//Display connection pin-out
#define screen1_dc 8
#define screen1_cs 10
#define screen1_sclk 13
#define screen1_mosi 11
#define screen1_rst 9


#define BLACK 0x0000
#define GREEN 0x07E0
#define WHITE 0xFFFF  
#define ERASE_COLOR 0x0000

#define LANDSCAPE 0
#define PORTRAIT 1
#define INVERTED_LANDSCAPE 2
#define INVERTED_PORTRAIT 3


//Adafruit_SSD1331 display = Adafruit_SSD1331(cs, dc, mosi, sclk, rst);

//Arduino_GFX display driver
//Display_SSD1331 display1_props = Display_SSD1331(screen1_dc, screen1_cs, screen1_sclk, screen1_mosi, screen1_rst);

//BackgroundDraw bg_draw = BackgroundDraw(&display1_props, GREEN);

//Screen orientation properties
int screen_res_h = 64;
int screen_res_v = 96;

//Process Icon properties
int icon_radius = 20; // 40x40 icon size
int icon_center_x = screen_res_h / 2; //draw logic expects vertical symmetry of icon!
int icon_center_y = 38; //can be freely changed, draw logic will calculate derivative constants automatically

//Encoder setup
int encoder_counter = 0;
int step_size = 5;
int currentStateCLK;
int lastStateCLK;
int currentStateDT;
String currentDir = "";
unsigned long lastButtonPress = 0;

/*

//TODO: refactor for variable Text size. Currently optimized for gfx -> text_size 2
void DrawCounterText(int counter, int color) {

    int bottom_edge = screen_res_v - 18;
    int left_edge = screen_res_h / 2 - 18;

    // Counter Centering
    if (counter == 100) {
        gfx->setCursor(left_edge, bottom_edge);
    }
    else if (counter > 9) {
        gfx->setCursor(left_edge + 9, bottom_edge);
    }
    else {
        gfx->setCursor(left_edge + 15, bottom_edge);
    }


    //TODO: Make this an externall function for generic text area;
    //Display refresh of Textbox area
    if (counter >= 25) {// if background above Textbox, use fill with USER_COLOR;
        for (int i = 0; i < 20; i++) {
           gfx->writeFastHLine(left_edge, bottom_edge + i, 35, color);
        }
    }
    else { //if background is crossing Textbox, find division line than fill with USER_COLOR and ERASE_COLOR
        int draw_height = screen_res_v - counter * screen_res_v / 100; //scale to screen size and invert animation relative to Y axis (bottom to top)
        for (int i = 0; i < draw_height - bottom_edge; i++) {
           gfx->writeFastHLine(left_edge, bottom_edge + i, 35, ERASE_COLOR);
        }
        for (int i = 0; i < screen_res_v - draw_height; i++) {
           gfx->writeFastHLine(left_edge, draw_height + i + 1, 35, color);
        }
    }
    gfx->print(counter);
}

//TODO: Makes slightly ovally-shaped circles, needs improvement
int CalculateIconBoundaryAtY(int draw_height) {
    int delta_y, result;
    if (draw_height < icon_center_y) {
        delta_y = icon_center_y - draw_height;
    }
    else {
        delta_y = draw_height - icon_center_y;
    }
    return icon_center_x - 1 - round(sqrt(icon_radius * icon_radius - delta_y * delta_y));
}
*/
int UpdateEncoderValue(int counter) {
    currentStateCLK = digitalRead(encoder1_clk);
    currentStateDT = digitalRead(encoder1_dt);
    //Check for change and act only on rising edge of input (CLK_HIGH), to prevent double triggering
    if (currentStateCLK != lastStateCLK && currentStateCLK == 1) {
        //If DT and CLK do not match on rising edge of CLK, when encoder in turning clockwize
        if (currentStateDT != currentStateCLK) {
            if (counter < 100)  counter += step_size;
            currentDir = "{CW}";
        }
        else {
            if (counter > 0) counter -= step_size;
            currentDir = "{CCW}";
        }
       // Serial.print("Direction: ");
        Serial.println(currentDir);
       //Serial.print(" | Counter: ");
        //Serial.println(counter);
    }
    lastStateCLK = currentStateCLK;
    return counter;
}

void setup() {
    pinMode(encoder1_clk, INPUT_PULLUP);
    pinMode(encoder1_dt, INPUT_PULLUP);
    pinMode(encoder1_sw, INPUT_PULLUP);

    Serial.begin(9600);
    lastStateCLK = digitalRead(encoder1_clk);

   /* gfx->begin();
    gfx->fillScreen(BLACK);
    gfx->setTextSize(2);
    gfx->setTextColor(WHITE);*/
}

void loop() {

    int updated_counter = UpdateEncoderValue(encoder_counter);
    if (updated_counter != encoder_counter) {
        //FastDrawCounter(updated_counter, encoder_counter);
        encoder_counter = updated_counter;
    }
    /*
    int btnState = digitalRead(encoder1_sw);
    if (btnState == LOW) {
        if (millis() - lastButtonPress > 50) {
            Serial.println("Button pressed!");
            lastButtonPress = millis();
        }
    }*/
    delay(1);
}