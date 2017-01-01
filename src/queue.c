//
// Created by Zihao on 2016/12/28.
//

#include "queue.h"

void init_queue(queue *q, int capacity) {
    q->front = 0;
    q->rear = -1;
    q->count = 0;
    q->capacity = capacity <= MAX_BUFFER ? capacity : MAX_BUFFER;
}

request *peek(queue *q) {
    if (q->count == 0) {
        return 0;
    }
    return q->requests[q->front];
}

int is_empty(queue *q) {
    return q->count == 0;
}

int is_full(queue *q) {
    return q->count == q->capacity;
}

int size(queue *q) {
    return q->count;
}

int enqueue(queue *q, request *r) {
    if (is_full(q)) {
        return -1;
    }
    if (q->rear == q->capacity - 1) {
        q->rear = -1;
    }
    q->rear++;
    q->requests[q->rear] = r;
    q->count++;
    return 0;
}

request *dequeue(queue *q) {
    if (is_empty(q)) {
        return 0;
    }
    request *r = q->requests[q->front++];
    if (q->front == q->capacity) {
        q->front = 0;
    }
    q->count--;
    return r;
}
