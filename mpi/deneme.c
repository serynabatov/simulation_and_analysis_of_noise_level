#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <arpa/inet.h>

static char* regionName = "Lombardy";
static char* countryName = "Italy";

void sleep_ms(double ms){
    int t = ms*1000;
    usleep(t);
}

// THIS FUNCTION WILL READ FROM A FILE
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

void prepareJSon(int rank, double avg, double sws[][6], int count, double threshold, char* jstring){
    
    
    char str[160];
    
    strcpy(jstring, "{\n\"sensor\": ");
    
    sprintf(str, "S%d,\n", rank);
    strcat(jstring, str);

    //strcpy(str, "";)
    
    if(avg < threshold){
        strcat(jstring, "\"type\": 0,\n");

        sprintf(str, "\"value\": %.4f,\n", avg);
        strcat(jstring, str);
    } else{
        strcat(jstring, "\"type\": 1,\n");

        sprintf(str, "\"value\": [");
        strcat(jstring, str);

        for(int i = 0; i < count-1; i++){
            sprintf(str, "%.4f, ", sws[rank][i]);
            strcat(jstring, str);
        }
        sprintf(str, "%.4f],\n", sws[rank][count-1]);
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

    sprintf(str, "\"timestamp\": %ld,\n", totalTime);
    strcat(jstring, str);

    sprintf(str, "\"region\": %s,\n", regionName);
    strcat(jstring, str);

    sprintf(str, "\"country\": %s\n}", countryName);
    strcat(jstring, str);
    
    //printf("%s\n", jstring);
    
    //return jstring;
}

void preparePostRequest(char* message, char* jstring, char* targetIP){
    char str[160];

    sprintf(message, "POST / HTTP/1.1\n");
    strcat(message, "From: MC1\n");
    strcat(message, "Content-Type: application/json\n");
    sprintf(str, "Content-Length: %ld\n\n", strlen(jstring));
    strcat(message, str);
    strcat(message, jstring);

    printf("%s\n", message);

}

/*
struct SensorData
{
    char[6] sensorName;
    int type;
    double avg;
    double[6] values;
    int timestamp;
    char[10] region;
    char[10] country;
};
*/

int main(int argc, char *argv[]) {
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    srand(time(NULL) + rank);

    

    if(rank == 0){
        
        int socket_desc;
        struct sockaddr_in server;
        char* targetIP = "0.0.0.0";
        int port = 8080;

        socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	    if (socket_desc == -1){
		    printf("Could not create socket");
	    }
        
        server.sin_addr.s_addr = inet_addr(targetIP);
        server.sin_family = AF_INET;
        server.sin_port = htons(port);
       
        if(connect(socket_desc, (struct sockaddr *)&server ,sizeof(server)) < 0){
            printf("connect error\n");
		    return 1;
        }
        //printf("Made the connection\n");
    

        
        double val;
        double sws[size][6];
        int counts[size];
        int threshold = 15;

          
        init_arrays(sws, counts, size);

        double avg;
        MPI_Status status;
       

        char message[2000];
        char server_reply[2000];
        char jstring[500];
        while (1)
        {
            
          
            
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        
            int src = status.MPI_SOURCE;

         

            MPI_Recv(&val, 1, MPI_DOUBLE, src, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            //printf("Value of slave %d: %.2f\n\n", src, data);

         

            avg = get_new_avg(sws, counts, rank, val);
	    
		

            prepareJSon(rank, avg, sws, counts[rank], threshold, jstring);
            //strcpy(message, "LOL");

            preparePostRequest(message, jstring, targetIP);
            
            if(send(socket_desc, message, strlen(message), 0) < 0){
                printf("Send failed\n");
                return 2;
            }
            printf("Data sent\n");
		

            /*
            if(recv(socket_desc, server_reply, 2000, 0) < 0){
                printf("Receive failed\n");
                return 3;
            }
            printf("Reply received: %s", server_reply);
            */
            /*
            printf("Sliding window of slave %d: ", rank);
            for(int i = 0; i < counts[rank]; i++){
                printf("%.2f, ", sws[rank][i]);
            }            
            printf("\n");
            */

        } 
        
    } else{
        double lastVal;
        double minVal = 5;
        double maxVal = 25;    
        double afterSend = 0.0;
        double beforeSend = 0.0;
        int first = 1; 
        
        while (1)
        {   
            
            lastVal = generate_data(minVal, maxVal);
            
            /*
            printf("Sliding window: ");
            for(int i = 0; i < count; i++){
                printf("%.2f, ", sw[i]);
            }
            printf("\n");
            */
            //printf("Average of slave %d: %.2f\n", rank, avg);
            if(first){
                beforeSend = MPI_Wtime();
                int timePassed_ms = (beforeSend - afterSend)*1000;
                int waitDuration_ms = 2000 - timePassed_ms;
                if(waitDuration_ms > 0){
                    sleep_ms(waitDuration_ms);
                }
                
            } else{
                first = 0;
            }

            //printf("New generated data by slave %d: %.2f\n", rank, lastVal);
            MPI_Send(&lastVal, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        
            afterSend = MPI_Wtime();           
        }        
        

    }


    

    MPI_Finalize();
    return 0;
}


