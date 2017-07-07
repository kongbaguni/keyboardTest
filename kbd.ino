/* Simple "Hello World" example.

   After uploading this to your board, use Serial Monitor
   to view the message.  When Serial is selected from the
   Tools > USB Type menu, the correct serial port must be
   selected from the Tools > Serial Port AFTER Teensy is
   running this code.  Teensy only becomes a serial device
   while this code is running!  For non-Serial types,
   the Serial port is emulated, so no port needs to be
   selected.

   This example code is in the public domain.
*/
#include <FastLED.h>

const int row[5] = {0,1,2,3,4};
const int col[12] = {5, 6, 7, 8, 9, 10, 17,18,19,20,21,22};
#define led     13
#define ledOut  14
#define RGBPower 16
#define LED_PIN  15
#define COLOR_ORDER GRB
#define CHIPSET     WS2811
#define NUM_LEDS    13
#define BRIGHTNESS  200
#define FRAMES_PER_SECOND 60

//눌러진 버튼 위치값 저장
int keyRead[12][5];
//눌러진 버튼의 배열
int btnIds[60];
int ledCount = 0;

bool gReverseDirection = false;
CRGB leds[NUM_LEDS];

void setup() {
  delay(3000); // sanity delay
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( BRIGHTNESS );

  for (int i = 0; i < 5; i++) {
    pinMode(row[i],INPUT);
  }
  for (int i = 0; i < 12; i++) {
    pinMode(col[i],OUTPUT);
  }
  pinMode(led,OUTPUT);
  pinMode(RGBPower,OUTPUT);
  pinMode(ledOut, OUTPUT);
  Serial.begin(9600); // USB is always 12 Mbit/sec
}

void loop() {
  checkKeyboard();
  digitalWrite(led,HIGH);
  digitalWrite(RGBPower,HIGH);
  ledBackLight();
  printKeyStatus();
  
  Fire2012(); // run simulation frame
  
  FastLED.show(); // display this frame
  FastLED.delay(1000 / FRAMES_PER_SECOND);

}

void checkKeyboard() {
  for (int i = 0; i < 12; i ++) {
    delay(12);
    digitalWrite(col[i],LOW);
    for(int j = 0; j < 5; j ++) {
      int result = digitalRead(row[j]);
      keyRead[i][j] = result;
    }
    digitalWrite(col[i],HIGH);
  }
}

void printKeyStatus() {
  Serial.println("===================");  
  int count = 0;
  for(int j = 0; j < 5; j ++) {
    for (int i = 0; i < 12; i ++) {
      if(keyRead[i][j] == LOW) {
          Serial.print("[#]");
          btnIds[count] = j*12 + i;
          count += 1;
        }
      else {
        Serial.print("___");
      }      
    }
    Serial.println(" ");
   }
  Serial.println("===================");  
  for (int i=0; i<count; i++) {
    Serial.print(btnIds[i]);
    Serial.print(" ");
  }
  Serial.println(" ");
}


void ledBackLight() {
  ledCount += 5;
  analogWrite(ledOut,ledCount % 255);
}


// Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
//// 
// This basic one-dimensional 'fire' simulation works roughly as follows:
// There's a underlying array of 'heat' cells, that model the temperature
// at each point along the line.  Every cycle through the simulation, 
// four steps are performed:
//  1) All cells cool down a little bit, losing heat to the air
//  2) The heat from each cell drifts 'up' and diffuses a little
//  3) Sometimes randomly new 'sparks' of heat are added at the bottom
//  4) The heat from each cell is rendered as a color into the leds array
//     The heat-to-color mapping uses a black-body radiation approximation.
//
// Temperature is in arbitrary units from 0 (cold black) to 255 (white hot).
//
// This simulation scales it self a bit depending on NUM_LEDS; it should look
// "OK" on anywhere from 20 to 100 LEDs without too much tweaking. 
//
// I recommend running this simulation at anywhere from 30-100 frames per second,
// meaning an interframe delay of about 10-35 milliseconds.
//
// Looks best on a high-density LED setup (60+ pixels/meter).
//
//
// There are two main parameters you can play with to control the look and
// feel of your fire: COOLING (used in step 1 above), and SPARKING (used
// in step 3 above).
//
// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100 
#define COOLING  55

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 120


void Fire2012()
{
// Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      CRGB color = HeatColor( heat[j]);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUM_LEDS-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
}

