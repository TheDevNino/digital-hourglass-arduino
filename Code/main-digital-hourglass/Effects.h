// Library für die WS2812b LEDs
#include <FastLED.h>

// Variablen für die WS2812b LEDs
#define NUM_LEDS 55
#define BRIGHTNESS 20
#define LED_TYPE WS2812
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
#define UPDATES_PER_SECOND 100
CRGBPalette16 currentPalette;
TBlendType currentBlending;
extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

CRGB light_color[7] = {
    CRGB(255, 102, 0),   // 2000K - Ultra warm white
    CRGB(255, 167, 43),  // 3000K - Warm white
    CRGB(255, 228, 214), // 4000K - Neutral white
    CRGB(230, 240, 255), // 5000K - Daylight white
    CRGB(196, 214, 255), // 6000K - Cool daylight
    CRGB(167, 191, 255), // 7000K - Blue daylight
    CRGB(143, 167, 255)  // 8000K - Blue-white daylight
};
int l_counter = 0;

int timerPhase = 1; // Bugfix: Evtl auch 0

// Button des Drehencoders
#define SW 4

void hourglass_LED()
{
    leds[timerPhase] = CRGB::Black;          // "Sandkorn" oben ausblenden
    leds[55 + 1 - timerPhase] = CRGB::White; // "Sandkorn" unten einblenden
    FastLED.show();
    timerPhase++;
}

void changeKelvin()
{
    for (int i = 0; i < NUM_LEDS; i++)
    {
        leds[i] = light_color[l_counter];
    }
    FastLED.show();
    if (l_counter == 6)
    {
        l_counter = 0;
    }
    else
    {
        l_counter++;
    }
}

// TODO @TheDoor47: Statt delay bitte millis, sonst ist die Steuerung im Zeitabschnitt nicht mehr möglich!

void rainbow()
{
    while (digitalRead(SW) == HIGH)
    {
        for (int i = 0; i < NUM_LEDS; i++)
        {
            leds[i] = CHSV(i * 255 / NUM_LEDS, 255, 255);
        }
        FastLED.show();
        delay(50);
    }
}

CRGB blendColors(CRGB color1, CRGB color2, float progress)
{
    CRGB blendedColor;
    blendedColor.red = color1.red + (color2.red - color1.red) * progress;
    blendedColor.green = color1.green + (color2.green - color1.green) * progress;
    blendedColor.blue = color1.blue + (color2.blue - color1.blue) * progress;
    return blendedColor;
}

void fadeToColor(CRGB startColor, CRGB endColor, unsigned long duration, int startLED, int endLED)
{
    unsigned long startTime = millis(); // Startzeit speichern
    while (millis() - startTime <= duration)
    {
        float progress = float(millis() - startTime) / duration;         // Fortschritt des Übergangs berechnen
        CRGB currentColor = blendColors(startColor, endColor, progress); // Farbe basierend auf dem Fortschritt mischen
        for (int i = startLED; i < endLED; i++)
        {
            leds[i] = currentColor; // LED mit der aktuellen Farbe aktualisieren
        }
        FastLED.show(); // LEDs aktualisieren
        delay(10);      // Eine kurze Pause für einen sanften Übergang
    }
    for (int i = startLED; i < endLED; i++)
    {
        leds[i] = endColor; // Abschließend die Endfarbe setzen
    }
    FastLED.show(); // LEDs aktualisieren
}

void colorWipe(CRGB color, int speed)
{
    while (digitalRead(SW) == HIGH)
    {
        for (int i = 0; i < NUM_LEDS; i++)
        {
            leds[i] = color;
            FastLED.show();
            delay(speed);
        }
    }
}

void theaterChase(CRGB color, int speed)
{
    while (digitalRead(SW) == HIGH)
    {
        for (int j = 0; j < 10; j++)
        {
            for (int q = 0; q < 3; q++)
            {
                for (int i = 0; i < NUM_LEDS; i += 3)
                {
                    leds[i + q] = color;
                }
                FastLED.show();
                delay(speed);
                for (int i = 0; i < NUM_LEDS; i += 3)
                {
                    leds[i + q] = CRGB::Black;
                }
            }
        }
    }
}

void twinkle(CRGB color, int speed)
{
    while (digitalRead(SW) == HIGH)
    {
        for (int i = 0; i < NUM_LEDS; i++)
        {
            if (random(100) < 50)
            {
                leds[i] = color;
            }
            else
            {
                leds[i] = CRGB::Black;
            }
        }
        FastLED.show();
        delay(speed);
    }
}

void fadeToBlack()
{
    while (digitalRead(SW) == HIGH)
    {
        for (int i = 0; i < 255; i++)
        {
            for (int j = 0; j < NUM_LEDS; j++)
            {
                leds[j].fadeToBlackBy(1);
            }
            FastLED.show();
            delay(10);
        }
    }
}