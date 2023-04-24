
/*

   TPD-B-Projekt "Digitale Sanduhr":

   Projektleitung:
   Theodor Diehl
   Gian Nino Cataffo

   2023, HAW Hamburg

   Link zum Projekt:
   https://github.com/TheDevNino/digital-hourglass-arduino

*/

// Library für die 7-Segmentanzeige
#include <ShiftRegister74HC595.h>

// Library für den IR Empfänger
#include <IRremote.h> 

// Library für die WS2812b LEDs
#include <FastLED.h>


// Grundlage für die gesamte Programmstruktur
enum Modus
{
  PGM_Auswahl,
  PGM_1,
  PGM_2,
  PGM_3
};
Modus aktiverModus = PGM_Auswahl;

// Werte(-bereiche)
int encoderWert[] = {1, 30, 1, 50};
int encoderIntervall[] = {1, 10, 1, 1};
int encoderMin[] = {1, 1, 1, 0};
int encoderMax[] = {3, 900, 5, 100};

// Drehencoder
#define CLK 2
#define DT 3
#define SW 4
int currentStateCLK;
int lastStateCLK;
int lastStateDT;
int buttonState = 0;
int lastButtonState = 0;
unsigned long buttonDownTime = 0;

// 7-Segmentanzeige
#define SDI 5
#define SCLK 6
#define LOAD 7
#define DIGITS 2
ShiftRegister74HC595<DIGITS> myRegister(SDI, SCLK, LOAD);
int digit1, digit2;
uint8_t off[] = {B11111111, B11111111};
uint8_t digits[] = {
    B11000000, // 0
    B11111001, // 1
    B10100100, // 2
    B10110000, // 3
    B10011001, // 4
    B10010010, // 5
    B10000010, // 6
    B11111000, // 7
    B10000000, // 8
    B10010000  // 9
};
// ToDo rotated Digits

// WS2812b

#define LED_PIN     8
#define NUM_LEDS    55
#define BRIGHTNESS  20
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
#define UPDATES_PER_SECOND 100
CRGBPalette16 currentPalette;
TBlendType    currentBlending;
extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

// IR Empfänger
int RECV_PIN = 12;
IRrecv irrecv(RECV_PIN); 
decode_results results; 

// Gyroskop
//  ...

// evtl Beeper

void setup()
{
  //delay( 100 ); // power-up safety delay
  
  // Encoder Pins
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);

  // Setup Serieller Monitor
  Serial.begin(9600);

  // Initialisierung der Encoder
  lastStateCLK = digitalRead(CLK);
  lastStateDT = digitalRead(DT);

  // Initialisierung des Displays
  showNumber(1);

  // Initialisierung des Infrarotempfängers
  irrecv.blink13(true); 
    
  // Initialisierung der LEDs
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
  
  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;
}


void loop()
{
  if (aktiverModus == PGM_1)
  {
    intervallAnpassung();
  }
  encoderAuswerten();
  //IR_Auswerten();

  // Define the LED arrangement in the form of a Sandclock
  // Top Half
  int ledCount = 0;
  for (int i = 7; i >= 1; i--) {
    for (int j = 1; j <= i; j++) {
      leds[ledCount] = CRGB::White;
      ledCount++;
    }
  }

  // Middle LED
  leds[28] = CRGB::White;

  // Bottom Half
  for (int i = 2; i <= 7; i++) {
    for (int j = 1; j <= i; j++) {
      leds[ledCount] = CRGB::White;
      ledCount++;
    }
  }
// Entprellen
  delay(1);
}

void rainbow() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(i * 255 / NUM_LEDS, 255, 255);
  }
  FastLED.show();
  delay(50);
}

void colorWipe(CRGB color, int speed) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
    FastLED.show();
    delay(speed);
  }
}

void theaterChase(CRGB color, int speed) {
  for (int j = 0; j < 10; j++) {
    for (int q = 0; q < 3; q++) {
      for (int i = 0; i < NUM_LEDS; i += 3) {
        leds[i + q] = color;
      }
      FastLED.show();
      delay(speed);
      for (int i = 0; i < NUM_LEDS; i += 3) {
        leds[i + q] = CRGB::Black;
      }
    }
  }
}

void twinkle(CRGB color, int speed) {
  for (int i = 0; i < NUM_LEDS; i++) {
    if (random(100) < 50) {
      leds[i] = color;
    } else {
      leds[i] = CRGB::Black;
    }
  }
  FastLED.show();
  delay(speed);
}

void fadeToBlack() {
  for (int i = 0; i < 255; i++) {
    for (int j = 0; j < NUM_LEDS; j++) {
      leds[j].fadeToBlackBy(1);
    }
    FastLED.show();
    delay(10);
  }
}
void IR_Auswerten(){
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    irrecv.resume();
  }
 
 /* if (irrecv.decode(&results)) {    //Wenn Daten empfangen wurden,
    Serial.println(results.value, DEC); //werden sie als Dezimalzahl (DEC) an den Serial-Monitor ausgegeben.

    for (int i = 0; i <= 9; i++) {
      if (results.value == 16724175) encoderWert[aktiverModus] = 0;
    }
    
    if (results.value == 16724175) encoderWert[aktiverModus] = 0;
    if (results.value == 16724175) encoderWert[aktiverModus] = 1;
    ´if (results.value == 16724175) encoderWert[aktiverModus] = 2;
    if (results.value == 16724175) encoderWert[aktiverWert] = 3;
    if (results.value == 16724175) encoderWert[aktiverWert] = 4;
    if (results.value == 16724175) encoderWert[aktiverWert] = 5;
    if (results.value == 16724175) encoderWert[aktiverWert] = 6;
    if (results.value == 16724175) encoderWert[aktiverWert] = 7;
    if (results.value == 16724175) encoderWert[aktiverWert] = 8;
    if (results.value == 16724175) encoderWert[aktiverWert] = 9;

    if (results.value == 16724175) encoderWert[aktiverModusverModus] += encoderIntervall[aktiverModus];
    if (results.value == 16724175) encoderWert[aktiverModus] -= encoderIntervall[aktiverModus];

*/
  }


void intervallAnpassung()
{
  if (encoderWert[PGM_1] <= 60)
  {
    encoderIntervall[PGM_1] = 1;
  }
  else
  {
    encoderIntervall[PGM_1] = 10;
  }
}

void encoderAuswerten()
{
  // Read the current state of CLK and DT
  int currentStateCLK = digitalRead(CLK);
  int currentStateDT = digitalRead(DT);

  // If last and current state of CLK are different, then pulse occurred
  // React to only 1 state change to avoid double count
  if (currentStateCLK != lastStateCLK && currentStateCLK == 1)
  {
    // Determine the direction of rotation based on the current and last DT state
    if (currentStateDT != currentStateCLK)
    {
      // Clockwise rotation
      encoderWert[aktiverModus] += encoderIntervall[aktiverModus];
    }
    else
    {
      // Counterclockwise rotation
      encoderWert[aktiverModus] -= encoderIntervall[aktiverModus];
    }

    // Clamp the parameter value to the allowed range
    encoderWert[aktiverModus] = constrain(encoderWert[aktiverModus], encoderMin[aktiverModus], encoderMax[aktiverModus]);

    // Print the updated parameter value
    Serial.print("PGM\tVal\tInt\n");
    Serial.print(aktiverModus);
    Serial.print("\t");
    Serial.print(encoderWert[aktiverModus]);
    Serial.print("\t");
    Serial.print(encoderIntervall[aktiverModus]);
    Serial.print("\n---------------\n");

    if (aktiverModus == PGM_1 && encoderWert[aktiverModus] >= 60)
    {
      showNumber(secondsToMinutes(encoderWert[aktiverModus]));
    }
    else
    {
      showNumber(encoderWert[aktiverModus]);
    }
  }

  // Remember last CLK and DT state
  lastStateCLK = currentStateCLK;
  lastStateDT = currentStateDT;

  int buttonState = digitalRead(SW);

  if (buttonState != lastButtonState)
  { // Wenn sich der Status des Buttons ändert
    if (buttonState == LOW)
    {                            // Wenn der Button gedrückt wird
      buttonDownTime = millis(); // Speichern der Startzeit
    }
    else
    {                                                               // Wenn der Button losgelassen wird
      unsigned long buttonUpTime = millis();                        // Speichern der Endzeit
      unsigned long buttonDuration = buttonUpTime - buttonDownTime; // Berechnen der Dauer
      buttonAuswerten(buttonDuration);
    }
  }

  lastButtonState = buttonState; // Speichern des vorherigen Status des Buttons
}

void buttonAuswerten(long duration)
{
  if (duration > 300)
  {
    aktiverModus = 0;
  }
  else if (duration > 30)
  {
    switch (aktiverModus)
    {
    case PGM_Auswahl:
      aktiverModus = encoderWert[aktiverModus];
      showNumber(encoderWert[aktiverModus]);
      break;
    case PGM_1:
      // start timer
      startTimer();
      break;
    case PGM_2:
      // start effekt
      effectProgram();
      break;
    case PGM_3:
      // change kelvin-wert
      break;
    }
  }
}

void effectProgram()
{
  if(encoderWert[aktiverModus]==1)
  {
  // Rainbow effect
  rainbow();
  
  }
  if(encoderWert[aktiverModus]==2)
  {
    // Color wipe effect
  colorWipe(CRGB::Red, 50);
  
  }
  if(encoderWert[aktiverModus]==3)
  {// Theater chase effect
  theaterChase(CRGB::Green, 50);
  
  }
  if(encoderWert[aktiverModus]==4)
  {// Twinkle effect
  twinkle(CRGB::Blue, 50);
  
  }
  if(encoderWert[aktiverModus]==5)
  {// Fade all LEDs to black
  fadeToBlack();
  }
}

int secondsToMinutes(int seconds)
{
  if (seconds >= 600)
  {
    return seconds / 60; // beide Digits zeigen die Minute
  }
  else
  {
    int minutes = seconds / 60;
    int tensSeconds = (seconds % 60) / 10;
    return minutes * 10 + tensSeconds; // nur das zweite Digit zeigt Zehn-Sekunden
  }
}

void startTimer()
{
  int encoderM1 = encoderWert[aktiverModus];
  unsigned long inputTime = encoderWert[aktiverModus];
  unsigned long startTime = millis();
  unsigned long elapsedTime = 0;

  while (inputTime > 0)
  {
    elapsedTime = millis() - startTime;

    if (elapsedTime >= 1000)
    {
      inputTime--;
      startTime = millis();
    }
    encoderWert[aktiverModus] = inputTime;

    if (inputTime > 60)
    {
      showNumber((secondsToMinutes(inputTime)));
    }
    else
    {
      showNumber(inputTime);
    }
    // TODO: Sanduhr Animation
  }
  encoderWert[aktiverModus] = encoderM1;
  // * Eventuell Abschluss-Animation und Beeper
}

void showNumber(int num) // https://robojax.com/learn/arduino/?vid=robojax_74HC595_2_digits (Änderungen vorgenommen)
{
  digit2 = num % 10;
  digit1 = (num / 10) % 10;
  // Send them to 7 segment displays
  if (digit1 == 0)
  {
    uint8_t numberToPrint[] = {B11111111, digits[digit2]};
    myRegister.setAll(numberToPrint);
  }
  else if (aktiverModus == PGM_1 && encoderWert[aktiverModus] >= 60 && encoderWert[aktiverModus] < 600) // Punkt einfügen
  {
    uint8_t editedDigit = digits[digit1];
    uint8_t mask = B10000000;    // Bitmaske zum Isolieren des ersten Bits
    uint8_t newMask = B00000000; // Bitmaske zum Löschen des ersten Bits

    if ((editedDigit & mask) != 0)
    {                         // Überprüfen, ob das erste Bit gesetzt ist
      editedDigit ^= mask;    // Wenn ja, das erste Bit löschen
      editedDigit |= newMask; // Das erste Bit auf 0 setzen
    }
    uint8_t numberToPrint[] = {editedDigit, digits[digit2]};
    myRegister.setAll(numberToPrint);
  }
  else
  {
    uint8_t numberToPrint[] = {digits[digit1], digits[digit2]};
    myRegister.setAll(numberToPrint);
  }
}
