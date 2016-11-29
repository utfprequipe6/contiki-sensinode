#include <stdio.h>
#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-conf.h"
#include "contiki-net.h"
#include "dev/leds.h"

#include "dev/hal_i2c.h"

#include "dev/adc-sensor.h"
#include "dev/button-sensor.h"
#include "dev/bmp280.h"
#include "dev/bmp280-sensor.h"

#define PRINTF(...) printf(__VA_ARGS__)

#define INTERVALO 2 //  leitura do sensor

static struct uip_udp_conn *com;

uip_ipaddr_t endereco;

uint16_t temperatura = 0x00;

static void configNET()
{
	uip_ip6addr(&endereco, 0xbbbb,0,0,0,0,0,0,1 );

	com = udp_new(&endereco,UIP_HTONS(8802),NULL);

}
PROCESS(sensors_test_process, "Sensor Test Process");
PROCESS(buttons_test_process, "Button Test Process");

AUTOSTART_PROCESSES(&sensors_test_process, &buttons_test_process);

PROCESS_THREAD(buttons_test_process, ev, data)
{
  struct sensors_sensor *sensor;

  PROCESS_BEGIN();

  while(1) {

    PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event);

    sensor = (struct sensors_sensor *)data;
    if(sensor == &button_sensor) {
      PRINTF("Botao precionado...\n");
      leds_toggle(LEDS_GREEN);
    }
  }

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sensors_test_process, ev, data)
{
  static struct etimer et;
  static struct sensors_sensor *sensores;

  int sensorOK = 0;

 PROCESS_BEGIN();

  PRINTF("\nInicializando os sensores BMP280...\n");


  //iniciar o timer (duty cicle dos sensores)
  etimer_set(&et, CLOCK_SECOND * INTERVALO);


  sensores = sensors_find(BMP_SENSOR);

  if(sensores)
  {
	  PRINTF("\nSensores BMP Ativado\n");
	  sensorOK = 1;
  } else PRINTF("\nFalha no Sensor BMP\n");

  PRINTF("\nEstado do sensor: %i ", sensores->status);

  while(1)
  {

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    if(sensorOK)
    temperatura = sensores->value(BMP_SENSOR_TYPE_TEMP);

    PRINTF("\nTemperatura: %i ", temperatura);


    etimer_reset(&et);
  }


  PROCESS_END();
}
