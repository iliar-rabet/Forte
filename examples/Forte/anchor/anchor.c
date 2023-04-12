#include "contiki.h"
#include "project-conf.h"
#include "simple-udp.h"
#include "lib/random.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/sicslowpan.h"
#include "net/ipv6/uiplib.h"
#include "net/routing/routing.h"
#include "net/routing/rpl-lite/rpl-timers.h"
#include "sys/etimer.h"
#include "sys/log.h"
#include "net/packetbuf.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "arch/dev/radio/cc2420/cc2420.h"
#include "os/net/mac/tsch/tsch.h"


#define TSCH_LOG_CONF_PER_SLOT                     1

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO
#undef UIP_CONF_TCP

#define SERVER_PT 4567
#define MN_PT 5678

static struct simple_udp_connection server_udp, mn_udp;
static uip_ipaddr_t dest_ipaddr;
static char mobile_str_addr[21];
static uip_ipaddr_t *mn_addr;

/*--------------------------Linked List -- LL -------------------------------*/
struct node {
  char *mob_addr;
  struct node* next;
};

// static struct node* LL_head = NULL;

// static void
// LL_append(struct node** head_ref, const char *new_addr)
// {
//   printf("in append\n");
    
//   struct node *new_node = (struct node*) malloc(sizeof(struct node));
//   struct node *last = *head_ref;
//   printf("malloc node done\n");
//   new_node->mob_addr = (char *) malloc(21 * sizeof(char));
//   printf("malloc string done\n");

//   strcpy(new_node->mob_addr, (const char *) new_addr);
//   new_node->next = NULL;
//   printf("before if\n");
//   if (*head_ref == NULL) {
//     *head_ref = new_node;
//     return;
//   }
//   printf("before while\n");
//   while (last->next != NULL){
//     printf("%s\n",last->mob_addr);
//     last = last->next;
//   }
//   printf("after while\n");
//   last->next = new_node;
//   return;
// }

// static int
// LL_del_srch_node(int opt, struct node** head_ref, const char *key)
// {
  
//   struct node *temp = *head_ref, *prev = NULL;
//   /* opt = 0 for search the node
//            1 for delete the node
//      ret = 0 if node not in list
//            1 if node is  in list
//            2   delete is success
//   */
//   if(temp==NULL)
//     return 0;
//   if(temp->mob_addr==NULL)
//     return 0;
//   if (temp != NULL && !strcmp(temp->mob_addr, key)) { //the first node is a match
//     if (opt) {
//       // printf("in the first if statement");
//       *head_ref = temp->next;
//       memset(temp->mob_addr, '\0', 21 * sizeof(char));
//       free(temp->mob_addr);
//       free(temp);
//       return 2;
//     }
//     return 1;
//   }
//   while (temp != NULL && strcmp(temp->mob_addr, key)) {
//     prev = temp;
//     temp = temp->next;
//   }
//   if (temp == NULL)
//     return 0;

//   if (opt) {
//     printf("now removing\n");
//     prev->next = temp->next;
//     memset(temp->mob_addr, '\0', 21 * sizeof(char));
//     free(temp->mob_addr);
//     free(temp);
//     return 2;
//   }
//   return 1;
// }
/*--------------------------Callback functions-------------------------------*/
static void
mn_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  char str[40];
  uip_ipaddr_copy(mn_addr,sender_addr);

  memset(mobile_str_addr, '\0', 21 * sizeof(char));
  uiplib_ipaddr_snprint(mobile_str_addr, 21, sender_addr);

  int16_t packet_rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
  // sprintf(str, "relayed %s %d from %s", data, packet_rssi,mobile_str_addr);
  sprintf(str, "relayed %s %d", data, packet_rssi);
  
  LOG_INFO("Relaying: %s asn %02x.%08"PRIx32"\n",
  str,  
  tsch_current_asn.ms1b, tsch_current_asn.ls4b);

  if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
    simple_udp_sendto(&server_udp, str, strlen(str), &dest_ipaddr);
  }
  else
  LOG_INFO("CAN't Relay\n");
}


static void
server_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  simple_udp_sendto(&server_udp, (char *)data, strlen((char *)data), mn_addr);
  LOG_INFO("Down-Relaying: %s asn %02x.%08"PRIx32"\n",
  data,  
  tsch_current_asn.ms1b, tsch_current_asn.ls4b);

}

  // char mn[21];

  // sprintf(mn,"%.20s",data+4);
  
  // printf("Received response: [%s] for [%s]\n", data,mn);

  // if (data[0]=='S') {
  //   if(LL_del_srch_node(0,&LL_head, (const char *) mn)==0){
  //     LL_append(&LL_head, (const char *) mn);
  //     handoff_flag=0;
  //     printf("NOW BEING BEST for %s\n", mn);
  //   }
  //   else{
  //     printf("already BEST\n");
  //   }
  // } else if (data[0]=='N') {
  //   LL_del_srch_node(1, &LL_head, (const char *) mn);
  // }
  // return;

// static void
// data_relay(struct simple_udp_connection *c,
//          const uip_ipaddr_t *sender_addr,
//          uint16_t sender_port,
//          const uip_ipaddr_t *receiver_addr,
//          uint16_t receiver_port,
//          const uint8_t *data,
//          uint16_t datalen)
// {
//   memset(mobile_ip_addr, '\0', 21 * sizeof(char));
//   uiplib_ipaddr_snprint(mobile_ip_addr, 21, sender_addr);

//   if(LL_del_srch_node(0, &LL_head, (const char *) mobile_ip_addr)) {
//     int16_t packet_rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
//     if(packet_rssi<-91 && handoff_flag==0){
//       printf("HANDOFF STARTED\n");
//       handoff_flag=1;
//     }
//     simple_udp_sendto(&up_dat_an_conn, data, datalen, &dest_ipaddr);
//     printf("relaying\n");
//   }
//   else{
//     printf("NOT relaying\n");
//   }
//   return;
// }
/*---------------------------Contiki Process---------------------------------*/
PROCESS(sdmob_anchor_node_process, "SD-MOB anchor node process");
AUTOSTART_PROCESSES(&sdmob_anchor_node_process);

PROCESS_THREAD(sdmob_anchor_node_process, ev, data)
{
  char x[20];
  static int tx_count=0;
  static struct etimer periodic_timer;

  PROCESS_BEGIN();

  NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, RADIO_TXPOWER_TXPOWER_Neg4dBm);

  simple_udp_register(&server_udp, SERVER_PT, NULL,
                      SERVER_PT, server_callback);

  simple_udp_register(&mn_udp, MN_PT, NULL,
                      MN_PT, mn_callback);

  etimer_set(&periodic_timer, 100*CLOCK_SECOND);

    while(1) {
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
      // LOG_INFO("Sending:\n",x);

      if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {

        sprintf(x,"KA %d ",tx_count);
        simple_udp_sendto(&server_udp, x, strlen(x), &dest_ipaddr);
        tx_count++;
        LOG_INFO("Done Sending: %s\n",x);
      } else {
        LOG_INFO("Not reachable yet\n");
      }
      /* Add some jitter */
      etimer_set(&periodic_timer, CLOCK_SECOND*100);
  }

  PROCESS_END();
}