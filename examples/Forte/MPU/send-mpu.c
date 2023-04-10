#include "DT.c"
#include "contiki.h"
#include "net/routing/routing.h"
#include "net/routing/rpl-lite/rpl.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include <stdint.h>
#include <inttypes.h>

#include "sys/log.h"

#include "sys/etimer.h"
#include "sys/ctimer.h"
#include "dev/leds.h"
#include "dev/watchdog.h"
#include "dev/button-hal.h"
#include "random.h"
#include "button-sensor.h"
#include "batmon-sensor.h"
#include "board-peripherals.h"

#include <stdio.h>
#include <stdint.h>


#define DEBUG DEBUG_PRINT
#include "net/ipv6/uip-debug.h"

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define ANCHOR_PORT	5678
#define SERVER_PORT	4567

// #undef FORTE
// #include "arch/platform/cc26x0-cc13x0/sensortag/mpu-9250-sensor.c"
/*---------------------------------------------------------------------------*/
#define CC26XX_DEMO_LOOP_INTERVAL       (CLOCK_SECOND / 64)
/*---------------------------------------------------------------------------*/
static struct simple_udp_connection server_conn;



/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/

#ifdef FORTE
static struct simple_udp_connection anchor_conn;
static void init_mpu_reading(void *not_used);
/*---------------------------------------------------------------------------*/
static int
get_mpu_reading(char * str)
{
  int value;
  int ax, ay, az, gx, gy, gz;
  // int16_t magData[3];
  char tmp[26];
  // int magStatus;
  strcpy(str,"");
  // clock_time_t next = CC26XX_DEMO_LOOP_INTERVAL;

  value = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_GYRO_X); if (value < 0) value = -value;
  sprintf(tmp,"%d.%02d ", value / 100, value % 100);
  strcat(str,tmp);
  ax = value;

  value = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_GYRO_Y); if (value < 0) value = -value;
  sprintf(tmp,"%d.%02d ", value / 100, value % 100);
  strcat(str,tmp);
  ay = value;

  value = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_GYRO_Z); if (value < 0) value = -value;
  sprintf(tmp,"%d.%02d ", value / 100, value % 100);
  strcat(str,tmp);
  az = value;
  
  value = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_X); if (value < 0) value = -value;
  sprintf(tmp,"%d.%02d ", value / 100, value % 100);
  strcat(str,tmp);
  gx = value;
  
  value = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_Y); if (value < 0) value = -value;
  sprintf(tmp,"%d.%02d ", value / 100, value % 100);
  strcat(str,tmp);
  gy = value;
  
  value = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_Z); if (value < 0) value = -value;
  sprintf(tmp,"%d.%02d ", value / 100, value % 100);
  strcat(str,tmp);
  gz = value;

  // magStatus=SensorMpu9250_magRead(magData);
  // printf("MPU MAG: X=%d,%d,%d\n",magData[0],magData[1],magData[2]);
  // printf("MPU MAG Status: X=%d\n",magStatus);
  // printf("---\n");

  // sprintf(tmp," %d %d %d", magData[0],magData[1],magData[2]);
  // strcat(str,tmp);


  // SENSORS_DEACTIVATE(mpu_9250_sensor);

//  ctimer_set(&mpu_timer, next, init_mpu_reading, NULL);
  int cls = DT(ax,ay,az,gx,gy,gz);
  // printf("%d %d %d %d %d %d %d",ax, ay,az, gx,gy,gz, cls);
  return cls;
}
/*---------------------------------------------------------------------------*/
static void
init_mpu_reading(void *not_used)
{
  mpu_9250_sensor.configure(SENSORS_ACTIVE, MPU_9250_SENSOR_TYPE_ALL);
}

/*---------------------------------------------------------------------------*/
static void
init_sensor_readings(void)
{
  init_mpu_reading(NULL);
}
/*---------------------------------------------------------------------------*/
#endif

static void
downward_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  LOG_INFO("Received downward\n");

}


PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer periodic_timer;
  static char str[62];
  static uip_ipaddr_t dest_ipaddr;
  static int tx_count;
  
  #ifdef FORTE
  static int state;
  static int MPU;
  static int mv_cnt=0;
  static int st_cnt=0;
  #endif

  PROCESS_BEGIN();
  
  #ifdef FORTE
  tx_count=1;
  state=0;
  init_sensor_readings();
  rpl_set_leaf_only(1);
  #endif

  /* Initialize UDP connection */
  simple_udp_register(&server_conn, SERVER_PORT, NULL,
                      SERVER_PORT, downward_callback);
  #ifdef FORTE
  simple_udp_register(&anchor_conn, ANCHOR_PORT, NULL,
                      ANCHOR_PORT, downward_callback);
  #endif

  etimer_set(&periodic_timer, 500*CC26XX_DEMO_LOOP_INTERVAL);


  while(1) {
    #ifdef FORTE    
    MPU = get_mpu_reading(str);
    if(state==0){
        if (MPU == 3 || MPU == 2){
          state = 1;
          mv_cnt=0;
        }
    }
    if(state == 1){
      if (MPU == 3 || MPU == 2){
          mv_cnt++;
        }
      if(mv_cnt >= 2){
        state = 2;
        st_cnt=0;
      }
        
    }
    if(state==2){
      if (MPU == 0 || MPU == 1){
        st_cnt++;
      } 
      else
        st_cnt=0;
      if(st_cnt >= 3)
        state=3;
    }
    if(state == 3){
      if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)){
        state=0;
      }
    }
    #endif

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
  
    
    if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
      #ifdef FORTE
      if(state == 0 || state == 1){
      // if(tx_count%2 == 1){ 
      #endif
        sprintf(str,"Unicast %d",tx_count);
        simple_udp_sendto(&server_conn, str, strlen(str), &dest_ipaddr);
      #ifdef FORTE
      }
      else{
        uip_create_linklocal_allnodes_mcast(&dest_ipaddr);
        sprintf(str,"Broadcast %d",tx_count);
        simple_udp_sendto(&anchor_conn, str, strlen(str), &dest_ipaddr);
      }
      #endif
      tx_count++;
      LOG_INFO("Done Sending: %s\n",str);
    } else {
      LOG_INFO("Not reachable yet\n");
    }

    /* Add some jitter */
    etimer_set(&periodic_timer, CLOCK_SECOND*2);
}

  PROCESS_END();
}
