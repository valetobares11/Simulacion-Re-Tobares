/* External definitions for single-server queueing system. */
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
//#include "lcgrand.h"  /* Header file for random-number generator. */
#define TIME_MAX 300.0 /*time limit*/
#define NUMBER_SERVERS 1 /*number servers*/
#define Q_LIMIT 100  /* Limit on queue length. */
#define DIST_LAVADERO 1 
#define INACTIVO 1.0e+10 /*representa servidos inactivo*/
#include "Queue.h"

float sim_duration;
float sim_time;
float time_next_arrivo;
float time_next_depart[NUMBER_SERVERS];
//server_depart time_next_depart[NUMBER_SERVERS];
int  next_event_type;
int busy_servers;
int served_clients;
float time_last_event;
float queue_total_time;
int laundry_total_time;
int server_usage;
float mean_clients_laundry;
float mean_clients_laundry;
int clients_in_laundry ;
float  mean_clients_queue;
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

    while (sim_time <= sim_duration || size() > 0 || busy_servers == NUMBER_SERVERS) {

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

    sim_duration = TIME_MAX;
    sim_time = 0.0;
    /* Initialize the state variables. */
    initialization(); //inicializamos la cola
    time_last_event = 0; //timepo del siguiente evento
    busy_servers = 0; //cuenta la cantidad de servers ocupados

    // Initialize the statistical counters. num_custs_delayed, total_of_delays, area_num_in_q, area_server_status
    served_clients = 0;
    laundry_total_time = 0;
    server_usage = 0;
    queue_total_time = 0;
    mean_clients_laundry = 0;
    mean_clients_queue = 0;
    clients_in_laundry = 0;

    /* Initialize event list.  Since no customers are present, the departure
       (service completion) event is eliminated from consideration. */

    time_next_arrivo = sim_time + gen_next_interarrive(DIST_LAVADERO); //Planificacion del primer arribo
     for (int i = 0; i < NUMBER_SERVERS; ++i)
    {
       time_next_depart[i] = INACTIVO; //Por defecto inician todos los servers inactivos
    }
}


void timing(void)  /* Timing function. */
{
    int   i;
    float min_time_next_event = 1.0e+29;

    next_event_type = 0; //Al final debe quedar en 1 en caso de que el proximo evento sea un arribo, 2: si es la salida de un servicio

    /* Determine the event type of the next event to occur. */

    min_time_next_event = time_next_arrivo;
     next_event_type = 1;
     for (int i = 0; i < NUMBER_SERVERS; ++i)
      {
        if (min_time_next_event > time_next_depart[i]) {
          min_time_next_event = time_next_depart[i];
          next_event_type = 2;
        }
      } 


    /* advance the simulation clock. */

    sim_time = min_time_next_event;
}

void update_time_avg_stats(void)  /* Update area accumulators for time-average
                                     statistics. */
{
    float time_since_last_event;

    /* Compute time since last event, and update last-event-time marker. */

    time_since_last_event = sim_time - time_last_event;
    time_last_event       = sim_time;

    /* Update area under number-in-queue function. */
    mean_clients_queue = mean_clients_queue + ((time_since_last_event)*size());
    mean_clients_laundry = mean_clients_laundry + ((time_since_last_event)*clients_in_laundry);


    /* Update area under server-busy indicator function. */
    server_usage = server_usage +((time_since_last_event)*busy_servers);

}

void arrive(void)  /* Arrival event function. */
{
    clients_in_laundry++;

    /* Schedule next arrival. */
    if(sim_time <= sim_duration) {
      
        time_next_arrivo = sim_time + gen_next_interarrive(DIST_LAVADERO);
    }
    else {
      time_next_arrivo = 1.0e+10; 
    }
    /* Check to see whether server is busy. */

    if (busy_servers == NUMBER_SERVERS) {

        /* Server is busy, so increment number of customers in queue. */

	      add(sim_time);

        /* Check to see whether an overflow condition exists. */

        if (size() > Q_LIMIT) {

            exit(2);
        }

        /* Guardar el tiempo de arribo de esta entidad para los calculos estadisticos */



    }

    else {

        /* Server libre, tener en  cuenta la entidad que pasa directamente al server para los calculos estadisticos */

        /*Llevar el numero de clientes que han entrado*/

        served_clients++;	
        /* Schedule a departure (service completion). */

        //arriva el cliente a un server y pasa a estar ocupado
        int i =0;
        while (time_next_depart[i] != INACTIVO){
          i++;
        }
        time_next_depart[i] = sim_time + gen_next_depart();
        busy_servers++;
    }
}


void depart(void)  /* Departure event function. */
{
     
    served_clients++;
    clients_in_laundry--;

    /* Check to see whether the queue is empty. */
    int min = search_min_time_server_depart();
    if (isEmpty()) {

        /* The queue is empty so make the server idle and eliminate the
           departure (service completion) event from consideration. */
        time_next_depart[min] = INACTIVO;
        busy_servers--;
    }

    else {

        /* The queue is nonempty, so decrement the number of customers in
           queue. */

        float time_cte = pop();

        /* Compute the delay of the customer who is beginning service and update
           the total delay accumulator. */
        

        /* Increment the number of customers delayed, and schedule departure. */

        time_next_depart[min] = sim_time + gen_next_depart();
        laundry_total_time += (sim_time - time_cte) + (time_next_depart[min]-sim_time);
        queue_total_time += (sim_time - time_cte);

    }
}
int search_min_time_server_depart()
{

    float time_min= 1.0e+10;
    int min=0;
   
    for (int i = 0; i < NUMBER_SERVERS; ++i)
    {
        if (time_min > time_next_depart[i]) {
            time_min = time_next_depart[i];
            min = i;
        }
    }
    
   return min;
}


void report(void)  /* Report generator function. */
{
    /* Compute and write estimates of desired measures of performance. */
/* Compute and write estimates of desired measures of performance. */
    printf("Tiempo promedio de un cliente en la cola.: %f\n",queue_total_time / (served_clients+0.0)); //media de demora en cola //

    printf("Tiempo promedio que un cliente pasa en el comercio: %f\n", laundry_total_time / (served_clients+0.0)); //media de demora en el local //

    printf("Número promedio de clientes en la cola: %f\n",mean_clients_queue / time_last_event ); //media de clientes en cola //

    printf("Número promedio de clientes en el comercio: %f\n",mean_clients_laundry / time_last_event ); //media de clientes en local //.

    printf("Utilización de la máquina lavadora de autos: %f\n",server_usage / (time_last_event*NUMBER_SERVERS)); //.


}


/*Generates the next arrivo in base of given probabilities*/
float gen_next_interarrive(int dist)
{
  
     if(dist == 1){
        return gen_interarrive_uniforme(); 
     }
     /* if(dist == 2){
        return exponencial();
     }
      if(dist == 3){
        return normal();
     }*/
    return 0;    
}



/*Generates the next depart in base of given probabilities*/
float  gen_next_depart()
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
