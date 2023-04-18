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

// IR Empfänger
#define IR 5

// WS2812b

// Gyroskop

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
}

void loop()
{
  if (aktiverModus == PGM_1)
  {
    intervallAnpassung();
  }
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

  // Debounce the reading
  delay(1);
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
      break;
    case PGM_3:
      // change kelvin-wert
      break;
    }
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