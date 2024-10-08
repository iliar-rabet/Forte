/*
 * Copyright (c) 201, RISE SICS
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
 *
 */

#include "contiki.h"
#include "net/ipv6/simple-udp.h"
#include "net/routing/routing.h"
#include "net/netstack.h"


/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "RPL BR"
#define LOG_LEVEL LOG_LEVEL_INFO


#define WITH_SERVER_REPLY  1
#define SERVER_PORT	4567

#include "arch/dev/radio/cc2420/cc2420.h"
#include "os/net/mac/tsch/tsch.h"


static struct simple_udp_connection udp_conn;
static bool flag=false;
static uip_ipaddr_t *best_addr;
static struct ctimer ct;
static int max_rss=-90;

#if WITH_SERVER_REPLY
static void timer_events(void *ptr){
  LOG_INFO("CTIMER Sending response to ");
  LOG_INFO_6ADDR(best_addr);
  LOG_INFO_("\n");

  simple_udp_sendto(&udp_conn, "HI", 2, best_addr);

}
#endif

static void
udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{

  LOG_INFO("Received request: %s asn %02x.%08"PRIx32" ",
  data,  
  tsch_current_asn.ms1b, tsch_current_asn.ls4b);
  
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");

#if WITH_SERVER_REPLY
  
  int rss;
  int pktnum;
  char mode[10];
  sscanf((char *)data, "relayed %s %d %d",mode, &pktnum, &rss);
  LOG_INFO("Mode: %s pktnum %d rss %d\n",mode,pktnum,rss);
  if(strcmp(mode, "Unicast") == 0 || !strstr((char *)data, "relayed")){
    LOG_INFO("Responding unicast\n");
    simple_udp_sendto(&udp_conn, data, datalen, sender_addr);
  }
  else{
    if (flag==false){ // first packet
      ctimer_set(&ct, CLOCK_SECOND / 10, timer_events, NULL);
    }
    flag=true;

    if(rss > max_rss){
      max_rss = rss;
      uip_ipaddr_copy(best_addr,sender_addr);
      LOG_INFO_("settign best to: ");
      LOG_INFO_6ADDR(best_addr);
      LOG_INFO_("\n");

    }
  }
  

#endif /* WITH_SERVER_REPLY */
}


/* Declare and auto-start this file's process */
PROCESS(contiki_ng_br, "Contiki-NG Border Router");
AUTOSTART_PROCESSES(&contiki_ng_br);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(contiki_ng_br, ev, data)
{
  PROCESS_BEGIN();

NETSTACK_ROUTING.root_start();
NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, RADIO_TXPOWER_TXPOWER_Neg4dBm);
 
#if BORDER_ROUTER_CONF_WEBSERVER
  PROCESS_NAME(webserver_nogui_process);
  process_start(&webserver_nogui_process, NULL);
#endif /* BORDER_ROUTER_CONF_WEBSERVER */

  LOG_INFO("Contiki-NG Border Router started\n");

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, SERVER_PORT, NULL,
                      SERVER_PORT, udp_rx_callback);

  PROCESS_END();
}
