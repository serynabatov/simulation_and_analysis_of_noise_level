#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"

#include "transmission.h"
#include <stdlib.h>

#include "sys/log.h"
#define LOG_MODULE "Node"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

static struct simple_udp_connection udp_conn;
msg_t message = (msg_t) { 0, 0};

#define START_INTERVAL		(15 * CLOCK_SECOND)
#define SEND_INTERVAL		  (10 * CLOCK_SECOND)

/* Generate a random floating point number from min to max */
double randfrom(double min, double max) 
{
    double range = (max - min); 
    double div = RAND_MAX / range;
    return min + (rand() / div);
}

//Set better value
static double noiseGen() {
	return (double)randfrom(20,60);
}

/*---------------------------------------------------------------------------*/
PROCESS(node_process, "Node");
AUTOSTART_PROCESSES(&node_process);
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
/*
	unsigned message_id = *(unsigned *)data;
  LOG_INFO("Received Message N: %u from ", message_id);
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");
*/
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(node_process, ev, data)
{
  static struct etimer periodic_timer;

  uip_ipaddr_t dest_ipaddr;

  PROCESS_BEGIN();

	LOG_INFO("Node Process\n");
	
  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);

  etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
			message.noise_level = noiseGen();
      /* Send to DAG root */
      LOG_INFO("Sending Message N:%u V:%lf to ", ++(message.message_id), message.noise_level);
      LOG_INFO_6ADDR(&dest_ipaddr);
      LOG_INFO_("\n");
      simple_udp_sendto(&udp_conn, &message, sizeof(msg_t), &dest_ipaddr);
    } else {
      LOG_INFO("Not reachable yet\n");
    }

    etimer_set(&periodic_timer, SEND_INTERVAL - CLOCK_SECOND);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
