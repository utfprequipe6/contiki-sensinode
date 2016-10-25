/*
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
 *
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include <string.h>
#include "dev/leds.h"
#include "dev/button-sensor.h"
#include "debug.h"

#define DEBUG DEBUG_PRINT
#include "net/uip-debug.h"

#define SEND_INTERVAL		10 * CLOCK_SECOND
#define MAX_PAYLOAD_LEN		40

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF  ((struct uip_udp_hdr *)&uip_buf[uip_l2_l3_hdr_len])

static char buf[MAX_PAYLOAD_LEN];

/* Our destinations and udp conns. One link-local and one global */
#define LOCAL_CONN_PORT 8802
static struct uip_udp_conn *l_conn;



#define GLOBAL_CONN_PORT 8802

//Meus sockets:
static struct uip_udp_conn *g_conn;
static struct uip_udp_conn *listen;
static struct uip_udp_conn *meuServer;
static struct uip_udp_conn *meuBroadcaster;

int16_t rssi;
int16_t lqi;

#define LED_TOGGLE_REQUEST (0x79) //y no ASCII
#define LED_SET_STATE (0x7A)
#define LED_GET_STATE (0x7B)
#define LED_STATE (0x7C)

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client process");

PROCESS_NAME(ping6_process);
AUTOSTART_PROCESSES(&udp_client_process, &ping6_process);


static void tcpip_handler ( void )
{
	char i=0;

	if( uip_newdata ()) // verifica se novos dados foram recebidos
	{
		char * dados = (( char *) uip_appdata ); // este buffer eh pardao do contiki
		PRINTF("\n\nDados Recebidos: %s\n" , dados);
		PRINTF("comando inicial: %i\n" , dados[0]);

		rssi = (int16_t)packetbuf_attr(PACKETBUF_ATTR_RSSI);
		lqi = (int16_t)packetbuf_attr( PACKETBUF_ATTR_LINK_QUALITY);

		PRINTF("\nRSSi: %i  LQI: %i \n", rssi, lqi);

		switch (dados[0])
		{

		case LED_SET_STATE :
		{
			leds_off(LEDS_ALL);
			leds_on(dados[1]);

			PRINTF(" \nRecebendo LED_SET_STATE de [");
			PRINT6ADDR (& g_conn -> ripaddr );
			PRINTF("]:% u", UIP_HTONS (g_conn -> rport ));
			buf[0]= LED_STATE;
			buf[1]= leds_get();

			uip_udp_packet_send( g_conn , buf, 2);//enviando estado dos leds para o servidor
			uip_udp_packet_send( meuServer , buf, 2);

			 PRINTF ("\nPacote UDP enviado... de: \n");
				  PRINT6ADDR(&uip_ds6_if.addr_list[1].ipaddr);
				  PRINTF(" \npara: \n ");
				   PRINT6ADDR(&g_conn->ripaddr); PRINTF(" e ");PRINT6ADDR(&meuServer->ripaddr);
				   PRINTF(" Porta: %u", UIP_HTONS(g_conn->rport));

			break ;
		}

		default :
		{
			PRINTF(" Comando Invalido :");
			for (i=0;i<uip_datalen ();i++)
			{
				PRINTF("0x%02X" ,dados [i]);
			}
			PRINTF("\n");
			break ;
		}
		}
	}
	return;

}
/*---------------------------------------------------------------------------*/
static void timeout_handler(void)
{  
  memset(buf, 0, MAX_PAYLOAD_LEN); //zera o buffer global

 // memcpy(buf, LED_TOGGLE_REQUEST,1);
  buf[0] = LED_TOGGLE_REQUEST;
  buf[1] = 0;

  //uip_udp_packet_send(g_conn, buf, MAX_PAYLOAD_LEN);
  if ( uip_ds6_get_global( ADDR_PREFERRED) == NULL)
  {
	  PRINTF ("\nErro: Sem IP global valido \n");
  }
  else
  {
	  uip_udp_packet_send( g_conn , buf , 2);
	  uip_udp_packet_send( meuServer, buf,2);
	  uip_udp_packet_send( meuBroadcaster, buf,2);

	  PRINTF ("\nPacote UDP enviado: [LED_TOGGLE_REQUEST] de: \n");
	  PRINT6ADDR(&uip_ds6_if.addr_list[1].ipaddr);
	  PRINTF(" \npara: \n ");
	  PRINT6ADDR(&g_conn->ripaddr); PRINTF(" e ");PRINT6ADDR(&meuServer->ripaddr);

	  PRINTF(" Porta: %u", UIP_HTONS(g_conn->rport));


	  rssi = (int16_t)packetbuf_attr(PACKETBUF_ATTR_RSSI);
	  lqi = (int16_t)packetbuf_attr( PACKETBUF_ATTR_LINK_QUALITY);

	  PRINTF("\nRSSi: %i  LQI: %i \n", rssi, lqi);


  }

}
/*---------------------------------------------------------------------------*/

static void print_local_addresses(void)
{
  int i;
  uint8_t state;
  PRINTF("IPv6 enderecos: [ ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++)
  {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused && (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);  PRINTF(" , ");
      if(state == ADDR_TENTATIVE) {
        uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
      }
    }
  }
  PRINTF(" ] ");
}

PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer et;

  uip_ipaddr_t ipaddr;
  uip_ipaddr_t ipServer;


  uip_ip6addr(&ipServer, 0xbbbb, 0, 0, 0, 0x24a4, 0x4f39, 0xb25f, 0xd17b); //lucas

  uip_ip6addr(&ipaddr  , 0xfe80, 0, 0, 0, 0, 0x19b1, 0x21a0, 0xdd76); //local teste

  PROCESS_BEGIN();
  PRINTF("UDP client process started\n");


  /* new connection with remote host */
  l_conn = udp_new(&ipaddr, UIP_HTONS(LOCAL_CONN_PORT), NULL);
  if(!l_conn) {
    PRINTF("udp_new l_conn error.\n");
  }
  udp_bind(l_conn, UIP_HTONS(LOCAL_CONN_PORT));

  PRINTF("Link-Local connection with ");
  PRINT6ADDR(&l_conn->ripaddr);
  PRINTF(" local/remote port %u/%u\n",
         UIP_HTONS(l_conn->lport), UIP_HTONS(l_conn->rport));

  //Endereco de Destino:
  //bbbb::2c72:b227:8a13:1be7 //bbbb::2838:3518:53c8:a77c
  uip_ip6addr(&ipaddr, 0xbbbb, 0, 0, 0, 0x2838, 0x3518, 0x53c8, 0xa77c); //PC da aula
  // endereço antigo: uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0x0212, 0x4b00, 0x07B9, 0x5ECC);

  g_conn = udp_new(&ipaddr, UIP_HTONS(GLOBAL_CONN_PORT), NULL);

  meuBroadcaster = udp_broadcast_new(8803,NULL);

  listen = udp_new(0, UIP_HTONS(0), NULL);

  meuServer = udp_new(&ipServer, UIP_HTONS(GLOBAL_CONN_PORT), NULL);

  if(!g_conn) PRINTF("udp_new g_conn falha.\n");

  udp_bind(listen, UIP_HTONS(GLOBAL_CONN_PORT));

  print_local_addresses();

  PRINTF("Global connection with ");
  PRINT6ADDR(&g_conn->ripaddr);
  PRINTF(" local/remote port %u/%u\n",
         UIP_HTONS(g_conn->lport), UIP_HTONS(g_conn->rport));

  etimer_set(&et, SEND_INTERVAL);

  while(1)
  {
    PROCESS_WAIT_EVENT();

    if(etimer_expired(&et))
    {
      timeout_handler();
      etimer_restart(&et);
    } else if(ev == tcpip_event)
    {
      tcpip_handler();
    }

  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
