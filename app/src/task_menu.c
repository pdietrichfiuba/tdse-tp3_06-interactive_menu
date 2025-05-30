/*
 * Copyright (c) 2023 Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>.
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
 *
 * @file   : task_menu.c
 * @date   : Set 26, 2023
 * @author : Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>
 * @version	v1.0.0
 */

/********************** inclusions *******************************************/
/* Project includes. */
#include "main.h"

/* Demo includes. */
#include "logger.h"
#include "dwt.h"

/* Application & Tasks includes. */
#include "board.h"
#include "app.h"
#include "task_menu_attribute.h"
#include "task_menu_interface.h"
#include "display.h"

/********************** macros and definitions *******************************/
#define G_TASK_MEN_CNT_INI			0ul
#define G_TASK_MEN_TICK_CNT_INI		0ul

#define DEL_MEN_XX_MIN				0ul
#define DEL_MEN_XX_MED				50ul
#define DEL_MEN_XX_MAX				500ul

/********************** internal data declaration ****************************/

typedef struct
{
	int parameter;
	bool power;
	int speed;
	bool spin;
} motor_t;

task_menu_dta_t task_menu_dta =
	{DEL_MEN_XX_MIN, ST_MEN_XX_IDLE, EV_MEN_ENT_IDLE, false, 0, 1, false, 0, false};

#define MENU_DTA_QTY	(sizeof(task_menu_dta)/sizeof(task_menu_dta_t))

/********************** internal functions declaration ***********************/

void init_motor (motor_t * motor){


	motor -> parameter = 1;
	motor -> power = false;
	motor -> speed = 0;
	motor -> spin = false;
}

/********************** internal data definition *****************************/
const char *p_task_menu 		= "Task Menu (Interactive Menu)";
const char *p_task_menu_ 		= "Non-Blocking & Update By Time Code";

/********************** external data declaration ****************************/
uint32_t g_task_menu_cnt;
volatile uint32_t g_task_menu_tick_cnt;

/********************** external functions definition ************************/
void task_menu_init(void *parameters)
{
	task_menu_dta_t *p_task_menu_dta;
	task_menu_st_t	state;
	task_menu_ev_t	event;
	bool b_event;

	/* Print out: Task Initialized */
	LOGGER_LOG("  %s is running - %s\r\n", GET_NAME(task_menu_init), p_task_menu);
	LOGGER_LOG("  %s is a %s\r\n", GET_NAME(task_menu), p_task_menu_);

	g_task_menu_cnt = G_TASK_MEN_CNT_INI;

	/* Print out: Task execution counter */
	LOGGER_LOG("   %s = %lu\r\n", GET_NAME(g_task_menu_cnt), g_task_menu_cnt);

	init_queue_event_task_menu();

	/* Update Task Actuator Configuration & Data Pointer */
	p_task_menu_dta = &task_menu_dta;

	/* Print out: Task execution FSM */
	state = p_task_menu_dta->state;
	LOGGER_LOG("   %s = %lu", GET_NAME(state), (uint32_t)state);

	event = p_task_menu_dta->event;
	LOGGER_LOG("   %s = %lu", GET_NAME(event), (uint32_t)event);

	b_event = p_task_menu_dta->flag;
	LOGGER_LOG("   %s = %s\r\n", GET_NAME(b_event), (b_event ? "true" : "false"));

	cycle_counter_init();
	cycle_counter_reset();

	displayInit( DISPLAY_CONNECTION_GPIO_4BITS );

    displayCharPositionWrite(0, 0);
	displayStringWrite("TdSE Bienvenidos");

	displayCharPositionWrite(0, 1);
	displayStringWrite("Test Nro: ");

	g_task_menu_tick_cnt = G_TASK_MEN_TICK_CNT_INI;
}

void task_menu_update(void *parameters)
{
	task_menu_dta_t *p_task_menu_dta;
	bool b_time_update_required = false;
	char menu_str[8];
	motor_t motors[QTY_MOTORS];


	for(int i = 0 ; i< QTY_MOTORS ; i++)
	{
		init_motor(&motors[i]);
	}


	/* Update Task Menu Counter */
	g_task_menu_cnt++;

	/* Protect shared resource (g_task_menu_tick) */
	__asm("CPSID i");	/* disable interrupts*/
    if (G_TASK_MEN_TICK_CNT_INI < g_task_menu_tick_cnt)
    {
    	g_task_menu_tick_cnt--;
    	b_time_update_required = true;
    }
    __asm("CPSIE i");	/* enable interrupts*/

    while (b_time_update_required)
    {
		/* Protect shared resource (g_task_menu_tick) */
		__asm("CPSID i");	/* disable interrupts*/
		if (G_TASK_MEN_TICK_CNT_INI < g_task_menu_tick_cnt)
		{
			g_task_menu_tick_cnt--;
			b_time_update_required = true;
		}
		else
		{
			b_time_update_required = false;
		}
		__asm("CPSIE i");	/* enable interrupts*/

    	/* Update Task Menu Data Pointer */
		p_task_menu_dta = &task_menu_dta;

    	if (DEL_MEN_XX_MIN < p_task_menu_dta->tick)
		{
			p_task_menu_dta->tick--;
		}
		else
		{
			snprintf(menu_str, sizeof(menu_str), "%lu", (g_task_menu_cnt/1000ul));
			displayCharPositionWrite(10, 1);
			displayStringWrite(menu_str);

			p_task_menu_dta->tick = DEL_MEN_XX_MAX;

			if (true == any_event_task_menu())
			{
				p_task_menu_dta->flag = true;
				p_task_menu_dta->event = get_event_task_menu();
			}


			switch (p_task_menu_dta->state)
			{
				case ST_MENU_1:

					// actions - next
					if(EV_MEN_NEX_ACTIVE == p_task_menu_dta->event && p_task_menu_dta->id_motor < QTY_MOTORS)
					{
						p_task_menu_dta->id_motor ++;
					}

					if(EV_MEN_NEX_ACTIVE == p_task_menu_dta->event && p_task_menu_dta->id_motor >= QTY_MOTORS)
					{
						p_task_menu_dta->id_motor = 0;
					}

					// actions - enter
					if (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event)
					{
						p_task_menu_dta->state = ST_MENU_2;
					}

					// actions - esc
					if(EV_MEN_ESC_ACTIVE == p_task_menu_dta->event)
					{
						p_task_menu_dta->state = ST_MENU_1;
					}


					break;

				case ST_MENU_2:
					//actions - enter
					if (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event &&  p_task_menu_dta->parameter==1)
					{
						p_task_menu_dta->state = ST_POWER;
					}
					if (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event &&  p_task_menu_dta->parameter==2)
					{
						p_task_menu_dta->state = ST_SPEED;
					}
					if (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event &&  p_task_menu_dta->parameter==3)
					{
						p_task_menu_dta->state = ST_SPIN;
					}

					// actions - next
					if(EV_MEN_NEX_ACTIVE == p_task_menu_dta->event &&  p_task_menu_dta->parameter<3)
					{
						p_task_menu_dta->state = ST_MENU_2;
						p_task_menu_dta->parameter ++ ;
					}
					if(EV_MEN_NEX_ACTIVE == p_task_menu_dta->event &&  p_task_menu_dta->parameter == 3)
					{
						p_task_menu_dta->state = ST_MENU_2;
						p_task_menu_dta->parameter =1 ;
					}

					// actions - esc
					if(EV_MEN_ESC_ACTIVE == p_task_menu_dta->event)
					{
						p_task_menu_dta->state = ST_MENU_1;
					}

				case ST_POWER:
					// actions - next
					if(EV_MEN_NEX_ACTIVE == p_task_menu_dta->event &&  p_task_menu_dta->power  )
					{
						p_task_menu_dta->power = false ;
					}
					if(EV_MEN_NEX_ACTIVE == p_task_menu_dta->event &&  !p_task_menu_dta->power  )
					{
						p_task_menu_dta->power = true ;
					}

					// actions - esc
					if(EV_MEN_ESC_ACTIVE == p_task_menu_dta->event )
					{
						p_task_menu_dta->state = ST_MENU_2;
					}

					// actions - enter
					if(EV_MEN_ENT_ACTIVE == p_task_menu_dta->event )
					{
						motors[p_task_menu_dta->id_motors].power = p_task_menu_dta->power;
					}


				case ST_SPEED :

					// actions - next
					if(EV_MEN_NEX_ACTIVE == p_task_menu_dta->event &&  p_task_menu_dta->speed < 9  )
					{
						p_task_menu_dta->speed ++ ;
					}
					if(EV_MEN_NEX_ACTIVE == p_task_menu_dta->event &&  p_task_menu_dta->speed >= 9 )
					{
						p_task_menu_dta->speed = 0 ;
					}

					// actions - esc
					if(EV_MEN_ESC_ACTIVE == p_task_menu_dta->event )
					{
						p_task_menu_dta->state = ST_MENU_2;
					}

					// actions - enter
					if(EV_MEN_ENT_ACTIVE == p_task_menu_dta->event )
					{
						motors[p_task_menu_dta->id_motors].speed = p_task_menu_dta->speed;
					}


				case ST_SPIN :

					// actions - next
					if(EV_MEN_NEX_ACTIVE == p_task_menu_dta->event &&  p_task_menu_dta->spin  )
					{
						p_task_menu_dta->spin = false ;
					}
					if(EV_MEN_NEX_ACTIVE == p_task_menu_dta->event &&  !p_task_menu_dta->spin )
					{
						p_task_menu_dta->spin = true ;
					}

					// actions - esc
					if(EV_MEN_ESC_ACTIVE == p_task_menu_dta->event )
					{
						p_task_menu_dta->state = ST_MENU_2;
					}

					// actions - enter
					if(EV_MEN_ENT_ACTIVE == p_task_menu_dta->event )
					{
						motors[p_task_menu_dta->id_motors].spin = p_task_menu_dta->spin;
					}

					break;

				default:

					p_task_menu_dta->tick  = DEL_MEN_XX_MIN;
					p_task_menu_dta->state = ST_MEN_XX_IDLE;
					p_task_menu_dta->event = EV_MEN_ENT_IDLE;
					p_task_menu_dta->flag  = false;

					break;
			}
		}
	}
}

/********************** end of file ******************************************/
