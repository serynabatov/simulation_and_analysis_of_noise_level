#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "cJSON.h"
#include "cJSON.c"

static char* countryName = "Italy";

void sleep_ms(double ms){
    int t = ms*1000;
    usleep(t);
}

double generate_data(double min, double max){
    double range = max - min;
    double div = RAND_MAX / range;
    return min + rand()/div;
}

void init_arrays(double sws[][6], int counts[], int rank){
    for(int i = 0; i < rank; i++){
        counts[i] = 0;
        for(int j = 0; j < 6; j++){
            sws[i][j] = 0;
        }
    }
}

double get_new_avg(double sws[][6], int counts[], int rank, double data){
    double avg; 
    if(counts[rank] < 6){
        sws[rank][counts[rank]] = data;      
        counts[rank] += 1;
        double rs = 0;
        for(int i = 0; i < counts[rank]; i++){
            rs += sws[rank][i];
        }
        avg = rs / counts[rank];
    } else{
        for(int i = 0; i < 6-1; i++){
            sws[rank][i] = sws[rank][i+1];
        }
        sws[rank][5] = data;
        double rs = 0;
        for(int i = 0; i < counts[rank]; i++){
            rs += sws[rank][i];
        }
        avg = rs / 6.0;
    }
    return avg;
}

void prepareJSon(int rank, int region_id, char* regionName, double avg, double sws[][6], int count, double threshold, char* jstring){
    
    
    char str[160];
    
    strcpy(jstring, "{\"sensor_id\": ");
    
    sprintf(str, "%d,", rank);
    strcat(jstring, str);

    //strcpy(str, "";)

    sprintf(str, "\"region_id\": %d,", region_id);
    strcat(jstring, str);

    sprintf(str, "\"region_name\": %s,", regionName);
    strcat(jstring, str);

    sprintf(str, "\"country_name\": %s}", countryName);
    strcat(jstring, str);
    
    if(avg < threshold){
        strcat(jstring, "\"type\": 0,");

        sprintf(str, "\"value\": %.4f,", avg);
        strcat(jstring, str);
    } else{
        strcat(jstring, "\"type\": 1,");

        sprintf(str, "\"value\": [");
        strcat(jstring, str);

        for(int i = 0; i < count-1; i++){
            sprintf(str, "%.4f, ", sws[rank][i]);
            strcat(jstring, str);
        }
        sprintf(str, "%.4f],", sws[rank][count-1]);
        strcat(jstring, str);
    }
    /*
    time_t curtime;
    time(&curtime);
    char* timeStr = ctime(&curtime);
    timeStr[strlen(timeStr)-1] = '\0';
    */

    struct timespec curtime;
    clock_gettime(CLOCK_REALTIME, &curtime);

    long totalTime = curtime.tv_sec*1000 + curtime.tv_nsec/1000000;

    sprintf(str, "\"timestamp\": %ld,", totalTime);
    strcat(jstring, str);

    
    //printf("%s\n", jstring);
    
    //return jstring;
}

int main(int argc, char *argv[]) {
    
    MPI_Init(&argc, &argv);
    int rank, size;
    int nodeType;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(rank == 0){ // environment setter
        FILE *f = fopen("environment.json", "rb");
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);

        char *jstring = malloc (fsize + 1);
        fread(jstring, fsize, 1, f);
        fclose(f);
        jstring[fsize] = 0;

        

        cJSON *env = cJSON_Parse(jstring);
        
        cJSON *n_regions_obj = cJSON_GetObjectItemCaseSensitive(env, "n_regions");

        int n_regions = n_regions_obj->valueint;

        int sensorRankStart = 1 + n_regions;

        cJSON *regions_obj = cJSON_GetObjectItemCaseSensitive(env, "regions");
        cJSON *region_obj = NULL;

        int regionRank = 1;
        cJSON_ArrayForEach(region_obj, regions_obj){
            cJSON *region_id_obj = cJSON_GetObjectItemCaseSensitive(region_obj, "region_id");
            cJSON *region_name_obj = cJSON_GetObjectItemCaseSensitive(region_obj, "region_name");
            cJSON *n_sensors_obj = cJSON_GetObjectItemCaseSensitive(region_obj, "n_sensors");

            int region_id = region_id_obj->valueint;
            char* region_name = region_name_obj->valuestring;
            int n_sensors = n_sensors_obj->valueint;

            MPI_Send(&region_id, 1, MPI_INT, regionRank, 0, MPI_COMM_WORLD);
            MPI_Send(region_name, strlen(region_name)+1, MPI_CHAR, regionRank, 0, MPI_COMM_WORLD);
            MPI_Send(&n_sensors, 1, MPI_INT, regionRank, 0, MPI_COMM_WORLD);
            MPI_Send(&sensorRankStart, 1, MPI_INT, regionRank, 0, MPI_COMM_WORLD);

            sensorRankStart += n_sensors;
            regionRank++;

        }
        int notRegion = -1;
        for(int i = regionRank; i < size; i++){
            MPI_Send(&notRegion, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
        cJSON_Delete(env);
    } else{ // sensors and microcontrollers
        int region_id;
        MPI_Recv(&region_id, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        if(region_id != -1){ //microcontrollers
            char* region_name;
            int n_sensors;
            int sensorRankStart;
            
            MPI_Status status;
            int str_size;
            MPI_Probe(0, 0, MPI_COMM_WORLD, &status);
            MPI_Get_count(&status, MPI_CHAR, &str_size);
            region_name = (char*)malloc(sizeof(char)*str_size);

            //printf("Region %d, str size: %d\n", region_id, str_size);
        
            MPI_Recv(region_name, str_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);

            MPI_Recv(&n_sensors, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            MPI_Recv(&sensorRankStart, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            //printf("This is region %d, named %s with %d sensors starting from rank %d\n", region_id, region_name, n_sensors, sensorRankStart);

            
            for(int i = 0; i < n_sensors; i++){
                MPI_Send(&i, 1, MPI_INT, i + sensorRankStart, 0, MPI_COMM_WORLD);
            }
           
            
            // Microcontrollers are ready

            double val;
            double sws[n_sensors][6];
            int counts[n_sensors];
            int threshold = 15;
            

            init_arrays(sws, counts, n_sensors);

            //printf("rank: %d, region id: %d, sensor rank start: %d\n", rank, region_id, sensorRankStart);

            double avg;

            
            
            while(1){
                
                char message[2000];
                char jstring[500];
                MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

                //printf("rank: %d, region id: %d, sensor rank start: %d\n", rank, region_id, sensorRankStart);
                
        
                int src = status.MPI_SOURCE;
                int src_id = src - sensorRankStart;
                //if(rank == 1) printf("%d\n", n_sensors);
                //printf("This is region %d. Message received by sensor %d.\n", region_id, src_id);
                
                MPI_Recv(&val, 1, MPI_DOUBLE, src, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                
                avg = get_new_avg(sws, counts, src_id, val);
                
                prepareJSon(src_id, region_id, region_name, avg, sws, counts[src_id], threshold, jstring);

                printf("%s\n", jstring);

            }
            
        } else{ // sensors
            int sensor_id;
            MPI_Status status;
            MPI_Probe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            int belonged_mc_rank = status.MPI_SOURCE;

            MPI_Recv(&sensor_id, 1, MPI_INT, belonged_mc_rank, 0, MPI_COMM_WORLD, &status);

            //printf("This is sensor %d for mc %d with rank %d\n", sensor_id, belonged_mc, rank);

            // Sensors are ready
            double lastVal;
            double afterSend = 0.0;
            double beforeSend = 0.0;
            double minVal = 5;
            double maxVal = 25;  
            int first = 1;
            int periodInMs = 2000;

            while(1){

                lastVal = generate_data(minVal, maxVal);

                if(!first){
                    first = 0;
                    beforeSend = MPI_Wtime();
                    int timePassed_ms = (beforeSend - afterSend)*1000;
                    int waitDuration_ms = periodInMs - timePassed_ms;
                    if(waitDuration_ms > 0){
                        sleep_ms(waitDuration_ms);
                    }
                }
                //if(rank == 23)
                //printf("%d\n", belonged_mc);
                MPI_Send(&lastVal, 1, MPI_DOUBLE, belonged_mc_rank, 0, MPI_COMM_WORLD);

                //printf("Sensor with rank %d, sensor_id %d, sending to mc with rank %d\n", rank, sensor_id, belonged_mc_rank);
                //break;

                afterSend = MPI_Wtime();
            }


        }

    }


    //printf("%d\n", rank);

    MPI_Finalize();

    return 0;
}