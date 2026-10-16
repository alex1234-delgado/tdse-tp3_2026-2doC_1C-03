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

/********************** inclusions *******************************************/
/* Project includes */
#include "main.h"

/* Demo includes */
#include "logger.h"
#include "dwt.h"

/* Application & Tasks includes */
#include "board.h"
#include "app.h"

#include "task_actuator_attribute.h"
#include "task_actuator_interface.h"
#include "task_display_attribute.h"
#include "task_display_interface.h"
#include "task_system_attribute.h"
#include "task_system_interface.h"
#include <stdio.h>

/********************** macros and definitions *******************************/
#define DEL_SYS_MIN			0ul
#define DEL_SYS_MED			250ul
#define DEL_SYS_MAX			500ul

#define SYSTEM_DTA_QTY		1ul


/********************** internal data declaration ****************************/
task_system_dta_t task_system_dta_list[SYSTEM_DTA_QTY];
task_system_dta_t task_system_dta_list[SYSTEM_DTA_QTY];

static char lcd_line1[17];
static char lcd_line2[17];

/********************** internal functions declaration ***********************/
static void task_system_update_lcd(void);

/********************** internal data definition *****************************/
const char *p_task_system 		= "Task System (System Statechart)";
const char *p_task_system_ 		= "Non-Blocking Code";
const char *p_task_system__ 	= "(Update by Time Code, period = 1mS)";

/********************** external functions definition ************************/
void task_system_init(void *parameters)
{
	uint32_t index;
	task_system_dta_t 	*p_task_system_dta;
	task_system_st_t	state;
	task_system_ev_t	event;
	bool b_event;

	/* Print out: Task Initialized */
	LOGGER_INFO(" ");
	LOGGER_INFO("  %s is running - Tick [mS] = %lu", GET_NAME(task_system_init), HAL_GetTick());
	LOGGER_INFO("   %s is a %s", GET_NAME(task_system), p_task_system);
	LOGGER_INFO("   %s is a %s", GET_NAME(task_system), p_task_system_);
	LOGGER_INFO("   %s is a %s", GET_NAME(task_system), p_task_system__);

	init_event_task_system();

	for (index = 0; SYSTEM_DTA_QTY > index; index++)
	{
		p_task_system_dta = &task_system_dta_list[index];

		state = ST_SYS_MAIN;
		p_task_system_dta->state = state;

		event = EV_SYS_IDLE;
		p_task_system_dta->event = event;

		b_event = false;
		p_task_system_dta->flag = b_event;

		p_task_system_dta->selected_motor = 0;
		p_task_system_dta->selected_param = 0;

		/* Valores iniciales por defecto pedidos en la guía del TP */
		p_task_system_dta->motor[0].power = MOTOR_POWER_ON;
		p_task_system_dta->motor[0].speed = 8;
		p_task_system_dta->motor[0].spin  = MOTOR_SPIN_LEFT;

		p_task_system_dta->motor[1].power = MOTOR_POWER_ON;
		p_task_system_dta->motor[1].speed = 8;
		p_task_system_dta->motor[1].spin  = MOTOR_SPIN_LEFT;

		LOGGER_INFO(" ");
		LOGGER_INFO("   %s = %lu   %s = %lu   %s = %s",
					GET_NAME(state), (uint32_t)state,
					GET_NAME(event), (uint32_t)event,
					GET_NAME(b_event), (b_event ? "true" : "false"));
	}

	task_system_update_lcd();
}

void task_system_update(void *parameters)
{
	task_system_dta_t *p_task_system_dta = &task_system_dta_list[0];

	if (true == any_event_task_system())
	{
		p_task_system_dta->flag = true;
		p_task_system_dta->event = get_event_task_system();
	}

	if (false == p_task_system_dta->flag)
	{
		return;
	}

	p_task_system_dta->flag = false;
	task_system_ev_t ev = p_task_system_dta->event;
	p_task_system_dta->event = EV_SYS_IDLE;

	uint8_t m = p_task_system_dta->selected_motor;

	switch (p_task_system_dta->state)
	{
		case ST_SYS_MAIN:
			if (EV_SYS_ENTER == ev)
			{
				p_task_system_dta->state = ST_SYS_MENU_1;
			}
			break;

		case ST_SYS_MENU_1:
			if (EV_SYS_NEXT == ev)
			{
				p_task_system_dta->selected_motor = (p_task_system_dta->selected_motor + 1) % 2;
			}
			else if (EV_SYS_ENTER == ev)
			{
				p_task_system_dta->selected_param = 0;
				p_task_system_dta->state = ST_SYS_MENU_2;
			}
			else if (EV_SYS_ESCAPE == ev)
			{
				p_task_system_dta->state = ST_SYS_MAIN;
			}
			break;

		case ST_SYS_MENU_2:
			if (EV_SYS_NEXT == ev)
			{
				p_task_system_dta->selected_param = (p_task_system_dta->selected_param + 1) % 3;
			}
			else if (EV_SYS_ENTER == ev)
			{
				p_task_system_dta->state = ST_SYS_MENU_3;
			}
			else if (EV_SYS_ESCAPE == ev)
			{
				p_task_system_dta->state = ST_SYS_MENU_1;
			}
			break;

		case ST_SYS_MENU_3:
			if (EV_SYS_NEXT == ev)
			{
				if (0 == p_task_system_dta->selected_param)
				{
					p_task_system_dta->motor[m].power = (MOTOR_POWER_ON == p_task_system_dta->motor[m].power) ? MOTOR_POWER_OFF : MOTOR_POWER_ON;
				}
				else if (1 == p_task_system_dta->selected_param)
				{
					p_task_system_dta->motor[m].speed = (p_task_system_dta->motor[m].speed + 1) % 10;
				}
				else if (2 == p_task_system_dta->selected_param)
				{
					p_task_system_dta->motor[m].spin = (MOTOR_SPIN_LEFT == p_task_system_dta->motor[m].spin) ? MOTOR_SPIN_RIGHT : MOTOR_SPIN_LEFT;
				}
			}
			else if ((EV_SYS_ESCAPE == ev) || (EV_SYS_ENTER == ev))
			{
				p_task_system_dta->state = ST_SYS_MENU_2;
			}
			break;

		default:
			p_task_system_dta->state = ST_SYS_MAIN;
			break;
	}

	task_system_update_lcd();
}

static void task_system_update_lcd(void)
{
	task_system_dta_t *p_dta = &task_system_dta_list[0];

	switch (p_dta->state)
	{
		case ST_SYS_MAIN:
			snprintf(lcd_line1, sizeof(lcd_line1), "Motor 1: %s,%lu,%s",
					 p_dta->motor[0].power == MOTOR_POWER_ON ? "ON" : "OFF",
					 p_dta->motor[0].speed,
					 p_dta->motor[0].spin == MOTOR_SPIN_LEFT ? "L" : "R");

			snprintf(lcd_line2, sizeof(lcd_line2), "Motor 2: %s,%lu,%s",
					 p_dta->motor[1].power == MOTOR_POWER_ON ? "ON" : "OFF",
					 p_dta->motor[1].speed,
					 p_dta->motor[1].spin == MOTOR_SPIN_LEFT ? "L" : "R");
			break;

		case ST_SYS_MENU_1:
			snprintf(lcd_line1, sizeof(lcd_line1), "Select Motor:   ");
			snprintf(lcd_line2, sizeof(lcd_line2), "> Motor %d       ", p_dta->selected_motor + 1);
			break;

		case ST_SYS_MENU_2:
			snprintf(lcd_line1, sizeof(lcd_line1), "Motor %d Param:  ", p_dta->selected_motor + 1);
			if (0 == p_dta->selected_param)
			{
				snprintf(lcd_line2, sizeof(lcd_line2), "> Power         ");
			}
			else if (1 == p_dta->selected_param)
			{
				snprintf(lcd_line2, sizeof(lcd_line2), "> Speed         ");
			}
			else
			{
				snprintf(lcd_line2, sizeof(lcd_line2), "> Spin          ");
			}
			break;

		case ST_SYS_MENU_3:
			snprintf(lcd_line1, sizeof(lcd_line1), "Set %-12s",
					 0 == p_dta->selected_param ? "Power:" :
					 1 == p_dta->selected_param ? "Speed:" : "Spin:");

			if (0 == p_dta->selected_param)
			{
				snprintf(lcd_line2, sizeof(lcd_line2), "> %-14s",
						 MOTOR_POWER_ON == p_dta->motor[p_dta->selected_motor].power ? "ON" : "OFF");
			}
			else if (1 == p_dta->selected_param)
			{
				snprintf(lcd_line2, sizeof(lcd_line2), "> %-14lu",
						 p_dta->motor[p_dta->selected_motor].speed);
			}
			else
			{
				snprintf(lcd_line2, sizeof(lcd_line2), "> %-14s",
						 MOTOR_SPIN_LEFT == p_dta->motor[p_dta->selected_motor].spin ? "LEFT" : "RIGHT");
			}
			break;

		default:
			break;
	}

	put_event_task_display(0, 0, lcd_line1);
	put_event_task_display(0, 1, lcd_line2);
}

/********************** end of file ******************************************/
