/**
 * Main loop
 *
 * @copyright	2012 Institute Automation of Complex Power Systems (ACS), RWTH Aachen University
 * @license	http://www.gnu.org/licenses/gpl.txt GNU Public License
 * @author	Steffen Vogel <info@steffenvogel.de>
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>

#include "lcd.h"
#include "rotary.h"
#include "adc.h"
#include "pid.h"
#include "uart.h"

#define DISPLAY_MODI 11
#define BAUDRATE 57600

#define MAGIC_BYTE 0xca
#define MAGIC_LEN 16

const char *mode_str[] = {
	"Stop  ",
	"Auto  ",
	"Manual",
	"UART  "
};

const char *display_str[] = {
	NULL,
	"PWM Motor",
	"PWM Servo",
	"PID Drive: P",
	"PID Drive: I",
	"PID Stering: P",
	"PID Stering: I",
	"ADC Inductor: L",
	"ADC Inductor: R",
	"ADC Batt: Logic",
	"ADC Batt: Drive"
};

enum channel {
	STERING_LEFT,
	STERING_RIGHT,
	BATT_LOGIC,
	BATT_DRIVE
};

enum state {
	HALT,
	AUTO,
	MANUAL
};

enum menu {
	OVERVIEW,
	PWM_DRIVE,
	PWM_STERING,
	PID_DRIVE_P,
	PID_DRIVE_I,
	PID_STERING_P,
	PID_STERING_I,
	ADC_STERING_LEFT,
	ADC_STERING_RIGHT,
	ADC_BATT_LOGIC,
	ADC_BATT_DRIVE,
};

enum cmd {
	SET_STATUS,	/* Mode, Display */
	SET_PWM,	/* Drive & Stering */
	SET_PID,	/* Controller state */
	GET_PID		/* Controller state */
};

// EEPROM
int16_t pid_drive_p_ee EEMEM = 0;
int16_t pid_drive_i_ee EEMEM = 0;
int16_t pid_stering_p_ee EEMEM = 39;
int16_t pid_stering_i_ee EEMEM = 0;

int16_t pwm_drive_ee EEMEM = 0;
int16_t pwm_stering_ee EEMEM = 0;

// Globale Daten
int16_t pwm_stering;
int16_t pwm_drive;

int8_t out_stering;
uint8_t out_drive;

int16_t adc_stering_left;
int16_t adc_stering_right;
int16_t adc_batt_logic;
int16_t adc_batt_drive;

uint8_t speed;
uint16_t speed_cnt;
bool speed_ovf = true;

struct pid pid_drive;
struct pid pid_stering;

enum state mode;
enum menu display;

bool edit = false;
bool redraw = true;

/**
 * Parameter editieren
 */
void parameter_edit(int16_t *parameter, int16_t *parameter_ee, enum taster input) {
	switch (input) {
		case DGB_SW: // Editier-Modus togglen
			edit = !edit;
			break;

		case DGB_CW: // Wert ändern
			if (edit) *parameter = *parameter + 1;
			break;

		case DGB_CCW: // Wert ändern
			if (edit) *parameter = *parameter - 1;
			break;

		case SW_GRUEN: // Laden aus EEPROM
			*parameter = eeprom_read_word((uint16_t *) parameter_ee);
			break;

		case SW_BLAU: // Speichern im EEPROM
			eeprom_write_word((uint16_t *) parameter_ee, *parameter);
			break;
	}
}

/**
 * Startsequenz
 */
void greeter() {
	// UART
	uart_puts("Donaudampfschiff\r\n");

	// LCD
	lcd_clear();
	lcd_string("Donaudampfschiff");
	lcd_setcursor(0, 1);
	lcd_string("   4.Sem.Proj.  ");
	_delay_ms(1500);
	lcd_clear();
}

/**
 * Initialize Timers and IO Ports
 */
void init() {
	DDRB = 0xff;
	DDRC = 0xff;
	DDRD = 0b11111011;

	// Geschwindigkeitssensor
	// Falling Edge on INT0 triggers Interrupt
	MCUCR = (1<<ISC01) | (0<<ISC00);
	GICR = (1<<INT0);

	// Timer Counter 0 init - 8 bit
	// f = 7.812 kHz
	// Verwendung fuer Auswertung der Taster und analog Signal Erzeugung
	TCCR0 = (0<<FOC0) | (1<<WGM00) | (0<<COM01) | (0<<COM00) | (1<<WGM01) | (0<<CS02) | (1<<CS01) | (0<<CS00);
	TIMSK |= (1<<TOIE0); // Timer 0 OVF Interrupt einschalten

	// Timer Counter 1 - Servo - 16 bit Counter - UINT16_MAX = 65535
	// Frequenz 50 Hz
	// Timer 1: f_PWM=f_Clk/(Prescaler1*(1+ICR1))
	TCCR1A = (1<<COM1A1) | (0<<COM1A0) | (1<<COM1B1) | (0<<COM1B0) | (1<<WGM11) | (0<<WGM10);
	TCCR1B = (1<<WGM13) | (1<<WGM12) | (0<<CS12) | (1<<CS11) | (0<<CS10); // Prescaler1 = 8
	TIMSK |= (1<<TOIE1);

	ICR1 = 40000;	// Für welchen Wert im Register ICR1 ergibt sich eine Grundfrequenz von 50 Hz
	OCR1A = 0;	// Register zum Einstellen des Servo Tastgrades

	// Timer Counter 2 init - 8 bit
	// Verwendung fuer Motor-PWM
	// Timer 2: f = f_Clk / (256 * Prescaler2)
	// f2 = 976 Hz
	TCCR2 = (0<<FOC2) | (1<<WGM20) | (1<<COM21) | (0<<COM20) | (1<<WGM21) | (1<<CS22) | (0<<CS21) | (0<<CS20); // Prescaler2 = 64
	OCR2 = 0;		// Register zum Einstellen des Motor Tastgrades
	TIMSK |= (1<<TOIE2);	// Timer 2 OVF Interrupt für Eingabe Polling einschalten
}


/**
 * Interupt Subroutine für Eingabe Polling
 */
ISR(TIMER0_OVF_vect) {
	enum taster input = dgb_read(); /* Drehgeber wird periodisch ausgelesen */

	if (edit == false && input) { /* Menu wechseln */
		if (input == DGB_CW && display < DISPLAY_MODI-1) {
			display++;
		}
		else if (input == DGB_CCW && display != OVERVIEW) {
			display--;
		}

		redraw = true;
	}

	switch (display) {
		case OVERVIEW:
			switch (input) {
				case SW_BLAU:
					mode = HALT;
					break;

				case SW_GRUEN:
					switch (mode) {
						case HALT:
							mode = MANUAL;
							break;

						case MANUAL:
							mode = AUTO;
							break;

						case AUTO:
							mode = MANUAL;
							break;

						default:
							mode = HALT;
					}
					break;

				case DGB_CW:
				case DGB_CCW:
				case DGB_SW:
					;
			}
			break;

		case PWM_DRIVE:
			parameter_edit(&pwm_drive, &pwm_drive_ee, input);
			break;

		case PWM_STERING:
			parameter_edit(&pwm_stering, &pwm_stering_ee, input);
			break;

		case PID_STERING_P:
			parameter_edit(&pid_stering.pFactor, &pid_stering_p_ee, input);
			break;

		case PID_STERING_I:
			parameter_edit(&pid_stering.iFactor, &pid_stering_i_ee, input);
			break;

		case PID_DRIVE_P:
			parameter_edit(&pid_drive.pFactor, &pid_drive_p_ee, input);
			break;

		case PID_DRIVE_I:
			parameter_edit(&pid_drive.iFactor, &pid_drive_i_ee, input);
			break;

		case ADC_STERING_LEFT:
		case ADC_STERING_RIGHT:
		case ADC_BATT_LOGIC:
		case ADC_BATT_DRIVE:
			/* ADC Werte können nicht geändert werden */;
	}
}

/**
 * Interupt Subroutine für UART Kommunikation
 */
ISR(TIMER1_OVF_vect) {
	static uint8_t cnt, cnt2, magic, dat = 1;
	char buffer[32];

	/* Send Data */
	/*if (cnt++ == 25) {

		switch (cnt2++ & 0x01) {
			case 0:
				sprintf(buffer,
					"spd=%i, adc_r=%i, adc_l=%i, ",
					speed,
					adc_stering_right,
					adc_stering_left
				);
				break;

			case 1:
				sprintf(buffer,
					"stering=%i, drive=%i\r\n",
					out_stering,
					out_drive
				);
				break;
		}

		uart_puts(buffer);

		cnt = 0;
	}*/

	/*uint16_t byte = uart_getc();
	if (byte != UART_NO_DATA) {
		if (byte & 0x01) {
			pwm_stering = *((int8_t *) &byte);
		}
		else {
			pwm_drive = (uint8_t) byte;
		}
	}*/
}

/**
 * Interupt Subroutine für Regelung
 */
ISR(TIMER2_OVF_vect) {
	speed_cnt++;
	speed_ovf |= (speed_cnt == INT16_MAX);

	/**
	 * Berechnung der neuen Sollwerte
	 */
	int16_t diff = adc_stering_right - adc_stering_left;
	uint16_t sum = adc_stering_right + adc_stering_left;

	/**

	 * Schreiben der Compare-Register
	 * Motor Tastgrad
	 */
	switch (mode) {
		case MANUAL:
			out_drive = pwm_drive;
			out_stering = pwm_stering;
			break;

		case AUTO:
			if (sum < 30) { /* Von Strecke abgekommen */
				mode = HALT;
				display = OVERVIEW;
				edit = false;
			}
			else {
				out_stering = pid_controller(0 , diff , &pid_stering);

				// output_drive = pid_controller(pwm_drive, set_drive, &pid_drive); // funktioniert noch nicht
				// output_drive = pid_controller(pwm_drive, speed, &pid_drive); // funktioniert noch nicht
				out_drive = (pwm_drive>>1) + (( pwm_drive * (128 - abs(out_stering))) >> 8);
			}
			break;

		case HALT:
		default:
			out_drive = 0;
			out_stering = 0;
	}

	/**
	 * Servo Tastgrad
	 * Links 2.1ms	=> 4200
	 * Mitte 1.5ms	=> 3000
	 * Rechts 0.9ms	=> 1800
	 */
	OCR1A = 3000 + (out_stering * 9);
	OCR2 = out_drive;
}


/**
 * Interupt Subroutine für ADC
 */
ISR(ADC_vect) {
	enum channel ch = ADMUX & ADMUX_MASK;

	switch (ch) {
		case STERING_LEFT:
			adc_stering_left = ADC;
			break;

		case STERING_RIGHT:
			adc_stering_right = ADC;
			break;

		case BATT_LOGIC:

			adc_batt_logic = ADC;
			break;

		case BATT_DRIVE:
			adc_batt_drive = ADC;
			break;
	}

	ADMUX = (ADMUX & ~ADMUX_MASK);
	ADMUX |= (ch >= 1) ? 0 : ++ch;

 	ADCSRA |= (1<<ADSC); // starte nächste Messung
}

/**
 * Interrupt Subroutine für Geschwindigkeitssensor
 */
ISR(INT0_vect) {
	if (speed_ovf) {
		speed = 0;
		speed_ovf = false;
	}
	else {
		speed = 2e4 / speed_cnt;
	}
	speed_cnt = 0;
}

int main() {

	// Gespeicherte Werte einlesen
	pwm_drive = eeprom_read_word((uint16_t *) &pwm_drive_ee);
	pwm_stering = eeprom_read_word((uint16_t *) &pwm_stering_ee);

	// Initialisierung
	init();
	adc_init();
	dgb_init();
	lcd_init();
	uart_init(UART_BAUD_SELECT(BAUDRATE, F_CPU));

	// PID mit EEPROM-Werten initialisieren
	pid_init(
		eeprom_read_word((uint16_t *) &pid_drive_p_ee),
		eeprom_read_word((uint16_t *) &pid_drive_i_ee),
		0, &pid_drive);
	pid_init(
		eeprom_read_word((uint16_t *) &pid_stering_p_ee),
		eeprom_read_word((uint16_t *) &pid_stering_i_ee),
		0, &pid_stering);

	// Interrupts aktivieren
	sei();

	// Startsqeuenz
	greeter();

	// Watchdog Timer aktivieren
	wdt_enable(WDTO_1S);

	// Menü
	while (1) {
		wdt_reset();

		lcd_home(); /* set cursor to top-leftmost postion */

		if (redraw) {
			lcd_clear();

			if (display_str[display]) {
				lcd_string(display_str[display]);
			}

			redraw = false;
		}

		switch (display) {
			case OVERVIEW:
				lcd_string(mode_str[mode]);
				lcd_string("  S:");
				lcd_int(out_stering, 6);

				lcd_setcursor(0, 1);
				lcd_string("V:");
				lcd_int(speed, 5);

				lcd_string(" D:");
				lcd_int(out_drive, 6);
				break;

			case PWM_DRIVE:
				lcd_setcursor(0, 1);
				lcd_int(pwm_drive, 16);
				break;

			case PWM_STERING:
				lcd_setcursor(0, 1);
				lcd_int(pwm_stering, 16);
				break;

			case PID_STERING_P:
				lcd_setcursor(0, 1);
				lcd_int(pid_stering.pFactor, 16);
				break;

			case PID_STERING_I:
				lcd_setcursor(0, 1);
				lcd_int(pid_stering.iFactor, 16);
				break;

			case PID_DRIVE_P:
				lcd_setcursor(0, 1);
				lcd_int(pid_drive.pFactor, 16);
				break;

			case PID_DRIVE_I:
				lcd_setcursor(0, 1);
				lcd_int(pid_drive.iFactor, 16);
				break;

			case ADC_STERING_LEFT:
				lcd_setcursor(0, 1);
				lcd_int(adc_stering_left, 5);
				lcd_bar(6, 1, 10, adc_stering_left / 10);
				break;

			case ADC_STERING_RIGHT:
				lcd_setcursor(0, 1);
				lcd_int(adc_stering_right, 5);
				lcd_bar(6, 1, 10, adc_stering_right / 10);
				break;

			case ADC_BATT_LOGIC:
				lcd_setcursor(10, 1);
				lcd_int(adc_batt_logic, 6);
				break;
		
			case ADC_BATT_DRIVE:
				lcd_setcursor(10, 1);
				lcd_int(adc_batt_drive, 6);
				break;
		}
	}

	return 0;
}
 
