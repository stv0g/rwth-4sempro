/**
 * Analog to Digital converter headers
 *
 * @copyright	2012 Institute Automation of Complex Power Systems (ACS), RWTH Aachen University
 * @license	http://www.gnu.org/licenses/gpl.txt GNU Public License
 * @author	Steffen Vogel <info@steffenvogel.de>
 */

#ifndef _ADC_H_
#define _ADC_H_

// Konfiguration
#define ADC_PORT		PORTA
#define ADC_DDR			DDRA
#define ADC_PIN_STERING_LEFT	0
#define ADC_PIN_STERING_RIGHT	1
#define ADC_PIN_BATT_LOGIC	2
#define ADC_PIN_BATT_DRIVE	3
#define ADMUX_MASK 0x1F

void adc_init();
uint16_t adc_read(uint8_t channel);
uint16_t adc_read_avg(uint8_t channel, uint8_t average);

#endif /* _ADC_H_ */
