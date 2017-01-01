/**
 * client.c: A multi-thread HTTP client.
 * 
 * Sends one HTTP request to the specified HTTP server.
 * Prints out the HTTP response.
 *
 */

#include "cse.h"
#include "client.h"
#ifdef __APPLE__
#include "pthread_barrier.h"
#endif

static char *host;
static int port;
static pthread_barrier_t barrier;

int main(int argc, char *argv[]) {
    int threads;
    char filename1[MAXLINE], filename2[MAXLINE];
    char *filename[2];
    filename[0] = filename1;
    filename[1] = filename2;

    getargs(&host, &port, &threads, filename1, filename2, argc, argv);

    pthread_t tid[threads];
    pthread_barrier_init(&barrier, NULL, threads);

    /* Open several connections with different threads to the specified host and port */
    int i;
    for (i = 0; i < threads; ++i) {
        pthread_create(&tid[i], NULL, &client_thread, filename);
    }
    pthread_join(tid[0], NULL);
    return 0;
}

void *client_thread(char **filename) {
    // keep request until client is killed
    while (1) {
        int clientfd = Open_clientfd(host, port);
        // request for the first filename
        clientSend(clientfd, filename[0]);
        clientPrint(clientfd);
        Close(clientfd);
        // sleep a while to avoid crashing server
//        sleep(3);
        // wait for other process
        pthread_barrier_wait(&barrier);

        // request for the second filename if exist
        if (filename[2]) {
            clientfd = Open_clientfd(host, port);
            clientSend(clientfd, filename[1]);
            clientPrint(clientfd);
            Close(clientfd);
            // sleep a while to avoid crashing server
//            sleep(3);
            // wait for other process
            pthread_barrier_wait(&barrier);
        }
    }
}

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
void getargs(char **host, int *port, int *threads, char *filename1, char *filename2, int argc, char *argv[]) {
    if (argc == 5) {
        *host = argv[1];
        *port = atoi(argv[2]);
        *threads = atoi(argv[3]);

        // add dash if there is not
        if (argv[4][0] == '/') {
            strcpy(filename1, argv[4]);
        } else {
            filename1[0] = '/';
            strcpy(filename1 + 1, argv[4]);
        }
    } else if (argc == 6) {
        *host = argv[1];
        *port = atoi(argv[2]);
        *threads = atoi(argv[3]);
        // add dash if there is not
        if (argv[4][0] == '/') {
            strcpy(filename1, argv[4]);
        } else {
            filename1[0] = '/';
            strcpy(filename1 + 1, argv[4]);
        }
        if (argv[5][0] == '/') {
            strcpy(filename2, argv[5]);
        } else {
            filename2[0] = '/';
            strcpy(filename2 + 1, argv[5]);
        }
    } else {
        fprintf(stderr, "Usage: %s <host> <portnum> <threads> <filename1> [filename2]\n\n", argv[0]);
        fprintf(stderr,
                "host: the name of the host that the web server is running on; the basic web client already handles this argument.\n\n");
        fprintf(stderr,
                "portnum: the port number that the web server is listening on and that the client should send to; the basic web client already handles this argument.\n\n");
        fprintf(stderr,
                "threads: the number of threads that should be created within the web client. Must be a positive integer.\n\n");
        fprintf(stderr,
                "filename1: the name of the file that the client is requesting from the server.\n\n");
        fprintf(stderr,
                "filename2: the name of a second file that the client is requesting from the server. This argument is optional. If it does not exist, then the client should repeatedly ask for only the first file. If it does exist, then each thread of the client should alternate which file it is requesting.\n\n");
        exit(1);
    }
}

/*
 * Send an HTTP request for the specified file
 */
void clientSend(int fd, char *filename) {
    char buf[MAXLINE];
    char hostname[MAXLINE];

    Gethostname(hostname, MAXLINE);

    /* Form and send the HTTP request */
    sprintf(buf, "GET %s HTTP/1.1\n", filename);
    sprintf(buf, "%shost: %s\n\r\n", buf, hostname);
    Rio_writen(fd, buf, strlen(buf));
}

/*
 * Read the HTTP response and print it out
 */
void clientPrint(int fd) {
    rio_t rio;
    char buf[MAXBUF];
    int length = 0;
    int n;

    Rio_readinitb(&rio, fd);

    /* Read and display the HTTP Header */
    n = Rio_readlineb(&rio, buf, MAXBUF);
    while (strcmp(buf, "\r\n") && (n > 0)) {
        printf("Header: %s", buf);
        n = Rio_readlineb(&rio, buf, MAXBUF);

        /* If you want to look for certain HTTP tags... */
        if (sscanf(buf, "Content-Length: %d ", &length) == 1) {
            printf("Length = %d\n", length);
        }
    }

    /* Read and display the HTTP Body */
    strcpy(buf, rio.rio_bufptr);
    printf("%s", buf);
//    n = Rio_readnb(&rio, buf, MAXBUF);
//    while (n > 0) {
//        n = Rio_readlineb(&rio, buf, MAXBUF);
//    }
}