#include <Adafruit_NeoPixel.h>

#define LED_PIN   6  // any PWM capable pin
#define NUM_LEDS 58

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup()
{
    strip.begin();
}

void loop()
{
    alternatePixels(20, 150, strip.Color(0, 50, 0), strip.Color(0, 0, 0));
    alternatePixels(75, 75, strip.Color(50, 40, 0), strip.Color(50, 0, 0));
    showMeteorPixels(10, 10, strip.Color(0, 50, 50));
    showKnightRider(10, 5, strip.Color(255, 0, 0));
    showRandomPixels(40, 80, 10, false);
    showRandomPixels(40, 80, 100, true);
}

void turnOffPixels()
{
    for (byte i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
    strip.show();
}

void alternatePixels(int iterations, int delayMilliseconds, uint32_t color1, uint32_t color2)
{
    turnOffPixels();
    for (int iter = 0; iter < iterations; iter++) {
        for (byte i = 0; i < NUM_LEDS; i++) {
            if ((iter + i) % 2 == 0) {
                strip.setPixelColor(i, color1);
            } else {
                strip.setPixelColor(i, color2);
            }
        }
        strip.show();
        delay(delayMilliseconds);
    }
}

void showMeteorPixels(int iterations, int delayMilliseconds, uint32_t color)
{
    const byte STEP = 1;
    uint8_t r, g, b;
    uint32_t col;
    unsigned long sum = 0;

    turnOffPixels();

    for (int iter = 0; iter < iterations; iter++) {
        byte headIndex = 0;
        do {
            sum = 0;

            for (byte i = 0; i < NUM_LEDS; i++) {
                col = strip.getPixelColor(i);
                sum += col;

                r = col >> 16;
                g = col >> 8;
                b = col;

                if (r > 0) {
                    r -= STEP;
                    r = max(0, r);
                }
                if (g > 0) {
                    g -= STEP;
                    g = max(0, g);
                }
                if (b > 0) {
                    b -= STEP;
                    b = max(0, b);
                }
                strip.setPixelColor(i, strip.Color(r, g, b));
            }

            if (headIndex < NUM_LEDS) {
                strip.setPixelColor(headIndex++, color);
                sum += color;
            }

            strip.show();
            delay(delayMilliseconds);
        } while (sum > 0);
    }
}

void showKnightRider(int iterations, int delayMilliseconds, uint32_t color)
{
    const byte STEP = 5;
    byte dir = 0, headIndex = 0;
    uint8_t r, g, b;
    uint32_t col;

    turnOffPixels();

    for (int iter = 0; iter < iterations; iter++) {
        for (int i = 0; i < NUM_LEDS; i++) {
            for (int k = 0; k < NUM_LEDS; k++) {
                col = strip.getPixelColor(k);

                r = col >> 16;
                g = col >> 8;
                b = col;

                if (r > 0) {
                    r -= STEP;
                    r = max(0, r);
                }
                if (g > 0) {
                    g -= STEP;
                    g = max(0, g);
                }
                if (b > 0) {
                    b -= STEP;
                    b = max(0, b);
                }

                strip.setPixelColor(k, strip.Color(r, g, b));
            }

            strip.setPixelColor(headIndex, color);
            if (dir == 0) {
                headIndex++;
            } else {
                headIndex--;
            }

            strip.show();
            delay(delayMilliseconds);
        }

        dir++;
        dir%=2;
    }
}

void showRandomPixels(int iterations, int delayMilliseconds, byte ledBrightness, bool randomizeColors)
{
    uint32_t c = strip.Color(ledBrightness, 0, 0);
    for (int iter = 0; iter < iterations; iter++) {
        turnOffPixels();
        for (byte i = 0; i < NUM_LEDS; i++) {
            if (random(0, 2) == 0) {
                if (randomizeColors) {
                    c = strip.Color(random(ledBrightness), random(ledBrightness), random(ledBrightness));
                }
                strip.setPixelColor(i, c);
            }
        }
        strip.show();
        delay(delayMilliseconds);
    }
}
