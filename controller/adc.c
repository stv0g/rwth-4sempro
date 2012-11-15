/**
 * Analog to Digital converter routines
 *
 * @copyright	2012 Institute Automation of Complex Power Systems (ACS), RWTH Aachen University
 * @license	http://www.gnu.org/licenses/gpl.txt GNU Public License
 * @author	Steffen Vogel <info@steffenvogel.de>
 */

#include <avr/io.h>

#include "adc.h"

/**
 * Initialisierung Analog to digital Converter (ADC)
 */
void adc_init() {
	uint16_t result;

	ADC_DDR = 0x00;					// Pins als Eingänge
	ADC_PORT = 0x00;				// Pullups deaktivieren

	ADMUX = (1<<REFS1) | (1<<REFS0);		// interne 2.56V als Referenz benutzen
	ADCSRA = (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);	// Frequenzvorteiler
	ADCSRA |= (1<<ADEN);				// ADC aktivieren
	ADCSRA |= (1<<ADIE);				// ADC Interrupts erlauben

  	ADCSRA |= (1<<ADSC);				// starte eine ADC-Wandlung 

  	while (ADCSRA & (1<<ADSC) ) { }			// auf Abschluss der Konvertierung warten
  	result = ADC;
}

/**
 * Auslesen des Kanals
 */
uint16_t adc_read(uint8_t channel) {
	// Kanal waehlen, ohne andere Bits zu beeinflußen
	ADMUX = (ADMUX & ~(0x1F)) | (channel & 0x1F);
	ADCSRA |= (1<<ADSC);				// eine Wandlung "single conversion"

	while (ADCSRA & (1<<ADSC) ) { }			// auf Abschluss der Konvertierung warten
	return ADC;					// ADC auslesen und zurückgeben
}

/**
 * Auslesen des Kanals mit Mittelwertrückgabe
 */
uint16_t adc_read_avg( uint8_t channel, uint8_t average) {
	uint32_t result = 0;

	for (uint8_t i = 0; i < average; ++i ) {
		result += adc_read(channel);
	}

	return (uint16_t) (result / average);
}

