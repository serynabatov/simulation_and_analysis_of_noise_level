#include <mpi.h>
#include <stdio.h>
#include <time.h>

void sleep_ms(double ms){
    int t = ms*1000;
    usleep(t);
}

// THIS FUNCTION WILL READ FROM A FILE
double get_new_avg(double* sw, int* count){
    double avg;
    int r = rand() % 20;
    if(*count < 6){
        sw[*count] = (double)r;
        *count += 1;
        double rs = 0;
        for(int i = 0; i < *count; i++){
            rs += sw[i];
        }
        
        avg = rs / *count;
    } else{
        for(int i = 0; i < 6-1; i++){
            sw[i] = sw[i+1];
        }
        sw[5] = (double)r;
        double rs = 0;
        for(int i = 0; i < *count; i++){
            rs += sw[i];
        }
        
        avg = rs / 6.0;
    }

    return avg;
}

int main(int argc, char *argv[]) {
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    srand(time(NULL) + rank);

    

    if(rank == 0){
        //printf("Hello, this is microcontroller!\n");
        double b;
        double sw[6];
        MPI_Status status;
        while (1)
        {
            
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        
            int src = status.MPI_SOURCE;
            int tag = status.MPI_TAG;
            if(tag){
                
                int size;
                MPI_Get_count(&status, MPI_DOUBLE, &size);
                MPI_Recv(sw, size, MPI_DOUBLE, src, tag, MPI_COMM_WORLD, &status);
                printf("The threshold exceeded by slave %d. Values: ", src);
                for(int i = 0; i < size; i++){
                    printf("%.2f, ", sw[i]);
                }
                printf("\n");
            } else{
                MPI_Recv(&b, 1, MPI_DOUBLE, src, tag, MPI_COMM_WORLD, &status);
                printf("Average of slave %d: %.2f\n", src, b);
            }
            
            
        } 
        
    } else{
        
        double sw[6];
        double avg = 0;
        
        int count = 0;

        

        //printf("%d", count);
        double afterSend = 0.0;
        double beforeSend = 0.0; 
        
        while (1)
        {   
            
            avg = get_new_avg(sw, &count);
            
            /*
            printf("Sliding window: ");
            for(int i = 0; i < count; i++){
                printf("%.2f, ", sw[i]);
            }
            printf("\n");
            */
            //printf("Average of slave %d: %.2f\n", rank, avg);
            if(count > 1){
                beforeSend = MPI_Wtime();
                int timePassed_ms = (beforeSend - afterSend)*1000;
                int waitDuration_ms = 2000 - timePassed_ms;
                if(waitDuration_ms > 0){
                    sleep_ms(waitDuration_ms);
                }
            }

            
            if(avg < 11.0){
                MPI_Send(&avg, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
            } else{
                MPI_Send(sw, count, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
            }
            afterSend = MPI_Wtime();           
        }        
        

    }


    

    MPI_Finalize();
    return 0;
}


