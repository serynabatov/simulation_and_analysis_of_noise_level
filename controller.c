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
#define READINGS 6

typedef struct noise_struct {
	const uip_ipaddr_t addr;
	double noise_levels[READINGS];
	struct noise_struct* next;
} noise_struct;

static struct simple_udp_connection udp_conn;
noise_struct* db_linked_list_head;

static noise_struct* createEntry(uip_ipaddr_t addr) {
	noise_struct* res = (noise_struct*) malloc(sizeof(noise_struct));
	uip_ip6addr_copy(&(res->addr), &addr);
	res->next = NULL;
	return res;
}

static noise_struct* getEntry(uip_ipaddr_t addr) {
	noise_struct* res = NULL;
	
	if (db_linked_list_head) {
		noise_struct* current = db_linked_list_head;
		while (current) {
			if (uip_ip6addr_cmp(&addr, &(current->addr))) {
				return current;
			}
			//If we still have a chance to find it, find it, otherwise add element at the end
			if (current->next) {
				current = current->next;
			} else {
				current->next = createEntry(addr);
				return current->next;
			}
		}
	} else {
		db_linked_list_head = createEntry(addr);
		res = db_linked_list_head;
	}
	return res;
}

static void appendNoiseLevel(noise_struct* current, double noise_level) {
	for (int i = READINGS-1; i!=0; --i) {
		(current->noise_levels)[i] = (current->noise_levels)[i-1];
	}
	(current->noise_levels)[0] = noise_level;
}

static uint16_t min(uint16_t a, uint16_t b) {
	if (a > b)
		return b;
	else 
		return a;
}

static double averageCalc(noise_struct* current, uint16_t message_id) {
	double sum = 0;
	unsigned available_reads = min(message_id, READINGS);
	
	printf("Values: ");
	for (int i = 0; i<available_reads; ++i) {
		sum += (current->noise_levels)[i];
		printf("%lf ", (current->noise_levels)[i]);
	}
	printf("\n");
	return sum/available_reads;
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
  
  noise_struct* current = getEntry(*sender_addr);
	appendNoiseLevel(current, message.noise_level);
  LOG_INFO("Average: %lf\n", averageCalc(current, message.message_id));

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
