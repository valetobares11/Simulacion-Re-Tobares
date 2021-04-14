/* External definitions for single-server queueing system. */
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include "Queue.h"
#define TIME_MAX 300.0 /* time limit */
#define NUMBER_WASHERS 1 /* NUMBER SERVERS */
#define Q_LIMIT 100  /* Limit on queue length. */
#define DIST_LAUNDRY 1 /* Distribución uniforme */
#define INACTIVE 1.0e+10 /* INF */

typedef struct server_depart {
    float arrive;
    float depart;
} server_depart;

server_depart time_next_depart[NUMBER_WASHERS];
float current_clock;
float time_next_arrive;
int next_event_type;
int busy_servers;
int served_clients;
float time_last_event;
float queue_total_time;
int commerce_total_time;
int server_usage;
void  initialize(void);
void  timing(void);
void  arrive(void);
void  depart(void);
void  report(void);
void  update_time_avg_stats(void);
float expon(float mean);
float gen_next_interarrive(int dist);
float  gen_next_depart(void);
double generate_random_number(void);
float gen_interarrive_uniforme();
void servers_depart();
void set_seed_random_number(void);
int search_min_time_server_depart();

void set_seed_random_number(void) {
  srand(time(NULL));
}

int main()  /* Main function. */
{

    /* Initialize the simulation. */

    initialize();

    /* Run the simulation while more delays are still needed. */

    while (current_clock <= TIME_MAX || size() > 0 || busy_servers == NUMBER_WASHERS) {

        /* Determine the next event. */

        timing();

        /* Update time-average statistical accumulators. */

        update_time_avg_stats();

        /* Invoke the appropriate event function. */

        switch (next_event_type) {
            case 1:
                arrive();
                break;
            case 2:
                depart();
                break;
        }
    }

    /* Invoke the report generator and end the simulation. */

    report();

    return 0;
}


void initialize(void)  /* Initialization function. */
{
    /* Initialize the simulation clock. */
    set_seed_random_number();

    current_clock = 0.0;

    /* Initialize the state variables. */
    initialization(); //inicializamos la cola
    time_last_event = 0; //timepo del siguiente evento
    busy_servers = 0; //cuenta la cantidad de servers ocupados

    // Initialize the statistical counters. num_custs_delayed, total_of_delays, area_num_in_q, area_server_status
    served_clients = 0;
    commerce_total_time = 0;
    server_usage = 0;
    queue_total_time = 0;

    /* Initialize event list.  Since no customers are present, the departure
       (service completion) event is eliminated from consideration. */

    time_next_arrive = current_clock + gen_next_interarrive(DIST_LAUNDRY); //Planificacion del primer arribo
    for (int i = 0; i < NUMBER_WASHERS; ++i)
    {
      time_next_depart[i].depart = INACTIVE; //Por defecto inician todos los servers INACTIVEs
    }
}


void timing(void)  /* Timing function. */
{
   float min_time_next_event = 1.0e+10;

   next_event_type = 0; //Al final debe quedar en 1 en caso de que el proximo evento sea un arribo, 2: si es la salida de un servicio

   /* Determine the event type of the next event to occur. */
   min_time_next_event = time_next_arrive;
   next_event_type = 1;

   for (int i = 0; i < NUMBER_WASHERS; ++i)
    {
      if (min_time_next_event > time_next_depart[i].depart) {
        min_time_next_event = time_next_depart[i].depart;
        next_event_type = 2;
      }
    } 


    /* advance the simulation clock. */

    current_clock = min_time_next_event;
}

void update_time_avg_stats(void)  /* Update area accumulators for time-average
                                     statistics. */
{
    float time_since_last_event;

    /* Compute time since last event, and update last-event-time marker. */

    time_since_last_event = current_clock - time_last_event;
    time_last_event       = current_clock;

    /* Update area under server-busy indicator function. */
    server_usage = server_usage +((time_since_last_event)*busy_servers);

}

void arrive(void)  /* Arrival event function. */
{
    /* Schedule next arrival. */
    if(current_clock <= TIME_MAX) {
      
        time_next_arrive = current_clock + gen_next_interarrive(DIST_LAUNDRY);
    }
    else {
      time_next_arrive = 1.0e+10; 
    }
    /* Check to see whether server is busy. */

    if (busy_servers == NUMBER_WASHERS) {

        /* Server is busy, so increment number of customers in queue. */

	      add(current_clock);

        /* Check to see whether an overflow condition exists. */

        if (size() > Q_LIMIT) {

            exit(2);
        }

        /* Guardar el tiempo de arribo de esta entidad para los calculos estadisticos */



    }

    else {

        /* Server libre, tener en  cuenta la entidad que pasa directamente al server para los calculos estadisticos */

        /*Llevar el numero de clientes que han entrado*/

        /* Schedule a departure (service completion). */

        //arriva el cliente a un server y pasa a estar ocupado
        int i = 0;

        while (time_next_depart[i].depart != INACTIVE){
          i++;
        }
        time_next_depart[i].arrive = current_clock;
        time_next_depart[i].depart = current_clock + gen_next_depart();
        busy_servers++;
    }
}


void depart(void)  /* Departure event function. */
{
    served_clients++;  

    /* Check to see whether the queue is empty. */
    int min = search_min_time_server_depart();
    if (isEmpty()) {

        /* The queue is empty so make the server idle and eliminate the
           departure (service completion) event from consideration. */
        commerce_total_time += time_next_depart[min].depart - time_next_depart[min].arrive;
        time_next_depart[min].depart = INACTIVE;
        busy_servers--;
    }

    else {

        /* The queue is nonempty, so decrement the number of customers in
           queue. */

        float time_cte = pop();

        /* Compute the delay of the customer who is beginning service and update
           the total delay accumulator. */
        
        /* Increment the number of customers delayed, and schedule departure. */

        time_next_depart[min].arrive = current_clock;
        time_next_depart[min].depart = current_clock + gen_next_depart();     
        commerce_total_time += (current_clock - time_cte) + (time_next_depart[min].depart - current_clock);
        queue_total_time += (current_clock - time_cte);
    }
}
int search_min_time_server_depart()
{

    float time_min = 1.0e+10;
    int min = 0;
   
    for (int i = 0; i < NUMBER_WASHERS; ++i)
    {
        if (time_min > time_next_depart[i].depart) {
            time_min = time_next_depart[i].depart;
            min = i;
        }
    }
    
   return min;
}


void report(void)  /* Report generator function. */
{

    float areaUsoServidor = (server_usage / time_last_event*NUMBER_WASHERS);
    float areaUsoCola = queue_total_time / time_last_event;

    /* Compute and write estimates of desired measures of performance. */
    printf("Tiempo promedio de un cliente en la cola: %.1lfm\n",queue_total_time / (served_clients+0.0)); //media de demora en cola

    printf("Tiempo promedio que un cliente pasa en el comercio: %.1lfm\n", commerce_total_time / (served_clients+0.0));  //media de demora en el local

    printf("Número promedio de clientes en la cola: %.2lf\n", areaUsoCola); //media de clientes en cola

    printf("Número promedio de clientes en el comercio: %.2lf\n", areaUsoCola + areaUsoServidor); //media de clientes en local

    printf("Porcentaje de utilización de %d lavadora/s: %%%.2lf\n", NUMBER_WASHERS, server_usage / (time_last_event*NUMBER_WASHERS)); //porcentaje de utilizacion de los n servidores

}


/*Generates the next arrive in base of given probabilities*/
float gen_next_interarrive(int dist)
{
  
     if(dist == 1){
        return gen_interarrive_uniforme(); 
     }

    return 0;    
}



/*Generates the next depart in base of given probabilities*/
float gen_next_depart()
{
    double randomNumber = generate_random_number();
    if (randomNumber <= 0.15) {
      return 10.0;
    }
    else {
      if (randomNumber <= 0.4) {
        return 20.0;
      }
      else {
        if (randomNumber <= 0.8) {
          return 30.0;
        }
        else {
          return 40.0;
        }
      }
    }
}

/*
* Generates a random double between 0 and 1
* (with 2 digit precision)
*/
double generate_random_number(void) {
  int generated_number = rand() % 101;
  return (double)generated_number/100;
}


float gen_interarrive_uniforme()
{
  double randomNumber = generate_random_number();
    if (randomNumber <= 0.05) {
      return 5.0;
    }
    else {
      if (randomNumber <= 0.1) {
        return 10.0;
      }
      else {
        if (randomNumber <= 0.2) {
          return 15.0;
        }
        else {
          if (randomNumber <= 0.3) {
            return 20.0;
          }
          else {
            if (randomNumber <= 0.6) {
              return 25.0;
            }
            else {
              if (randomNumber <= 0.8) {
                return 30.0;
              }
              else {
                if (randomNumber <= 0.95) {
                  return 35.0;
                }
                else {
                  return 40.0;
                }
              }
            }
          }
        }
      }
  }

}
