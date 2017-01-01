//
// Created by Zihao on 2016/12/28.
//

#ifndef SIMPLEWEBSERVER_QUEUE_H
#define SIMPLEWEBSERVER_QUEUE_H

#include "request.h"

#define MAX_BUFFER 500


typedef struct {
    request *requests[MAX_BUFFER];
    int front;
    int rear;
    int count;
    int capacity;
} queue;

void init_queue(queue *q, int capacity);

request *peek(queue *q);

int is_empty(queue *q);

int is_full(queue *q);

int size(queue *q);

int enqueue(queue *q, request *r);

request *dequeue(queue *q);

#endif //SIMPLEWEBSERVER_QUEUE_H
