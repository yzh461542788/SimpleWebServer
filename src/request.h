#include "cse.h"

#ifndef __REQUEST_H__
#define __REQUEST_H__

/* store request info */
typedef struct {
    int is_static;
    int fd;
    char method[MAXLINE];
    char uri[MAXLINE];
    char version[MAXLINE];
    char filename[MAXLINE];
    char cgiargs[MAXLINE];
} request;


void requestError(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
int requestParseURI(char *uri, char *filename, char *cgiargs);
void requestGetFiletype(char *filename, char *filetype);
void requestServeDynamic(int fd, char *filename, char *cgiargs);
void requestServeStatic(int fd, char *filename, int filesize);
void requestHandle(request *r);
int getRequest(request *r, int fd);
#endif