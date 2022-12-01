#include "contiki.h"
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

// #include "arch/platform/cc26x0-cc13x0/sensortag/mpu-9250-sensor.c"
/*---------------------------------------------------------------------------*/
#define CC26XX_DEMO_LOOP_INTERVAL       (CLOCK_SECOND * 20)
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
PROCESS(base_demo_process, "CC13xx/CC26xx base demo process");
AUTOSTART_PROCESSES(&base_demo_process);

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

PROCESS_THREAD(base_demo_process, ev, data)
{

  PROCESS_BEGIN();

  printf("CC13xx/CC26xx base demo\n");

  // init_sensors();

  
  init_sensor_readings();
  // sensorMagInit();
  get_mpu_reading();
  etimer_set(&et, CC26XX_DEMO_LOOP_INTERVAL);
  while(1) {
    
    // PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    printf("timer: %d\n", etimer_expired(&et) );
    // while (etimer_expired(&et)){
    //   printf("waiting\n");
    // }

    // get_mpu_reading();
    // etimer_set(&et, CC26XX_DEMO_LOOP_INTERVAL);

  }

  PROCESS_END();
}

