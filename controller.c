#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"

#include "transmission.h"
#include <stdlib.h>

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

typedef struct {
	uip_ipaddr_t* addr;
	double[6] noise_levels;
	noise_struct* next;
} noise_struct;

static struct simple_udp_connection udp_conn;
noise_struct* db_linked_list_head;

static noise_struct* getEntry(uip_ipaddr_t* addr) {
	noise_struct* res;
	
	if (db_linked_list_head) {
		noise_struct* current = db_linked_list_head;
		while (current) {
			if (uip_ip6addr_cmp(addr, current->addr)) {
				return current;
			}
			//If we still have a chance to find it, find it, otherwise add element at the end
			if (current->next) {
				current = current->next;
			} else {
				res = (noise_struct*) malloc(sizeof(noise_struct));
				res->addr = addr;
				res->next = NULL;
				current->next = res;
				return res;
			}
		}
	} else {
		res = (noise_struct*) malloc(sizeof(noise_struct));
		res->addr = addr;
		res->next = NULL;
		db_linked_list_head = res;
	}
	return res;
}

static double appendNoiseLevel(noise_struct* current, double noise_level) {
	for (int i = 0; i<5; ++i) {
		(current->noise_levels)[i+1] = (current->noise_levels)[i];
	}
	(current->noise_levels)[0] = noise_level;
}

static double averageCalc(noise_struct* current, double noise_level) {
	double sum = 0;
	unsigned n = 0;
	for (int i = 0; i<6; ++i) {
		if ((current->noise_levels)[i] != -1) {	
			sum += current->noise_levels)[i];
			++n;
		}
	}
	return sum/n;
}

PROCESS(controller_process, "Controller");
AUTOSTART_PROCESSES(&controller_process);
/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  msg_t message = *(msg_t *)data;
	
  LOG_INFO("Received Message N:%u V:%lf from ", message.message_id, message.noise_level);
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");
  
  noise_struct* current = getEntry(sender_addr);
	appendNoiseLevel(current, noise_level);
  LOG_INFO("Average: %lf\n", averageCalc(current));

  //Send to NodeRed with MQTT
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(controller_process, ev, data)
{
  PROCESS_BEGIN();

  /* Initialize DAG root */
  NETSTACK_ROUTING.root_start();

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                      UDP_CLIENT_PORT, udp_rx_callback);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
