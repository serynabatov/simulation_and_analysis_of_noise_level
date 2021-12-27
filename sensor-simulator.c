#include "contiki.h"
#include <stdio.h>
#include <stdlib.h>

/* Generate a random floating point number from min to max */
double randfrom(double min, double max) 
{
    double range = (max - min); 
    double div = RAND_MAX / range;
    return min + (rand() / div);
}

PROCESS(sensor_simulator_process, "Sensor simulator process");
AUTOSTART_PROCESSES(&sensor_simulator_process);
PROCESS_THREAD(sensor_simulator_process, ev, data) {
  static struct etimer timer;

  PROCESS_BEGIN();

  /* Setup a periodic timer that expires after 2 seconds. */
  etimer_set(&timer, CLOCK_SECOND * 2);

  while(1) {
    double d = randfrom(20,60);
    printf("Read value from the sensor: %lf\n", d);

    /* Wait for the periodic timer to expire and then restart the timer. */
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    etimer_reset(&timer);
  }

  PROCESS_END();
}

