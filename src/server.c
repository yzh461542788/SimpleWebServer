/**
 * server.c: A multi-thread simple web server
 */

#include "server.h"
#include "request.h"
#include "queue.h"

static pthread_cond_t buffer_has_empty_cond = PTHREAD_COND_INITIALIZER;
static pthread_cond_t buffer_has_content_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;

static char *shedalg;
static queue used_buffer;  // buffer queue for FIFO
static queue free_buffer;
static queue static_buffer;  // for HPDC and HPSC
static queue dynamic_buffer;

int main(int argc, char *argv[]) {
    int listenfd;
    int connfd;
    int port;
    int clientlen;
    int thread_num;
    int buffer_num;

    getargs(&port, &thread_num, &buffer_num, &shedalg, argc, argv);

    pthread_t tid[thread_num];
    struct sockaddr_in clientaddr;

    // Init buffers
    init_queue(&free_buffer, buffer_num);
    init_queue(&used_buffer, buffer_num);
    init_queue(&static_buffer, buffer_num);
    init_queue(&dynamic_buffer, buffer_num);
    request buffer[buffer_num];
    int i;
    for (i = 0; i < buffer_num; ++i) {
        enqueue(&free_buffer, buffer + i);
    }

    // Create worker threads
    if (strcmp(shedalg, "FIFO") == 0 || strcmp(shedalg, "ANY") == 0) {
        for (i = 0; i < thread_num; ++i) {
            pthread_create(&tid[i], NULL, &fifo_worker, NULL);
        }
    } else if (strcmp(shedalg, "HPDC") == 0) {
        for (i = 0; i < thread_num; ++i) {
            pthread_create(&tid[i], NULL, &hpdc_workder, NULL);
        }
    } else {
        for (i = 0; i < thread_num; ++i) {
            pthread_create(&tid[i], NULL, &hpsc_workder, NULL);
        }
    }

    listenfd = Open_listenfd(port);

    // main thread for accepting new http connections and schedule
    while (1) {
        // get request info
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *) &clientaddr, (socklen_t *) &clientlen);
        request *r = (request *) malloc(sizeof(request));
        getRequest(r, connfd);

        // wait for a empty buffer if there is no one
        pthread_mutex_lock(&buffer_mutex);
        if (is_empty(&free_buffer)) {
            pthread_cond_wait(&buffer_has_empty_cond, &buffer_mutex);
        }
        request *buf = dequeue(&free_buffer);
        *buf = *r;
        free(r);

        if (strcmp(shedalg, "FIFO") == 0 || strcmp(shedalg, "ANY") == 0) {
            enqueue(&used_buffer, buf);
        } else if (buf->is_static) {
            enqueue(&static_buffer, buf);
        } else {
            enqueue(&dynamic_buffer, buf);
        }
        // free the lock and waken worker threads
        pthread_cond_signal(&buffer_has_content_cond);
        pthread_mutex_unlock(&buffer_mutex);

        //
        // CSE: In general, don't handle the request in the main thread.
        // Save the relevant info in a buffer and have one of the worker threads
        // do the work.

        // requestHandle(connfd);
    }
}

/**
 * Parse the arguments
 * @param port the port number that the web server should listen on, should be above 2000
 * @param threads the number of worker threads that should be created within the web server. Must be a positive integer.
 * @param buffers the number of request connections that can be accepted at one time. Must be a positive integer.
 * @param schedalg the scheduling algorithm to be performed. Must be one of ANY, FIFO, HPSC, or HPDC.
 * @param argc arg number
 * @param argv args
 */
void getargs(int *port, int *threads, int *buffers, char **schedalg, int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: server [portnum] [threads] [buffers] [schedalg]\n\n");
        fprintf(stderr,
                "portnum: the port number that the web server should listen on, should be above 2000\n\n");
        fprintf(stderr,
                "threads: the number of worker threads that should be created within the web server. Must be a positive integer.\n\n");
        fprintf(stderr,
                "buffers: the number of request connections that can be accepted at one time. Must be a positive integer.\n\n");
        fprintf(stderr,
                "schedalg: the scheduling algorithm to be performed. Must be one of ANY, FIFO, HPSC, or HPDC.\n\n");
        exit(1);
    }
    *port = atoi(argv[1]);
    if (*port <= 2000) {
        fprintf(stderr,
                "port: the port number that the web server should listen on. Must be greater than 2000.\n");
        exit(1);
    }

    *threads = atoi(argv[2]);
    if (*threads <= 0) {
        fprintf(stderr,
                "threads: the number of worker threads that should be created within the web server. Must be a positive integer.\n");
        exit(1);
    }

    *buffers = atoi(argv[3]);
    if (*buffers <= 0) {
        fprintf(stderr,
                "buffers: the number of request connections that can be accepted at one time. Must be a positive integer.\n");
        exit(1);
    }

    if (strcmp(argv[4], "ANY") == 0 || strcmp(argv[4], "FIFO") == 0 ||
        strcmp(argv[4], "HPSC") == 0 || strcmp(argv[4], "HPDC") == 0) {
        *schedalg = argv[4];
    } else {
        fprintf(stderr,
                "schedalg: the scheduling algorithm to be performed. Must be one of ANY, FIFO, HPSC, or HPDC.\n");
        exit(1);
    }
}

void fifo_worker() {
    while (1) {
        pthread_mutex_lock(&buffer_mutex);
        // wait for a buffer with content if there is no one
        while (is_empty(&used_buffer)) {
            pthread_cond_wait(&buffer_has_content_cond, &buffer_mutex);
        }
        request *buffer = dequeue(&used_buffer);
        pthread_mutex_unlock(&buffer_mutex);
        requestHandle(buffer);

        pthread_mutex_lock(&buffer_mutex);
        enqueue(&free_buffer, buffer);
        pthread_cond_signal(&buffer_has_empty_cond);
        pthread_mutex_unlock(&buffer_mutex);
    }
}

void hpsc_workder() {
    while (1) {
        // lock the buffer read/write mutex
        pthread_mutex_lock(&buffer_mutex);
        // wait for a buffer with content if there is no one
        while (is_empty(&static_buffer) && is_empty(&dynamic_buffer)) {
            pthread_cond_wait(&buffer_has_content_cond, &buffer_mutex);
        }
        request *buffer;
        if (!is_empty(&static_buffer)) {
            buffer = dequeue(&static_buffer);
        } else {
            buffer = dequeue(&dynamic_buffer);
        }
        // unlock the buffer read/write mutex
        pthread_mutex_unlock(&buffer_mutex);

        requestHandle(buffer);

        // re-lock the buffer read/write mutex
        pthread_mutex_lock(&buffer_mutex);
        enqueue(&free_buffer, buffer);
        pthread_cond_signal(&buffer_has_empty_cond);
        pthread_mutex_unlock(&buffer_mutex);
    }
}

void hpdc_workder() {
    while (1) {
        // lock the buffer read/write mutex
        pthread_mutex_lock(&buffer_mutex);
        // wait for a buffer with content if there is no one
        while (is_empty(&dynamic_buffer) && is_empty(&static_buffer)) {
            pthread_cond_wait(&buffer_has_content_cond, &buffer_mutex);
        }
        request *buffer;
        if (!is_empty(&dynamic_buffer)) {
            buffer = dequeue(&dynamic_buffer);
        } else {
            buffer = dequeue(&static_buffer);
        }
        // unlock the buffer read/write mutex
        pthread_mutex_unlock(&buffer_mutex);

        requestHandle(buffer);

        // re-lock the buffer read/write mutex
        pthread_mutex_lock(&buffer_mutex);
        enqueue(&free_buffer, buffer);
        pthread_cond_signal(&buffer_has_empty_cond);
        pthread_mutex_unlock(&buffer_mutex);
    }
}
