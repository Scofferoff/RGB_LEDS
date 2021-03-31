#include <FastLED.h>
 
#define LED_PIN 7
#define NUM_LEDS 60
#define COLOR_ORDER GRB
#define LED_TYPE WS2812
#define MAX_BRIGHTNESS 255                   // watch the power!
#define GRAVITY           -9.81              // Downward (negative) acceleration of gravity in m/s^2
#define h0                1                  // Starting height, in meters, of the ball (strip length)
#define NUM_BALLS         1                  // Number of bouncing balls you want (recommend < 7, but 20 is fun in its own way)
#define MAX_BALLS         10                  // How many balls do you need?
#define POP_PROBABILITY   45                 // Value from 0 to 100 to determine if a ball pops.
struct CRGB leds[NUM_LEDS];

float h[MAX_BALLS] ;                         // An array of heights
float vImpact0 = sqrt( -2 * GRAVITY * h0 );  // Impact velocity of the ball when it hits the ground if "dropped" from the top of the strip
float vImpact[MAX_BALLS];                   // As time goes on the impact velocity will change, so make an array to store those values
float tCycle[MAX_BALLS];                    // The time since the last time the ball struck the ground
int   pos[MAX_BALLS];                       // The integer position of the dot on the strip (LED index)
long  tLast[MAX_BALLS];                     // The clock time of the last ground strike
float COR[MAX_BALLS];                       // Coefficient of Restitution (bounce damping)

int killMe;                                  // Roullette
bool popped = false;                         // Ball pop status
int bColour[MAX_BALLS] ;                     // Set initial colour for each ball

int currentNumBalls = NUM_BALLS ;                    // counter of number of balls in existence

uint8_t changeBallColour() {
  int colour = random(0,255);
  return colour;
}

void setup() {
  delay(3000);
  LEDS.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(MAX_BRIGHTNESS);
  
  randomSeed(analogRead(0)); // initialise random number seed
  
  for (int i = 0 ; i < MAX_BALLS ; i++) {    // Initialize variables
    tLast[i] = millis();
    h[i] = h0;
    pos[i] = 0;                              // Balls start on the ground
    vImpact[i] = vImpact0;                   // And "pop" up at vImpact0
    tCycle[i] = 0;
    COR[i] = 0.90 - float(i)/pow(MAX_BALLS,2); 
    // initial ball colour
    bColour[i] = changeBallColour();
  }
}

void loop() {
  for (int i = 0 ; i < currentNumBalls ; i++) {
    tCycle[i] =  millis() - tLast[i] ;     // Calculate the time since the last time the ball was on the ground

    // A little kinematics equation calculates positon as a function of time, acceleration (gravity) and intial velocity
    h[i] = 0.5 * GRAVITY * pow( tCycle[i]/1000 , 2.0 ) + vImpact[i] * tCycle[i]/1000;

    if ( h[i] < 0 ) {                      
      h[i] = 0;                            // If the ball crossed the threshold of the "ground," put it back on the ground
      vImpact[i] = COR[i] * vImpact[i] ;   // and recalculate its new upward velocity as it's old velocity * COR
      tLast[i] = millis();

      if ( vImpact[i] < 0.01 ) {
        vImpact[i] = vImpact0; // If the ball is barely moving, "pop" it back up at vImpact0
        bColour[i] = changeBallColour(); // randomise the colour on jump
        
        killMe = random(100);
        
       if (killMe > POP_PROBABILITY && currentNumBalls > 1) { // we always need a ball to generate another
        currentNumBalls--; // makes the last ball suddenly disappear, aka POP!
        // reset the ball, should be a function
        tLast[currentNumBalls] = millis();
        h[currentNumBalls] = h0;
        pos[currentNumBalls] = 0;                              // Balls start on the ground
        vImpact[currentNumBalls] = vImpact0;                   // And "pop" up at vImpact0
        tCycle[currentNumBalls] = 0;
        COR[currentNumBalls] = 0.90 - float(i)/pow(MAX_BALLS,2);
        popped = true;
       } else {
        popped = false; // no pop 
       }
       if (!popped && currentNumBalls < MAX_BALLS) {        
         currentNumBalls++;
       }
      }
    }
    pos[i] = round( h[i] * (NUM_LEDS - 1) / h0);       // Map "h" to a "pos" integer index position on the LED strip
  }

  // Change color of LEDs, then the "pos" LED on
  for (int i = 0 ; i < currentNumBalls ; i++) leds[pos[i]] = CHSV( bColour[i] , 255, 255);
  FastLED.show();
  //Then off for the next loop around
  for (int i = 0 ; i < currentNumBalls ; i++) {
    leds[pos[i]] = CRGB::Black;
  }
}
