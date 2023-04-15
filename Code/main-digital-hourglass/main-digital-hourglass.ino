/* 

   TPD-B-Projekt "Digitale Sanduhr":

   Projektleitung:
   Theodor Diehl
   Gian Nino Cataffo

   2023, HAW Hamburg

   Link zum Projekt:
   https://github.com/TheDevNino/digital-hourglass-arduino

*/

// Grundlage für die gesamte Programmstruktur
enum Modus {
  PGM_Auswahl,
  PGM_1,
  PGM_2,
  PGM_3
};
Modus (aktiverModus) = PGM_Auswahl;

// Speicherung von Werten für Programme
int encoderWert[4];
encoderWert[0] = 1;
encoderWert[1] = 10;
encoderWert[2] = 1;
encoderWert[3] = 50;

// Standard-Intervalle für Programme
int encoderIntervall[4];
encoderIntervall[0] = 1;  
encoderIntervall[1] = 10;
encoderIntervall[2] = 1; 
encoderIntervall[3] = 1; 


// - HARDWARE -

// Drehencoder
#define CLK   2
#define DT    3
#define SW    4

// IR Empfänger
#define IR    5

// ...



void setup() {
	// encoder pins
	pinMode(CLK,INPUT);
	pinMode(DT,INPUT);
	pinMode(SW, INPUT_PULLUP);

	// Setup Serial Monitor
	Serial.begin(9600);

	// Read the initial state of CLK
	lastStateCLK = digitalRead(CLK);
}

void loop() {

  switch(aktiverModus){
    case PGM_Auswahl:
      encoderAuswerten(0);
      break;
    case PGM_1:
      encoderAuswerten(1);
      break;
    case PGM_2:
      encoderAuswerten(2);
      break;
    case PGM_3:
      encoderAuswerten(3);
      break;
  }
}

void PGM_Auswahl(){}
void PGM_1(){}
void PGM_2(){}
void PGM_3(){}

void encoderAuswerten(int PGM){

	currentStateCLK = digitalRead(CLK);
  if (currentStateCLK != lastStateCLK  && currentStateCLK == 1){
    if (digitalRead(DT) != currentStateCLK) {
        encoderWert[PGM] = encoderWert[PGM] + encoderIntervall[PGM];
      } else {
        encoderWert[PGM] = encoderWert[PGM] - encoderIntervall[PGM];
      }
	}
	lastStateCLK = currentStateCLK; // Remember last CLK state

	// Read the button state
	int btnState = digitalRead(SW);
	if (btnState == LOW) { 	//If LOW, button pressed
		if (millis() - lastButtonPress > 50) {
			Serial.println("Button pressed!");
		}
		lastButtonPress = millis();
	}
}
