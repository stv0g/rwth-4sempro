/**
 * Rotary encoder routines
 *
 * @copyright	2012 Institute Automation of Complex Power Systems (ACS), RWTH Aachen University
 * @license	http://www.gnu.org/licenses/gpl.txt GNU Public License
 * @author	Steffen Vogel <info@steffenvogel.de>
 */

#include "rotary.h"

/**
 * Drehgeber initialisieren
 */
void dgb_init() {
	// DDR auf input setzen
	SW_DDR &= ~(1<<SW_PIN_GRUEN);
	SW_DDR &= ~(1<<SW_PIN_BLAU);
	DGB_DDR &= ~(1<<DGB_PIN_A);
	DGB_DDR &= ~(1<<DGB_PIN_B);
	DGB_DDR &= ~(1<<DGB_PIN_SW);

	// PullUps einschalten
	SW_PORT |= (1<<SW_PIN_GRUEN);
	SW_PORT |= (1<<SW_PIN_BLAU);
	DGB_PORT |= (1<<DGB_PIN_A);
	DGB_PORT |= (1<<DGB_PIN_B);
	DGB_PORT |= (1<<DGB_PIN_SW);
}

/**
 * Drehgeber auslesen
 * 
 * Diese Funktion sollte periodisch aufgerufen werden (Polling)
 */
uint8_t dgb_read() {
	// Debouncing
	volatile static uint8_t lastA;
	volatile static uint8_t lastSW;
	volatile static uint8_t lastSW_Gruen;
	volatile static uint8_t lastSW_Blau;

	volatile static bool trigger;
	volatile static bool ready;
	volatile static bool ignore;

	enum taster eingabe = 0;

	// Drehgeber Trigger finden
	if ((!(DGB_PIN & (1<<DGB_PIN_A))) && !trigger) {
		if(lastA < 3) {
			lastA++;
		}
		else {
			trigger = true;
			lastA = 0;
		}
	}

	// Richtung auswerten
	if (trigger && !ready) {
		if (!(DGB_PIN & (1<<DGB_PIN_B))) {
			eingabe = DGB_CCW;
			ready = true;
		}
		else {
			eingabe = DGB_CW;
			ready = true;
		}
	}

	if(ready && (DGB_PIN & (1<<DGB_PIN_A))) {
		ready = false;
		trigger = false;
	}

	// Schalter des Drehgebers auswerten
	if (!(DGB_PIN & (1<<DGB_PIN_SW))) {
		if (lastSW < 255) lastSW++;
	}
	else if (lastSW == 255) {
		lastSW = 0;
		if (ignore) {
			ignore = false;
		}
		else {
			eingabe = DGB_SW;
		}
	}

	// zusätzliche Schalter auswerten
	if (!(SW_PIN & (1<<SW_PIN_GRUEN))) {
		if(lastSW_Gruen < 255) {
			lastSW_Gruen++;
		}
	}
	else if (lastSW_Gruen == 255) {
		lastSW_Gruen = 0;

		if (ignore) {
			ignore = false;
		}
		else {
			eingabe = SW_GRUEN;
		}
	}


	if (!(SW_PIN & (1<<SW_PIN_BLAU))) {
		if (lastSW_Blau < 255) {
			lastSW_Blau++;
		}
	}
	else if (lastSW_Blau == 255) {
		lastSW_Blau = 0;

		if (ignore) {
			ignore = false;
		}
		else {
			eingabe = SW_BLAU;
		}
	}

	return eingabe;
}
