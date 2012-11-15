/**
 * Ansteuerung eines HD44780 kompatiblen LCD im 4-Bit-Interfacemodus
 *
 * Die Pinbelegung ist Ã¼ber defines in lcd.h einstellbar
 *
 * @copyright	2012 Institute Automation of Complex Power Systems (ACS), RWTH Aachen University
 * @license	http://www.gnu.org/licenses/gpl.txt GNU Public License
 * @link http://www.mikrocontroller.net/articles/HD44780
 * @link http://www.mikrocontroller.net/articles/AVR-GCC-Tutorial/LCD-Ansteuerung
 */

#include <stdlib.h>
#include <string.h>

#include <avr/eeprom.h>

#include "lcd.h"

/**
 * Erzeugt einen Enable-Puls
 */
static void lcd_enable() {
	LCD_PORT |= (1<<LCD_EN);	// Enable auf 1 setzen
	_delay_us( LCD_ENABLE_US );	// kurze Pause
	LCD_PORT &= ~(1<<LCD_EN);	// Enable auf 0 setzen
}

/**
 * Sendet eine 4-bit Ausgabeoperation an das LCD
 */
static void lcd_out(uint8_t data) {
	data &= 0xF0;				// obere 4 Bit maskieren

	LCD_PORT &= ~(0xF0>>(4-LCD_DB));	// Maske löschen
	LCD_PORT |= (data>>(4-LCD_DB));		// Bits setzen
	lcd_enable();
}

/**
 * Initialisierung: muss ganz am Anfang des Programms aufgerufen werden
 */
void lcd_init() {
	// verwendete Pins auf Ausgang schalten
	uint8_t pins = (0x0F << LCD_DB) |	// 4 Datenleitungen
			(1<<LCD_RS) |		// R/S Leitung
			(1<<LCD_EN);		// Enable Leitung
	LCD_DDR |= pins;

	// initial alle AusgÃ¤nge auf Null
	LCD_PORT &= ~pins;

	// warten auf die Bereitschaft des LCD
	_delay_ms( LCD_BOOTUP_MS );

	// Soft-Reset muss 3mal hintereinander gesendet werden zur Initialisierung
	lcd_out( LCD_SOFT_RESET );
	_delay_ms( LCD_SOFT_RESET_MS1 );

	lcd_enable();
	_delay_ms( LCD_SOFT_RESET_MS2 );

	lcd_enable();
	_delay_ms( LCD_SOFT_RESET_MS3 );

	// 4-bit Modus aktivieren
	lcd_out( LCD_SET_FUNCTION |
			 LCD_FUNCTION_4BIT );
	_delay_ms( LCD_SET_4BITMODE_MS );

	// 4-bit Modus / 2 Zeilen / 5x7
	lcd_command( LCD_SET_FUNCTION |
		LCD_FUNCTION_4BIT |
		LCD_FUNCTION_2LINE |
		LCD_FUNCTION_5X7 );

	// Display ein / Cursor aus / Blinken aus
	lcd_command( LCD_SET_DISPLAY |
		LCD_DISPLAY_ON |
		LCD_CURSOR_OFF |
		LCD_BLINKING_OFF);

	// Cursor inkrement / kein Scrollen
	lcd_command( LCD_SET_ENTRY |
		LCD_ENTRY_INCREASE |
		LCD_ENTRY_NOSHIFT );

	lcd_clear();
}

/**
 * Sendet ein Datenbyte an das LCD
 */
void lcd_data(uint8_t data) {
	LCD_PORT |= (1<<LCD_RS);	// RS auf 1 setzen

	lcd_out(data);			// zuerst die oberen,
	lcd_out(data<<4);		// dann die unteren 4 Bit senden

	_delay_us( LCD_WRITEDATA_US );
}

/**
 * Sendet einen Befehl an das LCD
 */
void lcd_command(uint8_t data) {
	LCD_PORT &= ~(1<<LCD_RS);	// RS auf 0 setzen

	lcd_out(data);			// zuerst die oberen,
	lcd_out(data<<4);		// dann die unteren 4 Bit senden

	_delay_us(LCD_COMMAND_US);
}

/**
 * Sendet den Befehl zur Löschung des Displays
 */
void lcd_clear() {
	lcd_command(LCD_CLEAR_DISPLAY);
	_delay_ms(LCD_CLEAR_DISPLAY_MS);
}

/**
 * Sendet den Befehl: Cursor Home
 */
void lcd_home() {
	lcd_command(LCD_CURSOR_HOME);
	_delay_ms(LCD_CURSOR_HOME_MS);
}

/**
 * Setzt den Cursor in Spalte x (0..15) Zeile y (1..4)
 */
void lcd_setcursor(uint8_t x, uint8_t y) {
	uint8_t data;

	switch (y) {
		case 0:	// 1. Zeile
			data = LCD_SET_DDADR + LCD_DDADR_LINE1 + x;
			break;

		case 1:	// 2. Zeile
			data = LCD_SET_DDADR + LCD_DDADR_LINE2 + x;
			break;

		case 2:	// 3. Zeile
			data = LCD_SET_DDADR + LCD_DDADR_LINE3 + x;
			break;

		case 3:	// 4. Zeile
			data = LCD_SET_DDADR + LCD_DDADR_LINE4 + x;
			break;

		default:
			return;								   // für den Fall einer falschen Zeile
	}

	lcd_command(data);
}

/**
 * Schreibt einen String auf das LCD
 */
void lcd_string(const char *data) {
	while (*data != '\0') {
		lcd_data(*data++);
	}
}

/**
 * Schreibt ein Zeichen in den Character Generator RAM
 */
void lcd_generatechar(uint8_t code, const uint8_t *data) {
	// Startposition des Zeichens einstellen
	lcd_command(LCD_SET_CGADR | (code<<3));

	// Bitmuster übertragen
	for (uint8_t i=0; i<8; i++) {
		lcd_data(data[i]);
	}
}

/**
 * Ausgabe Balkenanzeige
 */
void lcd_bar(uint8_t startx, uint8_t starty, uint8_t length, uint8_t percent) {

	uint8_t i;
	if (percent > 100) return;

	lcd_setcursor(startx, starty);
	lcd_data(0b00111100); // <

	for (i=0; i < length-2; i++) {
		lcd_setcursor( startx+1+i , starty );

		if (percent*(length-2)/100 <= i ) {
			lcd_data('-');
		}
		else {
			lcd_data(0b11111111);
		}
	}

	lcd_setcursor(startx+length-1, starty );
	lcd_data(0b00111110); // >
}

/**
 * Ausgabe einer Ganzzahl mit führenden Leerzeichen
 */
void lcd_int(int16_t number, uint8_t length) {
	char buffer[length+1];
	bool vz = true;
	bool zf = true;
	bool neg = (number < 0);

	buffer[length] = '\0';

	for (int8_t counter = length-1; counter >= 0; counter--) {
		if (zf) {
			buffer[counter] = '0' + (abs(number) % 10);
			number /= 10;
			zf = (bool) number;
		}
		else if (vz && neg) {
			buffer[counter] = '-';
			vz = false;
		}
		else {
			buffer[counter] = ' ';
		}
	}

	lcd_string(buffer);
}

