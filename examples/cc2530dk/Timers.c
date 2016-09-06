/* This is a very simple hello_world program.
 * It aims to demonstrate the co-existence of two processes:
 * One of them prints a hello world message and the other blinks the LEDs
 *
 * It is largely based on hello_world in $(CONTIKI)/examples/sensinode
 *
 * Author: George Oikonomou - <oikonomou@users.sourceforge.net>
 * Alterado para a disciplina de Rede de Sensores Sem Fio
 */

#include "contiki.h"
#include "dev/leds.h"

#include <stdio.h> /* For printf() */
/*---------------------------------------------------------------------------*/
#define LED_PING_EVENT	(44)
#define LED_PONG_EVENT	(45)

static struct etimer et_hello;
static struct etimer et_blink;
static struct etimer timer3;
static struct etimer timer4;
static uint16_t count;
static uint8_t blinks;

//Declaração dos jobs
/*---------------------------------------------------------------------------*/
PROCESS(hello_world_process, "Hello world process");
PROCESS(blink_process, "LED blink process");
PROCESS(processo3, "proc3_process");
PROCESS(processo4, "pong_process");

//Inicia o processo assim que possível
AUTOSTART_PROCESSES(&blink_process, &hello_world_process, &processo3, &processo4); //Inicia assim que o OS for ligado


// PROCESSO 1
PROCESS_THREAD(hello_world_process, ev, data)
{
  PROCESS_BEGIN();

  etimer_set(&et_hello, CLOCK_SECOND * 4);
  count = 0;

  while(1)
  {
    PROCESS_WAIT_EVENT();

    if(ev == PROCESS_EVENT_TIMER)
    {
      printf("Ola Mundo #%u!\n", count);
      count++;

      etimer_reset(&et_hello);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
//PROCESSO 2
PROCESS_THREAD(blink_process, ev, data)
{
  PROCESS_BEGIN();

  blinks = 0;
  printf("LEDS_ALL = %d\n", LEDS_ALL);

  leds_on(LEDS_ALL);
  etimer_set(&et_blink, 2*CLOCK_SECOND);
  PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);

  printf("Ola Mundo dos LEDs\n");
  //Task 2:
  while(1)
  {
    etimer_set(&et_blink, 2*CLOCK_SECOND);

    PROCESS_WAIT_EVENT();
    if(ev == PROCESS_EVENT_TIMER)
    {
    	process_post(&processo4,LED_PING_EVENT, (void*)(&blink_process));

    	leds_toggle(0xFF & LEDS_YELLOW);

    	printf("Pisca... (state %X)\n", leds_get());

    }
    if(ev == LED_PONG_EVENT)
    {
    	printf("Pong\n");
  }
  }
  PROCESS_END();
}

//PROCESSO 3

PROCESS_THREAD(processo3, ev, fila)
{
	PROCESS_BEGIN();

	etimer_set(&timer3, 5*CLOCK_SECOND);

	while(1)
	{
		PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
		leds_toggle(LEDS_RED);
		etimer_reset(&timer3);
		printf("Pisca... (state %X)\n", leds_get());
	}
	PROCESS_END();
}

/*---------------------------------------------------------------------------*/
// PROCESSO 4
PROCESS_THREAD(processo4, ev, data)
{

	PROCESS_BEGIN();

	while(1)
	{

		PROCESS_WAIT_EVENT();

		if(ev == LED_PING_EVENT)
		{
			etimer_set(&timer4, CLOCK_SECOND/5);
			leds_on(LEDS_BLUE);
			printf("pong: Led ping");
			process_post(&blink_process,LED_PONG_EVENT, NULL);

			//função delay com yield
			PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
			leds_off(LEDS_BLUE);
			etimer_stop(&timer4);

		}
	}

  PROCESS_END();
}
