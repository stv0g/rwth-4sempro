/**
 * Rotary encoder headers
 *
 * @copyright	2012 Institute Automation of Complex Power Systems (ACS), RWTH Aachen University
 * @license	http://www.gnu.org/licenses/gpl.txt GNU Public License
 * @author	Steffen Vogel <info@steffenvogel.de>
 */

#ifndef _ROTARY_H_
#define _ROTARY_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#define DGB_PORT	PORTB
#define DGB_DDR		DDRB
#define DGB_PIN_A	PB0
#define DGB_PIN_B	PB1
#define DGB_PIN_SW	PB2
#define DGB_PIN		PINB

// Definitionen für zusätzliche Schalter
#define SW_PORT		PORTB
#define SW_DDR		DDRB
#define SW_PIN		PINB
#define SW_PIN_BLAU	PB3
#define SW_PIN_GRUEN	PB4

enum taster {
	DGB_CCW = 1,
	DGB_CW,
	DGB_SW,
	SW_GRUEN,
	SW_BLAU
};

void dgb_init();
enum taster dgb_read();

#endif /* _ROTARY_H_ */
