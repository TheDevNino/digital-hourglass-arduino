/*

   TPD-B-Projekt "Digitale Sanduhr":

   Projektleitung:
   Theodor Diehl
   Gian Nino Cataffo

   2023, HAW Hamburg

   Link zum Projekt:
   https://github.com/TheDevNino/digital-hourglass-arduino

*/

// Headerdatei für die Effekte
#include "Effects.h";

// Library für die 7-Segmentanzeige
#include <ShiftRegister74HC595.h>

// Library für den IR Empfänger: V 2-0-1
#include <IRremote.h>

// Library für die WS2812b LEDs
#include <FastLED.h>

// Library für die serielle Schnittstelle (Hier: Gyroskop)
#include <Wire.h>

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
int luminanceM1;

// WS2812b - hier evtl löschen
#define LED_PIN 8
#define NUM_LEDS 55
#define BRIGHTNESS 20
#define LED_TYPE WS2812
#define COLOR_ORDER GRB
#define UPDATES_PER_SECOND 100

// IR Empfänger
#define RECV_PIN 9
IRrecv irrecv(RECV_PIN);
decode_results results;
int IR_Value[] = {
    16738455, // 0
    -14281,   // 1
    18615,    // 2
    2295,     // 3
    -6121,    // 4
    26775,    // 5
    10455,    // 6
    -10201,   // 7
    22695,    // 8
    10455,    // 9
    -26011,   // ++
    4845,     // ++
    8415,     // ++
    -12241,   // --
    -30091,   // --
    24735,    // --
    -21931,   // OK
    15045,    // Home
    -26521    // Home
};

// Gyroskop
const int MPU_addr = 0x68;
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
bool horizontalPosition, horizontalPositionM1;
int minVal = 265;
int maxVal = 402;
double x;
double y;
double z;

// evtl Beeper

void setup()
{
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

  // Initialisierung der seriellen Schnittstelle (Gyroskop)
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  // Initialisierung des Infrarotempfängers
  irrecv.enableIRIn(); // enable the receiver
  irrecv.blink13(true);

  // Initialisierung der LEDs
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  delay(400);
}

void loop()
{
  intervallAnpassung();
  Gyro_Auswerten();
  IR_Auswerten();
  encoderAuswerten();
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

void Gyro_Auswerten()
{
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true);
  AcX = Wire.read() << 8 | Wire.read();
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();
  int xAng = map(AcX, minVal, maxVal, -180, 180);
  int yAng = map(AcY, minVal, maxVal, -180, 180);
  int zAng = map(AcZ, minVal, maxVal, -180, 180);

  z = RAD_TO_DEG * (atan2(-yAng, -xAng) + PI);

  horizontalPositionM1 = horizontalPosition;
  if (z >= 240 && z <= 300)
    horizontalPosition = false;
  if (z >= 60 && z <= 120)
    horizontalPosition = true;
  if (horizontalPosition != horizontalPositionM1 && aktiverModus == PGM_1)
    signalAuswerten(1);

  // TODO: Anpassung der Segmentanzeige
}

void IR_Auswerten()
{
  if (irrecv.decode(&results))
  {
    int IR_Input = results.value;
    Serial.println(IR_Input);

    for (int i = 0; i <= 9; i++)
    {
      if (IR_Input == IR_Value[i])
      {
        if (aktiverModus == PGM_1)
        {
          encoderWert[aktiverModus] = i * 60;
        }
        else
        {
          encoderWert[aktiverModus] = i;
        }
      }
    }

    if (IR_Input == IR_Value[10] || IR_Input == IR_Value[13] || IR_Input == IR_Value[12])
      encoderWert[aktiverModus] += encoderIntervall[aktiverModus];
    if (IR_Input == IR_Value[13] || IR_Input == IR_Value[14] || IR_Input == IR_Value[15])
      encoderWert[aktiverModus] -= encoderIntervall[aktiverModus];
    if (IR_Input == IR_Value[16])
      signalAuswerten(1);
    if (IR_Input == IR_Value[17] || IR_Input == IR_Value[18])
      signalAuswerten(1000);

    encoderWert[aktiverModus] = constrain(encoderWert[aktiverModus], encoderMin[aktiverModus], encoderMax[aktiverModus]);
    if (aktiverModus == PGM_1 && encoderWert[aktiverModus] >= 60)
    {
      showNumber(secondsToMinutes(encoderWert[aktiverModus]));
    }
    else
    {
      showNumber(encoderWert[aktiverModus]);
    }

    irrecv.resume();

    // Debugging
    InfoToSerial();
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
      encoderWert[aktiverModus] -= encoderIntervall[aktiverModus];
    }
    else
    {
      encoderWert[aktiverModus] += encoderIntervall[aktiverModus];
    }

    // Clamp the parameter value to the allowed range
    encoderWert[aktiverModus] = constrain(encoderWert[aktiverModus], encoderMin[aktiverModus], encoderMax[aktiverModus]);

    // Größere Zahlen mit Dezimalstelle anzeigen
    if (aktiverModus == PGM_1 && encoderWert[aktiverModus] >= 60)
    {
      showNumber(secondsToMinutes(encoderWert[aktiverModus]));
    }
    else
    {
      showNumber(encoderWert[aktiverModus]);
    }

    // Debugging
    InfoToSerial();
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
      if (buttonDuration != 402)
      {
        Serial.println(buttonDuration);
        signalAuswerten(buttonDuration);
      }
    }
  }

  lastButtonState = buttonState; // Speichern des vorherigen Status des Buttons

  // Debounce the reading
  delay(1);
}

void signalAuswerten(long duration)
{
  if (duration > 500)
  {
    aktiverModus = 0;
    showNumber(encoderWert[aktiverModus]);
  }
  else if (duration <= 500)
  {
    switch (aktiverModus)
    {
    case PGM_Auswahl:
      aktiverModus = encoderWert[aktiverModus];
      showNumber(encoderWert[aktiverModus]);
      break;
    case PGM_1:
      // start timer
      startTimer(encoderWert[aktiverModus]);
      Serial.println("Timer Start");
      break;
    case PGM_2:
      // start effekt
      effectProgram();
      break;
    case PGM_3:
      // change kelvin-wert
      lightProgram();
      break;
    }
  }
}

void InfoToSerial()
{
  // Print the updated parameter value
  Serial.print("PGM\tVal\tInt\n");
  Serial.print(aktiverModus);
  Serial.print("\t");
  Serial.print(encoderWert[aktiverModus]);
  Serial.print("\t");
  Serial.print(encoderIntervall[aktiverModus]);
  Serial.print("\n---------------\n");
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

void startTimer(long dauer)
{
  unsigned long startTime = millis();
  long elapsedTime = 0;
  float animationInterval = (float)dauer * 1000 / 29.0;
  unsigned long lastAnimationTime = millis();
  int animationCounter = 0;
  Serial.print("Animationsintervall in ms: ");
  Serial.println(animationInterval);
  int timerPhase = -1;
  bool abbruch = false;

  for (int i = 0; i < NUM_LEDS / 2; i++)
  {
    if (horizontalPosition)
    {
      leds[i] = CRGB::White;
    }
    else
    {
      leds[i] = CRGB::Black;
    }
    FastLED.show();
  }
  for (int i = NUM_LEDS / 2; i < NUM_LEDS; i++)
  {
    if (horizontalPosition)
    {
      leds[i] = CRGB::Black;
    }
    else
    {
      leds[i] = CRGB::White;
    }
    FastLED.show();
  }

  while (elapsedTime < dauer)
  {
    if (millis() - startTime >= 1000)
    {
      startTime += 1000;
      elapsedTime++;
      Serial.print(elapsedTime);
      if ((dauer - elapsedTime) > 60)
      {
        showNumber((secondsToMinutes(dauer - elapsedTime)));
      }
      else
      {
        showNumber(dauer - elapsedTime);
      }
    }
    if (millis() - lastAnimationTime >= animationInterval)
    {
      lastAnimationTime = millis();
      if (horizontalPosition)
      {
        leds[timerPhase] = CRGB::Black; // "Sandkorn" unten ausblenden
        FastLED.show();
        leds[54 - timerPhase] = CRGB::White; // "Sandkorn" oben einblenden
        FastLED.show();
      }
      else
      {
        leds[54 - timerPhase] = CRGB::Black; // "Sandkorn" oben ausblenden
        FastLED.show();
        leds[timerPhase] = CRGB::White; // "Sandkorn" unten einblenden
        FastLED.show();
      }

      timerPhase++;
      animationCounter++;
    }

    if (digitalRead(SW) == LOW) // Timer Abbruch
    {
      elapsedTime = dauer;
      abbruch = true;
      delay(500);
    }
  }
  Serial.println("Timer abgelaufen.");
  showNumber(encoderWert[aktiverModus]);
  if (!abbruch)
  {
    if (!horizontalPosition)
    {
      fadeToColor(CRGB::White, CRGB::Red, 2000, 0, 25);
    }
    else
    {
      fadeToColor(CRGB::White, CRGB::Red, 2000, 25, 55);
    }
    // * Eventuell Beeper
  }
  abbruch = false;
}

void showNumber(int num)
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

void effectProgram()
{
  if (encoderWert[aktiverModus] == 1)
  {
    rainbow(); // Rainbow effect
  }
  if (encoderWert[aktiverModus] == 2)
  {
    colorWipe(CRGB::Red, 50); // Color wipe effect
  }
  if (encoderWert[aktiverModus] == 3)
  {
    theaterChase(CRGB::Green, 50); // Theater chase effect
  }
  if (encoderWert[aktiverModus] == 4)
  {
    twinkle(CRGB::Blue, 50); // Twinkle effect
  }
  if (encoderWert[aktiverModus] == 5)
  {
    fadeToBlack(); // Fade all LEDs to black
  }
}

void lightProgram()
{
  int luminance = map(encoderWert[3], encoderMin[3], encoderMax[3], 0, 50); // Encoder-Wert auf LED-Helligkeitswert anpassen

  if (luminanceM1 == luminance)
  {
    changeKelvin();
  }
  else
  {
    FastLED.setBrightness(luminance);
    luminanceM1 = luminance;
  }
}