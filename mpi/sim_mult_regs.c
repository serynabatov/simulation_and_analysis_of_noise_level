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
            
        } else{ // sensors
            int sensor_id;
            MPI_Status status;
            MPI_Probe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            int belonged_mc = status.MPI_SOURCE;

            MPI_Recv(&sensor_id, 1, MPI_INT, belonged_mc, 0, MPI_COMM_WORLD, &status);

            printf("This is sensor %d for mc %d with rank %d\n", sensor_id, belonged_mc, rank);

        }

    }


    //printf("%d\n", rank);

    MPI_Finalize();

    return 0;
}