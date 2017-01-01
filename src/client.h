//
// Created by Zihao on 2016/12/28.
//

#ifndef SIMPLEWEBSERVER_CLIENT_H
#define SIMPLEWEBSERVER_CLIENT_H

/*
 * Send an HTTP request for the specified file
 */
void clientSend(int fd, char *filename);

/*
 * Read the HTTP response and print it out
 */
void clientPrint(int fd);

/**
 * Client thread for sending request and receiving response
 */
void *client_thread(char **filename);

/**
 * Parse the arguments
 * @param host the name of the host that the web server is running on; the basic web client already handles this argument.
 * @param port the port number that the web server is listening on and that the client should send to; the basic web client already handles this argument.
 * @param threads threads: the number of threads that should be created within the web client. Must be a positive integer.
 * @param filename1 filename1: the name of the file that the client is requesting from the server.
 * @param filename2 filename2: the name of a second file that the client is requesting from the server. This argument
 * is optional. If it does not exist, then the client should repeatedly ask for only the first file. If it does exist, then each thread of the client should alternate which file it is requesting.
 * @param argc arg number
 * @param argv args
 */
void getargs(char **host, int *port, int *threads, char *filename1, char *filename2, int argc, char *argv[]);
#endif //SIMPLEWEBSERVER_CLIENT_H
