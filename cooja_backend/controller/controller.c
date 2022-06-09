#include "contiki.h"

#include "sys/log.h"
#define LOG_MODULE "Controller"
#define LOG_LEVEL LOG_LEVEL_INFO

/* Declare and auto-start the Controller process */
PROCESS(contiki_ng_br, "Controller");
AUTOSTART_PROCESSES(&contiki_ng_br);


PROCESS_THREAD(contiki_ng_br, ev, data)
{
  PROCESS_BEGIN();

  LOG_INFO("Controller started\n");

  PROCESS_END();
}
