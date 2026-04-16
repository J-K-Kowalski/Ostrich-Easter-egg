/*
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * +                                                                    +
 * + RP2040zero_BLINK_NeoPixel.ino                                      +
 * +                                                                    +
 * + ver.-02-                                                           +
 * +                                                                    +
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * (code from original -Blink.ino- --> https://www.arduino.cc/en/Tutorial/BuiltInExamples/Blink)
 *
 * Turns an RGB LED on/off changing colors, repeatedly.
 * souce code modified to run on a RP2040-Zero MCU Board
 *
 * --------------------------------------------------------------------------------------------------------------
 *
 *
 *
 * RP2040 basic tech specs:
 *  - 3V3 supply
 *  - Dual-core Arm Cortex-M0+ 133MHz
 *  - 264KB RAM
 *  - 2MB off-chip Flash memory (support 16MB QSPI bus)
 *  - DMA (Direct Memory Access) controller
 *  - 30 GPIO pins, 4 of which can be used as analog inputs
 *  - 2 x UART, 2 x SPI, 2 x I2C controllers
 *  - 16 PWM channels
 *  - 1 x USB 1.1 & PHY controller with host and device support
 *  - 8 x Programmable Input/Output (PIO) state machines
 *  - USB storage boot mode with UF2 support for drag-and-drop programming
 *  - Addressable GRB LED (WS2812 on GPIO16)
 *    Note: Follow the order of GRB to sent data, the High Bit sent at FIRST.
 *    Composition of 24bit data:
 *     -- -- -- -- -- -- -- -- | -- -- -- -- -- -- -- -- | -- -- -- -- -- -- -- --
 *    |G7|G6|G5|G4|G3|G2|G1|G0|||R7|R6|R5|R4|R3|R2|R1|R0|||B7|B6|B5|B4|B3|B2|B1|B0|
 *     -- -- -- -- -- -- -- -- | -- -- -- -- -- -- -- -- | -- -- -- -- -- -- -- --
 *
 * The Raspberry PI-Pico ADC conversion speed per sample is 2µs that is 500kS/s.
 * The RP2040 microcontroller operates on a 48MHZ clock frequency which comes from USB PLL.
 * So, its ADC takes a 96 CPU clock cycle to perform one conversion.
 * Therefore, the sampling frequency is (96 * 1/48MHz) = (96 * 0.000000020834) = 2µs per sample (500kS/s).
 *
*/

#include  <Adafruit_NeoPixel.h>


// How many NeoPixels are attached to the Arduino?
const uint8_t LED_COUNT  =  16; //60
// Note: NeoPixel brightness, 0 (min) to 255 (max)
const uint8_t BRIGHTNESS = 255; // Set BRIGHTNESS  (0 .. 255)
const uint8_t LED_PIN    = 29; // RP2040-Zero GPIO of onboard WS2812 RGB LED


// Declare our NeoPixel strip object:
// Note: on - Waveshare rp2040 Zero- a WS2812 RGB LED is connected from DIN(WS2812) to GPIO16(MCU)
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
//Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)



//
// -----  START of functions Prototypes declarations --------------------------------
//
void    colorWipe(uint32_t, int);
void    whiteOverRainbow(uint32_t, int);
void    pulseWhite(uint8_t, uint16_t);
void    rainbowFade2White(int, uint32_t, int);
//
// -----  END of functions Prototypes declarations --------------------------------
//


void colorWipe(uint32_t color, int wait) {
// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
  //
  for (int i = 0; i < strip.numPixels(); i++) { // For each pixel in strip...
    //
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    //
    delay(wait);                           //  Pause for a moment
    //
  } // for
  //
} // colorWipe()

void whiteOverRainbow(uint32_t whiteSpeed, int whiteLength) {
  //
  if (whiteLength >= strip.numPixels()) whiteLength = strip.numPixels() - 1;
  //
  int head = whiteLength - 1;
  int tail = 0, loopNum  = 0, loops = 3;
  //
  uint32_t lastTime = millis(), firstPixelHue = 0;
  //
  for (;;) { // #1
    // Repeat forever (or until a 'break' or 'return')
    for (int i = 0; i < strip.numPixels(); i++) { // #2
      // For each pixel in strip...
      if ( ((i >= tail)   && (i <=  head)) || // If between head & tail...
           ((tail > head) && ((i >= tail)  || (i <=  head)))
         ) {
        //
        strip.setPixelColor(i, strip.Color(0, 0, 0, 255)); // Set white
        //
      } else {                                             // else set rainbow
        //
        int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
        strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
        //
      } // if-else
      //
    } // for #2
    //
    strip.show(); // Update strip with new contents
    // There's no delay here, it just runs full-tilt until the timer and
    // counter combination below runs out.
    firstPixelHue += 40; // Advance just a little along the color wheel
    //
    if ((millis() - lastTime) > whiteSpeed) { // Time to update head/tail?
      //
      if (++head >= strip.numPixels()) {      // Advance head, wrap around
        //
        head = 0;
        if (++loopNum >= loops) return;
        //
      } // if
      //
      if (++tail >= strip.numPixels()) tail = 0; // Advance tail, wrap around
      //
      lastTime = millis();                       // Save time of last movement
      //
    } // if
    //
  } // for #1
  //
} // whiteOverRainbow()

void pulseWhite(uint8_t wait, uint16_t iPause) {
  //
  for (int j = 0; j < 256; j++) { // Ramp up from 0 to 255
    // Fill entire strip with white at gamma-corrected brightness level 'j':
    strip.fill(strip.Color(0, 0, 0, strip.gamma8(j)));
    strip.show();
    //
    delay(wait);
    //
  } // for
  //
  delay(iPause);
  //
  for (int j = 255; j >= 0; j--) { // Ramp down from 255 to 0
    //
    strip.fill(strip.Color(0, 0, 0, strip.gamma8(j)));
    strip.show();
    //
    delay(wait);
    //
  } // for
  //
} // void pulseWhite()

void rainbowFade2White(int wait, uint32_t rainbowLoops, int whiteLoops) {
  //
  int fadeVal = 0, fadeMax = 100;
  // Hue of first pixel runs 'rainbowLoops' complete loops through the color
  // wheel. Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to rainbowLoops*65536, using steps of 256 so we
  // advance around the wheel at a decent clip.
  for (uint32_t firstPixelHue = 0; firstPixelHue < (rainbowLoops * 65536); firstPixelHue += 256) { // #1
    //
    for (int i = 0; i < strip.numPixels(); i++) { // #2
      // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      uint32_t pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the three-argument variant, though the
      // second value (saturation) is a constant 255.
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue, 255, 255 * fadeVal / fadeMax)));
      //
    } // for #2
    //
    strip.show();
    delay(wait);
    //
    if (firstPixelHue < 65536) {
      // First loop,
      if (fadeVal < fadeMax) fadeVal++; // fade in
      //
    } else if (firstPixelHue >= ((rainbowLoops - 1) * 65536)) {
      // Last loop,
      if (fadeVal > 0) fadeVal--; // fade out
      //
    } else {
      // Interim loop, make sure fade is at max
      fadeVal = fadeMax;
      //
    } // if-else
    //
  } // for #1
  //
  for (int k = 0; k < whiteLoops; k++) {
    //
    pulseWhite(5, 100); // Pause 1 second between LED ramps
    delay(10); // Pause 1 second at the end
    //
  } // for
  //
  //delay(200); // Pause 1/2 second
  //
} // rainbowFade2White()


// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =  =


void setup() {
  //
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  //
  strip.setBrightness(BRIGHTNESS);
  //
} // setup()


void loop() {
  // Fill along the length of the strip in various colors...
  //
  colorWipe(strip.Color(255,   0,   0), 100); // Red
  //delay(200); // Pause 1/2 second
  //
  colorWipe(strip.Color(  0, 255,   0), 100); // Green
  //delay(200); // Pause 1/2 second
  //
  colorWipe(strip.Color(  0,   0, 255), 100); // Blue
  //delay(200); // Pause 1/2 second
  //
  // not usable with RP2040-Zero:
  //colorWipe(strip.Color(  0,   0,   0, 255), 50); // True white (not RGB white)
  //delay(500); // Pause 1/2 second
  //
  whiteOverRainbow(75, 5);
  //
  pulseWhite(5, 200);
 //delay(200); // Pause 1/2 second
  //
  rainbowFade2White(3, 3, 1);
  //
} // loop()
