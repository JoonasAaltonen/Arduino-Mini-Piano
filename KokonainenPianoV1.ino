/*
 Name:		KokonainenPianoV1.ino
 Created:	11/16/2016 11:52:14 AM
 Author:	Joonas
*/

#include <Key.h>
#include <Keypad.h>    // Kirjastot näppäinmatriisille
#include "notes.h"     // Itse tehty kirjasto äänen korkeuksille 

const byte ROWS = 5;
const byte COLS = 4;
char keys[ROWS][COLS] = {     // Näppäinmatriisin määrittely
	{ 'A',  'B',  'C',  'D' },
	{ 'E',  'F',  'G',  'H' },
	{ 'I',  'J', 'K', 'L' },
	{ 'M', 'N', 'O', 'P' },
	{ 'Q', 'R', '*', '#' }
};
byte ROWPins[ROWS] = { 4, 5, 6, 7, 8 };   // Määritellään I/O-pinnit
byte COLPins[COLS] = { 12, 11, 9, 10 };

int redPin = 13;        // Määritellään I/O-pinnit LEDille ja kaijuttimelle
int speakerPin = A3;

Keypad keypad = Keypad(makeKeymap(keys), ROWPins, COLPins, ROWS, COLS);  // keypad-muuttuja jolla käsitellään näppäimiä
                                                          
int melody[] = {    // Kaikki nuotit väliltä C3 (matala) D#8 (korkea)
	NOTE_C3, NOTE_CS3, NOTE_D3, NOTE_DS3, NOTE_E3, NOTE_F3, NOTE_FS3, NOTE_G3, NOTE_GS3, NOTE_A3, NOTE_AS3, NOTE_B3,
	NOTE_C4, NOTE_CS4, NOTE_D4, NOTE_DS4, NOTE_E4, NOTE_F4, NOTE_FS4, NOTE_G4, NOTE_GS4, NOTE_A4, NOTE_AS4, NOTE_B4,
	NOTE_C5, NOTE_CS5, NOTE_D5, NOTE_DS5, NOTE_E5, NOTE_F5, NOTE_FS5, NOTE_G5, NOTE_GS5, NOTE_A5, NOTE_AS5, NOTE_B5,
	NOTE_C6, NOTE_CS6, NOTE_D6, NOTE_DS6, NOTE_E6, NOTE_F6, NOTE_FS6, NOTE_G6, NOTE_GS6, NOTE_A6, NOTE_AS6, NOTE_B6,
	NOTE_C7, NOTE_CS7, NOTE_D7, NOTE_DS7, NOTE_E7, NOTE_F7, NOTE_FS7, NOTE_G7, NOTE_GS7, NOTE_A7, NOTE_AS7, NOTE_B7,
	NOTE_C8, NOTE_CS8, NOTE_D8, NOTE_DS8, NOTE_E8, NOTE_F8,
};

long startTime;       // Ajastin muuttuja 1, käytetään alku ajan tallennukseen
long elapsedTime;     // nykyinen aika - alku aika = kulunut aika, muuttuja kuluneen ajan tallennukseen
int keypress = 0;     // Näppäinten painalluksia laskeva muuttuja 
char saved[150];      // Painallukset tallentava taulukko
long durations[150];  // Painallusten pituudet tallentava taulukko
int minusArray[150];  // Taulukko johon tallennetaan painalluksen aikana aktiivisena oleva minus arvo
int minus = 65;       // Arvo jonka avulla lasketaan oikean nuotti näppäimen painallukselle
int octave = 1;       // muuttuja joka kertoo mikä oktaavi on aktiivisena
bool saving = false;  // Tallennus on/off muuttuja

void setup()
{
	Serial.begin(9600);       // Debuggausta varten serial konsolin alustaminen
	pinMode(redPin, OUTPUT);  // Määritellään I/O pinnin toiminta

	keypad.addEventListener(keypadEvent);   // Lisätään keypadEventin "vahti"
	keypad.setDebounceTime(1);              // Määritellään näppäimistön "palautumis"-aika (millisekunteja)
	keypad.setHoldTime(1);                  // Määritellään aika, jonka jälkeen näppäimistö lukee painalluksen
}                                         // pohjassa pitämisenä, painamisen sijaan (millisekunteja)
void loop() 
{
	char key = keypad.getKey();       // loop metodi tutkii näppäimistön painalluksia ja kutsuu keypadEventtiä
}

void play(int keyInt)               // Metodi jossa jokaisen nuotin soittaminen tapahtuu, parametrina painetun näppäimen
{                                   // sisältämän merkin ASCII-koodi eli kokonaisluku
	if (keyInt != 35 && keyInt != 42)   // Jos näppäin ei ole * tai #
	{
		startTime = millis();             // Aloitetaan ajanotto
		while (keypad.getState() == HOLD)     // Pysytään silmukassa niin kauan kun näppäimen tila on alas painettu
		{
			char key = keypad.getKey();         // Luetaan näppäintä jotta tiedetään, milloin näppäimen tila vaihtuu uudestaan

			tone(speakerPin, melody[keyInt - minus], 10);   // tone-funktio ottaa parametriksi I/O-pinnin, taajuuden ja ajan
			                                                // Tarkemmat selitykset raportissa
			if (keypad.getState() == RELEASED)     // Jos näppäin vapautetaan
				break;                               // Poistutaan silmukasta
		}
		elapsedTime = millis() - startTime;     // Lasketaan kulunut aika: tämän hetkinen aika - aloitus aika
		durations[keypress] = elapsedTime;      // Lisätään durations taulukkoon pituus, 
	}                                         // keypress muuttuja laskee painalluksia jolloin painalluksen aika on samassa indeksissä kuin painallus
}

void show()       // Metodi jota käytetään tallennuksen toistoon
{
	for (int i = 0; i < sizeof(saved); i++)   // saved[]-taulukon pituinen (150) for-silmukka, 
	{                                         // määrittää muuttujan i jota käytetään alempana
		if (saved[i] == NULL)   // Kun taulukosta tulee vastaan ensimmäinen tyhjä indeksi
		{
			Serial.println("SAAATANA");     // debug-ilmoitus: Taulukko tyhjä
			break;                          // Poistutaan silmukasta
		}
		startTime = millis();             // Aloitetaan ajanotto
		elapsedTime = 0;                  // Nollataan kulunutta aikaa mittaava muuttuja
		while (elapsedTime < durations[i])      // Pysytään silmukassa niin pitkään kuin kulunut aika on vähemmän kuin 
		{                                       // durations-taulukon indeksi i, joka on sama kuin for-silmukan kierrosluku
			elapsedTime = millis() - startTime;   // lasketaan kulunutta aikaa, jotta tiedetään kuinka pitkään pysyä silmukassa
			tone(speakerPin, melody[saved[i] - minusArray[i]]);   // tone-funktio, tarkempi selitys raportissa
		}
		noTone(speakerPin);     // varmistetaan, ettei I/O pinni yritä toistaa ääntä
		delay(66);              // tallennuksen toiston nuottien välissä oleva tauko
	}
}

void keypadEvent(KeypadEvent eKey)    // Näppäimistän lukuun tarvittava event
{                                     // Jota kutsutaan joka näppäimen painalluksella

	int keyInt = eKey;                  // tyyppimuunnos, char eKey --> int keyInt, lukee merkin ja antaa sen ASCII-arvon

	switch (keypad.getState())          // switch jolla luetaan näppäimen tilaa
	{
	case HOLD:                          // Mikäli näppäintä on painettu ja sitä pidetään pohjassa yli 1 millisekunti:
		
		if (keyInt == 42 && saving == false)		// Jos näppäin on * (=REC) ja tallennus ei ole päällä, Tallennuksen aloitus,
		{
			keypress = 0;						                  // nollataan näppäinten painallusten lasku
			saving = true;                            // muuttuja jota käytetään vahtimaan onko tallennus päällä vai ei
			Serial.println("SHAVING STARTED!");       // debug-viesti serial-konsoliin
			digitalWrite(redPin, HIGH);               // Sytytetään punainen LED
		}
		else if (keyInt == 42 && saving == true)	  // jos näppäin on * ja tallnennus on päällä, Tallennuksen lopetus
		{
			saving = false;                           // Tallennus-muuttuja = false
			Serial.println("SHAVING ENDED!");         // debug-viesti
			digitalWrite(redPin, LOW);                // punainen LED pois päältä
		}
		if (keyInt == 35 && saved[0] != NULL && saving == false)
		{				                    // Jos painetaan #-näppäintä, tallennettuna ei ole mitään ja tallennus ei ole päällä
			show();		                // Toistetaan tallennus
      for (int i = 0; i < sizeof(saved); i++)
      {
        saved[i] = (char)0;                    // (char)0 = sama kuin NULL
        minusArray[i] = (char)0;               // Tyhjennetään taulukot toiston jälkeen
        durations[i] = (char)0;      
      }
		}
		else if (keyInt == 35)	          // Muutoin jos painetaan #-näppäintä
		{
			if (octave <= 4)                // jos oktaavi on pienempi tai yhtäsuuri kuin 4
			{
				octave++;		                  // Siirrytään 1 oktaavi ylemmäs
				minus -= 12;	                // Vähennetään alkuperäisestä minus muuttujasta 12
			}
			else {                          // Viidennellä kerralla palataan alkutilaan
				octave = 1;
				minus = 65;
			}
		}
		else      // "Normaali" näppäimen painallus eli mikä tahansa muu kuin * tai #
		{
			Serial.println(keyInt);           // Tulostaa näppäimen sisältämän merkin ASCII-arvon
			Serial.println(eKey);             // Tulostaa näppäimen sisältämän merkin
			play(keyInt);                     // Kutsutaan play-metodia näppäimen ASCII-arvo parametrina
			if (saving == true && (keyInt != 42 && keyInt != 35) && keypress < sizeof(saved))   // Jos tallennus on päällä
			{                                         // ja ei paineta*- tai #-näppäintä ja ei ole tallennettu yli 150 nuottia
				saved[keypress] = keyInt;       // Saved-taulukkoon lisätään näppäimen ASCII-arvo
				minusArray[keypress] = minus;   // minusArray-taulukkoon lisätään sen hetkinen minus arvo (riippuu oktaavista)
				keypress++;                     // keypress kasvaa yhdellä, käytetään taulukoiden indeksinä
			}
		}

	default:      // Pakollinen default case switchiä varten.
		break;
	}
}
