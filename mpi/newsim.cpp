# include <cstdlib>
# include <ctime>
# include <iomanip>
# include <iostream>
# include <mpi.h>
#include "cJSON.h"
#include "cJSON.c"
#include <string.h>
#include <vector>
#include <cmath>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cstring>
#include <librdkafka/rdkafkacpp.h>

static volatile sig_atomic_t run = 1;

static void sigterm(int sig) {
  run = 0;
}

class ExampleDeliveryReportCb : public RdKafka::DeliveryReportCb {
 public:
  void dr_cb(RdKafka::Message &message) {
    /* If message.err() is non-zero the message delivery failed permanently
     * for the message. */
    if (message.err())
      std::cerr << "% Message delivery failed: " << message.errstr()
                << std::endl;
    else
      std::cerr << "% Message delivered to topic " << message.topic_name()
                << " [" << message.partition() << "] at offset "
                << message.offset() << std::endl;
  }
};

using namespace std;

double clamp(double val, double min, double max){
  if(val < min) return min;
  else if (val > max) return max;
  else return val;
}

void sleep_ms(double ms){
    int t = ms*1000;
    usleep(t);
}

void timestamp ();

double fRand(double fMin, double fMax)
{
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

int main ( int argc, char *argv[] ){
  int rank, size;

  MPI_Init(&argc, &argv);
  MPI_Comm_size ( MPI_COMM_WORLD, &size);
  MPI_Comm_rank ( MPI_COMM_WORLD, &rank);
  srand(time(NULL) + rank);

  if(rank == 0){ // The main node that takes data from other nodes and sends it to kafka


    // Set up parameters for kafka and create kafka producer
    string brokers = "localhost:9092";
    string topic = "sensor-readings";
    
    RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    
    string errstr;
    if (conf->set("bootstrap.servers", brokers, errstr) !=
      RdKafka::Conf::CONF_OK) {
    std::cerr << errstr << std::endl;
    exit(1);
    }

    signal(SIGINT, sigterm);
    signal(SIGTERM, sigterm);
    ExampleDeliveryReportCb ex_dr_cb;

    if (conf->set("dr_cb", &ex_dr_cb, errstr) != RdKafka::Conf::CONF_OK) {
      std::cerr << errstr << std::endl;
      exit(1);
    }

    RdKafka::Producer *producer = RdKafka::Producer::create(conf, errstr);
    if (!producer) {
      std::cerr << "Failed to create producer: " << errstr << std::endl;
      exit(1);
    }

    

    delete conf;
    

    
    MPI_Status status;
    while(true){

      // Get the data from one of the nodes
      int message_len = 0;
       MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
       int src = status.MPI_SOURCE;
       MPI_Get_count(&status, MPI_DOUBLE, &message_len);
       double noises[message_len];
       MPI_Recv(noises, message_len, MPI_DOUBLE, src, 0, MPI_COMM_WORLD, &status);
       
      int num_sources = message_len/4;


      // send each noise measurement one by one
      for(int i = 0; i < num_sources; i++){
        cJSON *message = cJSON_CreateObject();
        

        // check if we are sending the average or a raw reading
        int value_exceeded;
        if(noises[i+num_sources] < 50){
          value_exceeded = 0;
        } else{
          value_exceeded = 1;
        }

        // get the timestamp
        struct timespec curtime;
        clock_gettime(CLOCK_REALTIME, &curtime);

        long totalTime = curtime.tv_sec*1000 + curtime.tv_nsec/1000000;
        char timestamp[40];

        sprintf(timestamp, "%ld", totalTime);

        // add key-values to json
        cJSON_AddNumberToObject(message, "x", noises[i + num_sources*2]);
        cJSON_AddNumberToObject(message, "y", noises[i + num_sources*3]);
        cJSON_AddNumberToObject(message, "value", noises[i]);
        cJSON_AddBoolToObject(message, "exceeded", value_exceeded);
        cJSON_AddStringToObject(message, "timestamp", timestamp);

        // conver json to string
        char* msgstr = cJSON_Print(message);
        /*
        printf("%s\n\n", msgstr);
        printf("Message length: %ld\n\n", strlen(msgstr));
        */
        
        // send to kafka broker
        RdKafka::ErrorCode err = producer->produce(topic, 
                        RdKafka::Topic::PARTITION_UA, 
                        RdKafka::Producer::RK_MSG_COPY, 
                        msgstr, strlen(msgstr), 
                        NULL, 0, 0, NULL, NULL);

        producer->poll(0);
      }
      

      

     
       
    }
  }

  else{

    // get parameters for the region
    FILE *f = fopen("regions.json", "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *jstring = (char*) malloc(fsize + 1);
    fread(jstring, fsize, 1, f);
    fclose(f);
    jstring[fsize] = 0;

    

    cJSON *env = cJSON_Parse(jstring);

    // set up the parameters from json
    char region_id[5];
    sprintf(region_id, "%d", rank-1);
    cJSON *region_obj = cJSON_GetObjectItemCaseSensitive(env, region_id);
    
    cJSON *region_name_obj = cJSON_GetObjectItemCaseSensitive(region_obj, "name");
    string region_name = region_name_obj->valuestring;

    cJSON *p_obj = cJSON_GetObjectItemCaseSensitive(region_obj, "p");
    int P = p_obj->valueint;

    cJSON *v_obj = cJSON_GetObjectItemCaseSensitive(region_obj, "v");
    int V = v_obj->valueint;

    cJSON *we_obj = cJSON_GetObjectItemCaseSensitive(region_obj, "we");
    int WE = we_obj->valueint;

    cJSON *le_obj = cJSON_GetObjectItemCaseSensitive(region_obj, "le");
    int LE = le_obj->valueint;

    cJSON *ws_obj = cJSON_GetObjectItemCaseSensitive(region_obj, "ws");
    int WS = ws_obj->valueint;

    cJSON *ls_obj = cJSON_GetObjectItemCaseSensitive(region_obj, "ls");
    int LS = ls_obj->valueint;

    cJSON *np_obj = cJSON_GetObjectItemCaseSensitive(region_obj, "np");
    double NP = np_obj->valuedouble;

    cJSON *nv_obj = cJSON_GetObjectItemCaseSensitive(region_obj, "nv");
    double NV = nv_obj->valuedouble;

    cJSON *dp_obj = cJSON_GetObjectItemCaseSensitive(region_obj, "dp");
    double DP = dp_obj->valuedouble;

    cJSON *dv_obj = cJSON_GetObjectItemCaseSensitive(region_obj, "dv");
    double DV = dv_obj->valuedouble;

    cJSON *vp_obj = cJSON_GetObjectItemCaseSensitive(region_obj, "vp");
    double VP = vp_obj->valuedouble;

    cJSON *vv_obj = cJSON_GetObjectItemCaseSensitive(region_obj, "vv");
    double VV = vv_obj->valuedouble;
    
    cJSON *thr_obj = cJSON_GetObjectItemCaseSensitive(region_obj, "thr");
    double threshold = thr_obj->valuedouble;

   

    
    
    // do the simulation
    // initialization
    vector<double> locations_x;
    vector<double> locations_y;

    vector<double> directions_x;
    vector<double> directions_y;
    //double threshold = 150;

    double gateway_location_x = (WS + WE) / 2.0;
    double gateway_location_y = (LS + LE) / 2.0;

    // set random locations and directions
    for(int i = 0; i < P+V; i++){
      double x = fRand(WS, WE);
      double y = fRand(LS, LE);

      locations_x.push_back(x);
      locations_y.push_back(y);

      double dx = fRand(-10, 10);
      double dy = fRand(-10, 10);

      double len = sqrt(dx*dx + dy*dy);

      dx = dx/len;
      dy = dy/len;

      directions_x.push_back(dx);
      directions_y.push_back(dy);
    }
    int num_sources = locations_x.size();
    // sliding window for noises
    double noises[num_sources][6];

    /*
    for(int i = 0; i < locations_x.size(); i++){
      for(int j = 0; j < 6; j++){
        noises[i][j] = 0;
      }
    }
    */

    double past_locations_x[num_sources][6];
    double past_locations_y[num_sources][6];


    // the list of values to be sent to the main node
    // num_sources = locations_x.size()
    // 0, num_sources -> noise value
    // num_sources, num_sources*2 -> whether it's an avg or raw reading, 0 if avg, 100 if raw reading
    // num_sources*2, num_sources*3 -> location x
    // num_sources*3, num_sources*4 -> location y
    double noises_to_send[num_sources*4];
    for(int i = 0; i < num_sources; i++){
      noises_to_send[i] = 0;
    }
    

    int num_measurements = 0;
    int sw_index = 0;
    // get noise and relocate
    while(true){
      // noise 
      for(int i = 0; i < num_sources; i++){
        noises[i][sw_index] = 0;
        past_locations_x[i][sw_index] = locations_x[i];
        past_locations_y[i][sw_index] = locations_y[i];
        for(int j = 0; j < num_sources; j++){
          double dist_x = locations_x[i] - locations_x[j];
          double dist_y = locations_y[i] - locations_y[j];
          double dist = sqrt(dist_x*dist_x + dist_y*dist_y);

          if(j < P){
            if(dist < DP){ 
              noises[i][sw_index] += NP;
            }
          } else{
            if(dist < DV){
              noises[i][sw_index] += NV;
            }
          }

          

        }
      }
      
      int old_index = sw_index;
      sw_index = (sw_index+1)%6;

      if(num_measurements < 6){
        num_measurements++;
        continue;
      }
      
      // TODO don't send averages until we reach 6 meaurements
      for(int i = 0; i < num_sources; i++){
        double sum = 0;
        double sum_loc_x = 0;
        double sum_loc_y = 0;
        for(int j = 0; j < num_measurements; j++){
          sum += noises[i][j];
          sum_loc_x += past_locations_x[i][j];
          sum_loc_y += past_locations_y[i][j];
        }
        double avg = sum / num_measurements;
        if(avg > threshold){ 
          noises_to_send[i] = noises[i][old_index];
          noises_to_send[i+num_sources] = 0;

          noises_to_send[i + num_sources*2] = past_locations_x[i][old_index];
          noises_to_send[i + num_sources*3] = past_locations_y[i][old_index];
        }
        else{ 
          noises_to_send[i] = avg;
          noises_to_send[i+num_sources] = 100;
          noises_to_send[i + num_sources*2] = sum_loc_x / num_measurements;
          noises_to_send[i + num_sources*3] = sum_loc_y / num_measurements;
        }
      }
      

      

      // send to main node
      MPI_Send(noises_to_send, num_sources*4, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
     
      // TODO can make sleep time more accurate
      sleep_ms(2000);

      // location
      for(int i = 0; i < directions_x.size(); i++){
        // set new location
        double v = i < P ? VP : VV;
        locations_x[i] = clamp(locations_x[i]+v*directions_x[i], WS, WE);
        locations_y[i] = clamp(locations_y[i]+v*directions_y[i], LS, LE);


        // set new direction
        double dx = fRand(-10, 10);
        double dy = fRand(-10, 10);

        double len = sqrt(dx*dx + dy*dy);

        dx = dx/len;
        dy = dy/len;

        directions_x[i] = dx;
        directions_y[i] = dy;
        }

     
      
    }
  }

  MPI_Finalize();

  return 0;
}



