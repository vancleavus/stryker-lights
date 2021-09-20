#include <FastLED.h>

/**
 * TODO:
 *    sliders
 *    hardware

**/

#define NUM_LEDS 900
#define LEDPIN 56

#define NUM_LED_COLUMNS (4)
#define NUM_LED_ROWS (4)
#define NUM_BTN_COLUMNS (4)
#define NUM_BTN_ROWS (4)
#define NUM_COLORS (3)

#define MAX_DEBOUNCE (3)

#define BUFFER_SIZE 16

#define SLIDER_R A5
#define SLIDER_G A6
#define SLIDER_B A7

#define SCAN_DELTA 1
#define STRIP_REPS_PER_SCAN 5
#define RNBW_REPS_PER_STRIP 8


CRGB leds[NUM_LEDS];

CRGB ButtonLEDs[NUM_LED_COLUMNS][NUM_LED_ROWS];
uint8_t buttonSelect=13; //0-15

CRGB rnbw[1];
uint8_t hue;

uint16_t sliderReads[3][BUFFER_SIZE];
uint16_t bufferPointer;

unsigned long nextLoop;
uint16_t stripCount,rnbwCount;


const uint8_t btnselpins[4]  = {41,45,49,53};
//                              grn,ylw,blu,wht
const uint8_t btnreadpins[4] = {37,36,33,32};
//                              blk,ylw,grn,blu
const uint8_t ledselpins[4]  = {40,44,48,52};
//                              grn,ylw,blu,wht

const uint8_t colorpins[4][3] = {{4,3,2},{7,6,5},{10,9,8},{13,12,11}};
//                              Red1,Grn1... 1 has blk btnread
int8_t debounce_count[NUM_BTN_COLUMNS][NUM_BTN_ROWS];
uint8_t current=0;

void setupButtonColors()
{
  CRGB btnleds[12];
  fill_rainbow(btnleds,12,0,21);
  for(int i=0; i<12; i++){
    btnleds[i].r=btnleds[i].r;
    btnleds[i].g=btnleds[i].g;
    btnleds[i].b=btnleds[i].b;
    ButtonLEDs[i/4][i%4]=btnleds[i];
  }
  ButtonLEDs[3][0]=CRGB(200,200,160);
  //12 = White
  //13 = Sliders
  //14 = One Rainbow across strip
  //15 = One color, progress through rainbow
}

void setupCircularBuffer(){
  bufferPointer=0;
  for(int i=0; i<BUFFER_SIZE; i++){
    sliderReads[0][i]=0;
    sliderReads[1][i]=0;
    sliderReads[2][i]=0;
  }
}

void setupPins(){
  uint8_t i;

    // initialize
    // select lines
    for(i = 0; i < NUM_LED_COLUMNS; i++)
    {
        pinMode(ledselpins[i], OUTPUT);

        // with nothing selected by default
        digitalWrite(ledselpins[i], HIGH);
    }

    for(i = 0; i < NUM_BTN_COLUMNS; i++)
    {
        pinMode(btnselpins[i], OUTPUT);

        // with nothing selected by default
        digitalWrite(btnselpins[i], HIGH);
    }

    // key return lines
    for(i = 0; i < 4; i++)
    {
        pinMode(btnreadpins[i], INPUT_PULLUP);
    }

    // LED drive lines
    for(i = 0; i < NUM_LED_ROWS; i++)
    {
        for(uint8_t j = 0; j < NUM_COLORS; j++)
        {
            pinMode(colorpins[i][j], OUTPUT);
            digitalWrite(colorpins[i][j], LOW);
        }
    }

    for(uint8_t i = 0; i < NUM_BTN_COLUMNS; i++)
    {
        for(uint8_t j = 0; j < NUM_BTN_ROWS; j++)
        {
            debounce_count[i][j] = 0;
        }
    }
}

void scanButtons(){

  uint8_t val;
  uint8_t i, j;

    //run
    digitalWrite(btnselpins[current], LOW);
    digitalWrite(ledselpins[current], LOW);

    for(i = 0; i < NUM_LED_ROWS; i++)
    {
      analogWrite(colorpins[i][0], ButtonLEDs[current][i].r);
      analogWrite(colorpins[i][1], ButtonLEDs[current][i].g);
      analogWrite(colorpins[i][2], ButtonLEDs[current][i].b);
//        uint8_t val = (LED_outputs[current][i] & 0x03);
//
//        if(val)
//        {
//            digitalWrite(colorpins[i][val-1], HIGH);
//        }
  }


  delay(2);

  for( j = 0; j < NUM_BTN_ROWS; j++)
  {
    val = digitalRead(btnreadpins[j]);

    if(val == LOW)
    {
      // active low: val is low when btn is pressed
      if( debounce_count[current][j] < MAX_DEBOUNCE)
      {
        debounce_count[current][j]++;
        if( debounce_count[current][j] == MAX_DEBOUNCE )
        {
          buttonSelect=(current * NUM_BTN_ROWS) + j;
          //Serial.println("Button press: " + String(buttonSelect));
          //Serial.println("\tR: " + String(ButtonLEDs[current][j].r) + "\tG: " + String(ButtonLEDs[current][j].g) + "\tB: " + String(ButtonLEDs[current][j].b));
//          Serial.print("Key Down ");
//          Serial.println((current * NUM_BTN_ROWS) + j);

//          LED_outputs[current][j]++;
        }
      }
    }
    else
    {
      // otherwise, button is released
      if( debounce_count[current][j] > 0)
      {
        debounce_count[current][j]--;
      }
    }
  }// for j = 0 to 3;

  delay(2);

  digitalWrite(btnselpins[current], HIGH);
  digitalWrite(ledselpins[current], HIGH);

  for(i = 0; i < NUM_LED_ROWS; i++)
  {
    for(j = 0; j < NUM_COLORS; j++)
    {
      digitalWrite(colorpins[i][j], LOW);
    }
  }

  current++;
  if (current >= NUM_BTN_COLUMNS)
  {
    current = 0;
  }
}

void updateCustomColor(){
  uint32_t rSum=0;
  uint32_t gSum=0;
  uint32_t bSum=0;
  uint16_t i;
  uint16_t rFinal=0;
  uint16_t gFinal=0;
  uint16_t bFinal=0;
  
  sliderReads[0][bufferPointer]=analogRead(SLIDER_R);
  sliderReads[1][bufferPointer]=analogRead(SLIDER_G);
  sliderReads[2][bufferPointer]=analogRead(SLIDER_B);

  bufferPointer++;
  if(bufferPointer>=BUFFER_SIZE) bufferPointer=0;
  
  for(i=0; i<BUFFER_SIZE; i++){
    rSum+=sliderReads[0][i];
    gSum+=sliderReads[1][i];
    bSum+=sliderReads[2][i];
  }
  rSum=rSum/BUFFER_SIZE;
  gSum=gSum/BUFFER_SIZE;
  bSum=bSum/BUFFER_SIZE;
  
  rFinal=map(rSum,0,1023,0,255);
  gFinal=map(gSum,0,1023,0,255);
  bFinal=map(bSum,0,1023,0,255);
  
  ButtonLEDs[3][1]=CRGB(rFinal,gFinal,bFinal);
  
}

void rotateRainbow() {
  CRGB rnbw[1];
  hue++;
  fill_rainbow(rnbw,1,hue,2);
  //bad fix but it works
  if(rnbw[0].r==160 && rnbw[0].g==0 && rnbw[0].b==0){
    rnbw[0].g=150;
  }
  ButtonLEDs[3][2]=rnbw[0];
  ButtonLEDs[3][3]=rnbw[0];

}

void progressLEDStrip() {
  CRGB nextColor=ButtonLEDs[buttonSelect/4][buttonSelect%4];
  nextColor.r = nextColor.r/2;
  nextColor.g = nextColor.g/2;
  nextColor.b = nextColor.b/2;
  if(buttonSelect==13 || buttonSelect==15){
    for(int i=0; i<NUM_LEDS; i++){
      leds[i] = nextColor;
    }
  }
  else {
    for(int i=NUM_LEDS-1; i>0; i--){
      leds[i] = leds[i-1];
    }
    leds[0] = nextColor;
  }
  FastLED.show();
}



void setup() {
  Serial.begin(9600);
  FastLED.addLeds<NEOPIXEL, LEDPIN>(leds,NUM_LEDS);
  
  for(int i=0; i<NUM_LEDS; i++){
    leds[i]=CRGB(0,0,0);
  }
  FastLED.show();
  setupButtonColors();
  setupCircularBuffer();
  setupPins();
  nextLoop = millis() + SCAN_DELTA;
}

void loop() {
  if(millis() >= nextLoop){
    nextLoop = millis()+SCAN_DELTA;
    scanButtons();
    
    if(stripCount++ >= STRIP_REPS_PER_SCAN){
      stripCount=0;
      progressLEDStrip();
      
      if(rnbwCount++ >= RNBW_REPS_PER_STRIP){
        rnbwCount = 0;
        rotateRainbow();
        updateCustomColor();
      }
    }
  }
}
