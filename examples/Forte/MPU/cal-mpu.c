#include "contiki.h"
#include "net/routing/routing.h"
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
#include "target-conf.h"

#include <stdio.h>
#include <stdint.h>

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

// #include "arch/platform/cc26x0-cc13x0/sensortag/mpu-9250-sensor.c"
/*---------------------------------------------------------------------------*/
#define CC26XX_DEMO_LOOP_INTERVAL       (CLOCK_SECOND * 1)
/*---------------------------------------------------------------------------*/
static struct simple_udp_connection udp_conn;
static uint32_t rx_count = 0;

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/


static void init_mpu_reading(void *not_used);
/*---------------------------------------------------------------------------*/
static void
print_mpu_reading(int reading)
{
  if(reading < 0) {
    printf("-");
    reading = -reading;
  }

  printf("%d.%02d", reading / 100, reading % 100);
}
/*---------------------------------------------------------------------------*/
// static struct ctimer mpu_timer;
/*---------------------------------------------------------------------------*/
static void
get_mpu_reading()
{
  int value;
  int16_t magData[3];
  int magStatus;
  // clock_time_t next = CC26XX_DEMO_LOOP_INTERVAL;

  printf("MPU Gyro: X=");
  value = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_GYRO_X);
  print_mpu_reading(value);
  printf(" deg/sec\n");

  printf("MPU Gyro: Y=");
  value = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_GYRO_Y);
  print_mpu_reading(value);
  printf(" deg/sec\n");

  printf("MPU Gyro: Z=");
  value = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_GYRO_Z);
  print_mpu_reading(value);
  printf(" deg/sec\n");

  printf("MPU Acc: X=");
  value = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_X);
  print_mpu_reading(value);
  printf(" G\n");

  printf("MPU Acc: Y=");
  value = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_Y);
  print_mpu_reading(value);
  printf(" G\n");

  printf("MPU Acc: Z=");
  value = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_Z);
  print_mpu_reading(value);
  printf(" G\n");


  magStatus=SensorMpu9250_magRead(magData);
  printf("MPU MAG: X=%d,%d,%d\n",magData[0],magData[1],magData[2]);
  printf("MPU MAG Status: X=%d\n",magStatus);
  printf("---\n");

  // SENSORS_DEACTIVATE(mpu_9250_sensor);

//  ctimer_set(&mpu_timer, next, init_mpu_reading, NULL);
}
/*---------------------------------------------------------------------------*/
static void
init_mpu_reading(void *not_used)
{
  mpu_9250_sensor.configure(SENSORS_ACTIVE, MPU_9250_SENSOR_TYPE_ALL);
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
// static void
// init_sensors(void)
// {
//   SENSORS_ACTIVATE(batmon_sensor);
// }
/*---------------------------------------------------------------------------*/
static void
init_sensor_readings(void)
{
  init_mpu_reading(NULL);
}
/*---------------------------------------------------------------------------*/


PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer periodic_timer;
  static char str[32];
  uip_ipaddr_t dest_ipaddr;
  static uint32_t tx_count;
  static uint32_t missed_tx_count;

  PROCESS_BEGIN();

  init_sensor_readings();
  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, NULL);

  etimer_set(&periodic_timer, random_rand() % CC26XX_DEMO_LOOP_INTERVAL);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    get_mpu_reading();

    if(NETSTACK_ROUTING.node_is_reachable() &&
        NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {

      // /* Print statistics every 10th TX */
      // if(tx_count % 10 == 0) {
      //   LOG_INFO("Tx/Rx/MissedTx: %" PRIu32 "/%" PRIu32 "/%" PRIu32 "\n",
      //            tx_count, rx_count, missed_tx_count);
      // }

      // /* Send to DAG root */
      // LOG_INFO("Sending request %"PRIu32" to ", tx_count);
      // LOG_INFO_6ADDR(&dest_ipaddr);
      // LOG_INFO_("\n");
      snprintf(str, sizeof(str), "hello %" PRIu32 "", tx_count);
      simple_udp_sendto(&udp_conn, str, strlen(str), &dest_ipaddr);
      tx_count++;
    } else {
      LOG_INFO("Not reachable yet\n");
      if(tx_count > 0) {
        missed_tx_count++;
      }
    }

    /* Add some jitter */
    etimer_set(&periodic_timer, CC26XX_DEMO_LOOP_INTERVAL);
  }

  PROCESS_END();
}