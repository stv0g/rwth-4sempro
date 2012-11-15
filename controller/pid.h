/**
 * General PID implementation for AVR
 *
 * Discrete PID controller implementation. Set up by giving P/I/D terms
 * to pid_init(), and uses a struct pid to store internal values.
 * 
 * Based on ATMEL AppNote: AVR221 - Discrete PID controller
 *
 * @copyright	2012 Institute Automation of Complex Power Systems (ACS), RWTH Aachen University
 * @license	http://www.gnu.org/licenses/gpl.txt GNU Public License
 * @author	Atmel Corporation <avr@atmel.com>
 * @link	http://www.atmel.com
 * @revision	456
 * @date 	2006-02-16 12:46:13
 */

#ifndef _PID_H_
#define _PID_H_

#include <stdlib.h>
#include <stdint.h>

#define SCALING_FACTOR	128
#define SCALING_SHIFT 7

/**
 * PID Status
 *
 * Setpoints and data used by the PID control algorithm
 */
struct pid {
	int16_t lastProcessValue;	// Last process value, used to find derivative of process value.
	int32_t sumError;		// Summation of errors, used for integrate calculations
	int16_t pFactor;		// The Proportional tuning constant, multiplied with SCALING_FACTOR
	int16_t iFactor;		// The Integral tuning constant, multiplied with SCALING_FACTOR
	int16_t dFactor;		// The Derivative tuning constant, multiplied with SCALING_FACTOR
	int16_t maxError;		// Maximum allowed error, avoid overflow
	int32_t maxSumError;		// Maximum allowed sumerror, avoid overflow
};

/**
 * Maximum values
 *
 * Needed to avoid sign/overflow problems
 */
#define MAX_INT		INT16_MAX
#define MAX_LONG	INT32_MAX
#define MAX_I_TERM	(MAX_LONG / 2)

void pid_init(int16_t pFactor, int16_t iFactor, int16_t dFactor, struct pid *pid);
int16_t pid_controller(int16_t setPoint, int16_t processValue, struct pid *pid);
void pid_reset_integrator(struct pid *pid);

#endif /* _PID_H_ */
