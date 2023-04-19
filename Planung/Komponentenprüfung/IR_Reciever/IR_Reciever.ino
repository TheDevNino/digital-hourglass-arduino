#include <IRremote.h> // V 2.0.1

IRrecv irrecv(2); // IR-Empfänger an Pin 2
decode_results results;

void setup() {
  Serial.begin(9600);
  irrecv.enableIRIn(); // IR-Empfänger aktivieren
}

void loop() {
  if (irrecv.decode(&results)) { // Wenn ein IR-Signal empfangen wurde
    Serial.println(results.value, HEX); // hexadezimalen Wert des Codes auf dem LCD-Display anzeigen
    irrecv.resume(); // Bereit für den Empfang von weiteren IR-Signalen
  }
}