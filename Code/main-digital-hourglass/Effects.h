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

// Button des Drehencoders
#define SW 4

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