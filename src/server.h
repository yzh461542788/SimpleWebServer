//
// Created by Zihao on 2016/12/28.
//

#ifndef SIMPLEWEBSERVER_SERVER_H
#define SIMPLEWEBSERVER_SERVER_H

/**
 * Parse the arguments
 * @param port the port number that the web server should listen on; the basic web server already handles this argument.
 * @param threads the number of worker threads that should be created within the web server. Must be a positive integer.
 * @param buffers the number of request connections that can be accepted at one time. Must be a positive integer.
 * @param schedalg the scheduling algorithm to be performed. Must be one of ANY, FIFO, HPSC, or HPDC.
 * @param argc arg number
 * @param argv args
 */
void getargs(int *port, int *threads, int *buffers, char **schedalg, int argc, char *argv[]);

/**
 * Worker thread for FIFO policy
 * @return
 */
void fifo_worker() ;

/**
 * Worker thread for HPDC policy
 * @return
 */
void hpdc_workder() ;

/**
 * Worker thread for HPSC policy
 * @return
 */
void hpsc_workder() ;
#endif //SIMPLEWEBSERVER_SERVER_H
