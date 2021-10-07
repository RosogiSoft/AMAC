//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1331.h>
#include <SPI.h>
#include <math.h>
#include <Arduino_GFX_Library.h>
#include <string.h>

//Encoder connection pin-out
#define CLK 3
#define DT 4
#define SW 2

//SoftwareSPI Display connection pin-out
/*
#define sclk 13
#define mosi 11
#define cs 10
#define rst 9
#define dc 8*/

//HardwareSPI Display connection pin-out
#define sclk 52
#define mosi 51
#define cs 49
#define rst 47
#define dc 48

#define BLACK 0x0000
#define GREEN 0x07E0
#define WHITE 0xFFFF
#define ERASE_COLOR 0x0000

#define LANDSCAPE 0
#define PORTRAIT 1
#define INVERTED_LANDSCAPE 2
#define INVERTED_PORTRAIT 3


//Adafruit_SSD1331 display = Adafruit_SSD1331(cs, dc, mosi, sclk, rst);

//Arduino_DataBus *bus = new Arduino_SWSPI(dc, cs, sclk, mosi, -1 /* MISO */);
Arduino_DataBus* bus = new Arduino_HWSPI(dc, cs);
Arduino_GFX* gfx = new Arduino_SSD1331(bus, rst, INVERTED_PORTRAIT);

//Screen orientation properties
int screen_res_h = 64;
int screen_res_v = 96;

//Process Icon properties
int icon_radius = 20;                  // 40x40 icon size
int icon_center_x = screen_res_h / 2;  //draw logic expects vertical symmetry of icon!
int icon_center_y = 38;                //can be freely changed, draw logic will calculate derivative constants automatically

//Encoder setup
int encoder_counter = 0;
int counter_step = 5;
int currentStateCLK;
int lastStateCLK;
int currentStateDT;
String currentDir = "";
unsigned long lastButtonPress = 0;
unsigned long lastEncoderTurn = 0;


//Latency test
unsigned long timer_1_SendEncoder = 0;
unsigned long timer_2_GetFeedBack = 0;
unsigned long timer_3_RenderTextUpdate = 0;

//Serial I/O
const byte numChars = 64;
char receivedChars[numChars];
char tempChars[numChars];
char strip_name[numChars] = { 0 };
char message_type[numChars] = { 0 };
int strip_vol = 0;
boolean newSerialData = false;


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
    if (counter >= 25) {  // if background above Textbox, use fill with USER_COLOR;
        for (int i = 0; i < 20; i++) {
            gfx->writeFastHLine(left_edge, bottom_edge + i, 35, color);
        }
    }
    else {                                                          //if background is crossing Textbox, find division line than fill with USER_COLOR and ERASE_COLOR
        int draw_height = screen_res_v - counter * screen_res_v / 100;  //scale to screen size and invert animation relative to Y axis (bottom to top)
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
    int start_draw_height = screen_res_v - prev_counter * screen_res_v / 100;  //scale to screen size and invert animation relative to Y axis (bottom to top)
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

void CheckEncoderTurn() {
    //Avoid very fast loop() which can result in multiple readings of same turn
    if (millis() - lastEncoderTurn > 1) {
        currentStateCLK = digitalRead(CLK);
        currentStateDT = digitalRead(DT);
        //Check for change and act only on rising edge of input (CLK_HIGH), to prevent double triggering
        if (currentStateCLK != lastStateCLK && currentStateCLK == 1) {
            //If DT and CLK do not match on rising edge of CLK, when encoder in turning clockwize
            if (currentStateDT != currentStateCLK) {
                currentDir = "{1}";
            }
            else {
                currentDir = "{0}";
            }
            timer_1_SendEncoder = millis();
            Serial.println(currentDir);
        }
        lastStateCLK = currentStateCLK;
        lastEncoderTurn = millis();
    }
}

void setup() {
    pinMode(CLK, INPUT_PULLUP);
    pinMode(DT, INPUT_PULLUP);
    pinMode(SW, INPUT_PULLUP);
    lastStateCLK = digitalRead(CLK);

    Serial.begin(500000);
    Serial.setTimeout(50);
    Serial.println("Serial is open");

    gfx->begin();
    gfx->fillScreen(BLACK);
    gfx->setTextSize(2);
    gfx->setTextColor(WHITE);

    Serial.println("Screen setup complete");
}

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '{';
    char endMarker = '}';
    char rc;

    while (Serial.available() > 0 && newSerialData == false) {
        rc = Serial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0';  // terminate the string
                recvInProgress = false;
                ndx = 0;
                newSerialData = true;
            }
        }
        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

void parseSerialData() {  // split the data into its parts

    char* strtokIndx;                     // this is used by strchr() as an index
    strtokIndx = strtok(tempChars, ":");  // get the first part - strip_name
    strcpy(strip_name, strtokIndx);

    strtokIndx = strtok(NULL, "=");
    strcpy(message_type, strtokIndx);

    strtokIndx = strtok(NULL, "=");
    strip_vol = atoi(strtokIndx);  // convert this part to an integer
}

void showParsedData() {
    Serial.print("{Strip name: ");
    Serial.print(strip_name);
    Serial.println("}");
    Serial.print("{Message type: ");
    Serial.print(message_type);
    Serial.println("}");
    Serial.print("{Strip volume: ");
    Serial.print(strip_vol);
    Serial.println("}");
}

void loop() {

    CheckEncoderTurn();
    //CheckEncoderTurnWithCounter();
    recvWithStartEndMarkers();
    if (newSerialData == true) {
        // this temporary copy is necessary to protect the original data
        //  because strtok() used in parseData() replaces the commas with \0
        strcpy(tempChars, receivedChars);
        parseSerialData();
        //showParsedData();
        newSerialData = false;
        timer_2_GetFeedBack = millis();
    }
    if (strip_vol != encoder_counter) {
        FastDrawCounter(strip_vol, encoder_counter);
        timer_3_RenderTextUpdate = millis();
        encoder_counter = strip_vol;
        /*
            Serial.print("{Serial: ");
            Serial.print(timer_2_GetFeedBack - timer_1_SendEncoder);
            Serial.println("}");

            Serial.print("{Render: ");
            Serial.print(timer_3_RenderTextUpdate - timer_2_GetFeedBack);
            Serial.println("}");

            Serial.print("{Full: ");
            Serial.print(timer_3_RenderTextUpdate - timer_1_SendEncoder);
            Serial.println("}");
            */
    }
    /*
    int btnState = digitalRead(SW);
    if (btnState == LOW) {
      if (millis() - lastButtonPress > 50) {
        Serial.println("Button pressed!");
        lastButtonPress = millis();
      }
    }*/
}