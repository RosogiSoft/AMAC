#include <SPI.h>
#include <math.h>
#include <Arduino_GFX_Library.h>

//Encoder connection pin-out
#define CLK 3
#define DT 4
#define SW 2

//Display connection pin-out
#define sclk 13
#define mosi 11
#define cs 10
#define rst 9
#define dc 8

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
Arduino_DataBus* bus = new Arduino_SWSPI(dc, cs, sclk, mosi, -1 /* MISO */);
Arduino_GFX* gfx = new Arduino_SSD1331(bus, rst, PORTRAIT);

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

//Arduino_GFX BLOCK

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

int FillRowAndAvoidIcon(int draw_height, int color) {
    if (draw_height > (icon_center_y - icon_radius) && draw_height < (icon_center_y + icon_radius)) {
        int icon_boundary = CalculateIconBoundaryAtY(draw_height);
        gfx->writeFastHLine(1, draw_height, icon_boundary, color);
        gfx->writeFastHLine(screen_res_h - icon_boundary - 1, draw_height, icon_boundary + 1, color);
    }
    else {
        gfx->writeFastHLine(1, draw_height, screen_res_h, color);
    }
}

void FastDrawCounter(int updated_counter, int prev_counter) {
    gfx->startWrite();
    int start_draw_height = screen_res_v - prev_counter * screen_res_v / 100; //scale to screen size and invert animation relative to Y axis (bottom to top)
    if (updated_counter > prev_counter) {
        for (int i = 0; i < updated_counter - prev_counter + 1; i++) {
            FillRowAndAvoidIcon(start_draw_height - i + 1, GREEN);
        }
    }
    else {
        for (int i = 0; i < prev_counter - updated_counter + 1; i++) {
            FillRowAndAvoidIcon(start_draw_height + i - 1, ERASE_COLOR);
        }
    }
    DrawCounterText(updated_counter, GREEN);
    gfx->endWrite();
}

int UpdateEncoderValue(int counter) {
    currentStateCLK = digitalRead(CLK);
    currentStateDT = digitalRead(DT);
    //Check for change and act only on rising edge of input (CLK_HIGH), to prevent double triggering
    if (currentStateCLK != lastStateCLK && currentStateCLK == 1) {
        //If DT and CLK do not match on rising edge of CLK, when encoder in turning clockwize
        if (currentStateDT != currentStateCLK) {
            if (counter < 100)  counter += step_size;
            currentDir = "CW";
        }
        else {
            if (counter > 0) counter -= step_size;
            currentDir = "CCW";
        }
        Serial.print("Direction: ");
        Serial.print(currentDir);
        Serial.print(" | Counter: ");
        Serial.println(counter);
    }
    lastStateCLK = currentStateCLK;
    return counter;
}

void setup() {
    pinMode(CLK, INPUT_PULLUP);
    pinMode(DT, INPUT_PULLUP);
    pinMode(SW, INPUT_PULLUP);

    Serial.begin(9600);
    lastStateCLK = digitalRead(CLK);

    gfx->begin();
    gfx->fillScreen(BLACK);
    gfx->setTextSize(2);
    gfx->setTextColor(WHITE);
}

void loop() {

    int updated_counter = UpdateEncoderValue(encoder_counter);
    if (updated_counter != encoder_counter) {
        FastDrawCounter(updated_counter, encoder_counter);
        encoder_counter = updated_counter;
    }

    int btnState = digitalRead(SW);
    if (btnState == LOW) {
        if (millis() - lastButtonPress > 50) {
            Serial.println("Button pressed!");
            lastButtonPress = millis();
        }
    }
    delay(1);
}