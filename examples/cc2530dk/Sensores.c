/*
 * Copyright (c) 2010, Loughborough University - Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *         Example to demonstrate-test cc2530 sensor functionality
 *
 *         B1 turns LED_GREEN on and off.
 *
 *         The node takes readings from the various sensors every x seconds and
 *         prints out the results.
 *
 *         We use floats here to translate the AD conversion results to
 *         meaningful values. However, our printf does not have %f support so
 *         we use an ugly hack to print out the value by extracting the integral
 *         part and then the fractional part. Don't try this at home.
 *
 *         Temperature:
 *           Math is correct, the sensor needs calibration per device.
 *           I currently use default values for the math which may result in
 *           very incorrect values in degrees C.
 *           See TI Design Note DN102 about the offset calibration.
 *
 *         Supply Voltage (VDD):
 *           For VDD, math is correct, conversion is correct.
 *           See DN101 for details.
 *
 *         Make sure you enable/disable things in contiki-conf.h
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#include "contiki.h"
#include "contiki-conf.h"
#include "dev/leds.h"

#include "dev/button-sensor.h"
#include "dev/adc-sensor.h"

#define DEBUG 1
#define BOTAO1 1


#include <stdio.h>

static int rv;
static int rv1;
static int rv2;
static int dec;
static float frac;
static int dec1;
static float frac1;
static int botao_apertado = 0x00;

#define PRINTF(...) printf(__VA_ARGS__)

/*---------------------------------------------------------------------------*/
PROCESS(sensors_test_process, "Sensor Test Process");


PROCESS(buttons_test_process, "Button Test Process");
AUTOSTART_PROCESSES(&sensors_test_process, &buttons_test_process);


PROCESS_THREAD(buttons_test_process, ev, data)
{
  struct sensors_sensor *sensor;

  PROCESS_BEGIN();

  while(1) {

    PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event);

    /* If we woke up after a sensor event, inform what happened */
    sensor = (struct sensors_sensor *)data;
    if(sensor == &button_sensor)
    {
      PRINTF("Botao pressionado\n");

      PRINTF("Temp=%d.%02u C) \n", dec, (unsigned int)(frac*100));//Temperatura


      //PRINTF("Supply=%d.%02u V (%d) : %i\n", dec1, (unsigned int)(frac1*100),(unsigned int)rv1);//Tensao


      PRINTF("AIN6 valor lido: %d\n",(unsigned int) rv2);


      botao_apertado = 1;

      leds_on(LEDS_YELLOW);
    }
  }

  PROCESS_END();
}


/*---------------------------------------------------------------------------*/

uint8_t temperatura;

PROCESS_THREAD(sensors_test_process, ev, data)
{
  static struct etimer et;

  /* Sensor Values */
 // static int rv;
  static struct sensors_sensor *sensor;
  static float sane = 0;


  PROCESS_BEGIN();

  PRINTF("========================\n");
  PRINTF("Iniciando sensores\n");
  PRINTF("========================\n");

  /* Set an etimer. We take sensor readings when it expires and reset it. */
  etimer_set(&et, CLOCK_SECOND * 2);

  while(1)
  {

    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);

    leds_off(LEDS_YELLOW);

    sensor = sensors_find(ADC_SENSOR);


    if(sensor)
    {
      PRINTF("evento de medicao...\n");
      leds_on(LEDS_RED);
      /*
       * Temperature:
       * Using 1.25V ref. voltage (1250mV).
       * Typical AD Output at 25°C: 1480
       * Typical Co-efficient     : 4.5 mV/°C
       *
       * Thus, at 12bit decimation (and ignoring the VDD co-efficient as well
       * as offsets due to lack of calibration):
       *
       *          AD - 1480
       * T = 25 + ---------
       *              4.5
       */
      rv = sensor->value(ADC_SENSOR_TYPE_TEMP);


      if(rv != -1)
      {
        sane = 25 + ((rv - 1480) / 4.5);
        dec = sane;
        frac = sane - dec;
      //  PRINTF("  Temp=%d.%02u C (%d)\n", dec, (unsigned int)(frac*100), rv);
      }
      /*
       * Power Supply Voltage.
       * Using 1.25V ref. voltage.
       * AD Conversion on VDD/3
       *
       * Thus, at 12bit resolution:
       *
       *  PROCESS_BEGIN();        ADC x 1.25 x 3
       *                 Supply = -------------- V
       *                               2047
       */
      rv1 = sensor->value(ADC_SENSOR_TYPE_VDD);


      if(rv1 != -1)
      {
        //sane = rv * 3.75 / 2047;
    	//sane = ((3*(1.0512))*rv1)/(2047-rv1*0.12);
    	sane = (rv1*3.30/2047);

    	dec1 = sane;
    	frac1 = sane - dec;


        if(botao_apertado == 1)
        {
        	PRINTF("  Vcc=%d.%02u C (%u) \n", dec1, (unsigned int)(frac1*100), rv);
			botao_apertado = 0;
        }

      }

      rv2 = sensor->value(6);//ADC_SENSOR_TYPE_AIN6

      if(rv2 == -1)
      {
    	 PRINTF("AIN6 - Falha \n");
      }

      /*
       * Battery Voltage - ToDo
       *   rv = sensor->value(ADC_SENSOR_TYPE_BATTERY);
       */

      leds_off(LEDS_RED);
    }
    etimer_reset(&et);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
