#include <basic_utils.h>
#include <sys/time.h>
#include <time.h>
#include <cmath>
#include <ctime>
#include <unistd.h>
#include <stdio.h>

double clamp(double val, double min, double max){
  if(val < min) return min;
  else if (val > max) return max;
  else return val;
}

void sleep_ms(double ms){
    int t = ms*1000;
    usleep(t);
}

double fRand(double fMin, double fMax)
{
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

void get_timestamp(char* timestamp){
    struct timespec curtime;
    clock_gettime(CLOCK_REALTIME, &curtime);

    long totalTime = curtime.tv_sec*1000 + curtime.tv_nsec/1000000;

    sprintf(timestamp, "%ld", totalTime);
}


double get_distance(double x1, double x2, double y1, double y2){
    double dist_x = x1 - x2;
    double dist_y = y1 - y2;
    double dist = sqrt(dist_x*dist_x + dist_y*dist_y);
    return dist;
}
