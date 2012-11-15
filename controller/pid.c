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

#include <stdlib.h>
#include <stdint.h>

#include "pid.h"

/**
 * Initialisation of PID controller parameters
 *
 * Initialise the variables used by the PID algorithm
 *
 * @param p_factor	Proportional term
 * @param i_factor	Integral term
 * @param d_factor	Derivate term
 * @param pid		Struct with PID status
 */
void pid_init(int16_t pFactor, int16_t iFactor, int16_t dFactor, struct pid *pid) {
	// Start values for PID controller
	pid->sumError = 0;
	pid->lastProcessValue = 0;

	// Tuning constants for PID loop
	pid->pFactor = pFactor;
	pid->iFactor = iFactor;
	pid->dFactor = dFactor;

	// Limits to avoid overflow
	pid->maxError = MAX_INT / (pid->pFactor + 1);
	pid->maxSumError = MAX_I_TERM / (pid->iFactor + 1);
}

/**
 * PID control algorithm
 *
 * Calculates output from setpoint, process value and PID status
 *
 * @param setPoint	Desired value
 * @param processValue	Measured value
 * @param pid		PID status struct
 */
int16_t pid_controller(int16_t setPoint, int16_t processValue, struct pid *pid) {
	int16_t error, pTerm, dTerm;
	int32_t iTerm, ret, temp;

	error = setPoint - processValue;

	// Calculate pTerm and limit error overflow
	if (error > pid->maxError){
		pTerm = MAX_INT;
	}
	else if (error < -pid->maxError){
		pTerm = -MAX_INT;
	}
	else {
		pTerm = pid->pFactor * error;
	}

	// Calculate iTerm and limit integral runaway
	temp = pid->sumError + error;
	if (temp > pid->maxSumError){
		iTerm = MAX_I_TERM;
		pid->sumError = pid->maxSumError;
	}
	else if (temp < -pid->maxSumError){
		iTerm = -MAX_I_TERM;
		pid->sumError = -pid->maxSumError;
	}
	else {
		pid->sumError = temp;
		iTerm = pid->iFactor * pid->sumError;
	}

	// Calculate Dterm
	dTerm = pid->dFactor * (pid->lastProcessValue - processValue);

	pid->lastProcessValue = processValue;

	ret = (pTerm + iTerm + dTerm) >> SCALING_SHIFT;

	if (ret > MAX_INT){
		ret = MAX_INT;
	}
	else if (ret < -MAX_INT){
		ret = -MAX_INT;
	}

	return (int16_t) ret;
}

/**
 * Resets the integrator
 *
 * Calling this function will reset the integrator in the PID regulator
 */
void pid_reset_integrator(struct pid *pid) {
	pid->sumError = 0;
}
