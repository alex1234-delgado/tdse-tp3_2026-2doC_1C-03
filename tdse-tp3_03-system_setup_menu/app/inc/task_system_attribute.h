/*
 * Copyright (c) 2026 Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @author : Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>
 */

#ifndef TASK_SYSTEM_ATTRIBUTE_H_
#define TASK_SYSTEM_ATTRIBUTE_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/
#include <stdint.h>
#include <stdbool.h>

/********************** macros ***********************************************/

/********************** typedef **********************************************/

/* Tipos de datos para los parámetros de los Motores */
typedef enum {
	MOTOR_POWER_OFF = 0,
	MOTOR_POWER_ON
} task_system_power_t;

typedef enum {
	MOTOR_SPIN_LEFT = 0,
	MOTOR_SPIN_RIGHT
} task_system_spin_t;

typedef struct {
	task_system_power_t power;
	uint32_t speed;             /* Velocidad entre 0 y 9 */
	task_system_spin_t spin;
} task_system_motor_t;

/* Eventos enviados desde la tarea Sensor al presionar los botones */
typedef enum task_system_ev {
	EV_SYS_IDLE,
	EV_SYS_BTN_A,
	EV_SYS_ENTER,
	EV_SYS_NEXT,
	EV_SYS_ESCAPE
} task_system_ev_t;

/* Estados de la FSM del Menú de Configuración (4 Niveles) */
typedef enum task_system_st {
	ST_SYS_MAIN = 0,    /* Nivel 1: Resumen de Motor 1 y Motor 2 */
	ST_SYS_MENU_1,      /* Nivel 2: Seleccionar Motor (1 o 2) */
	ST_SYS_MENU_2,      /* Nivel 3: Seleccionar Parámetro (Power, Speed, Spin) */
	ST_SYS_MENU_3       /* Nivel 4: Modificar Valor del Parámetro */
} task_system_st_t;

/* Estructura de datos de la tarea System */
typedef struct {
	uint32_t            tick;
	task_system_st_t    state;
	task_system_ev_t    event;
	bool                flag;

	/* Datos y navegación del menú de motores */
	task_system_motor_t motor[2];
	uint8_t             selected_motor; /* 0 = Motor 1, 1 = Motor 2 */
	uint8_t             selected_param; /* 0 = Power, 1 = Speed, 2 = Spin */
} task_system_dta_t;
/********************** external data declaration ****************************/
extern task_system_dta_t task_system_dta_list[];

/********************** external functions declaration ***********************/

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* TASK_SYSTEM_ATTRIBUTE_H_ */

/********************** end of file ******************************************/
