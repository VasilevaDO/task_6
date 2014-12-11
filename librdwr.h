#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/time.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>

#define VAL_NUM 968

char* read_buf(int fd, char* message, int size);
void write_buf(int fd, char* message, int size);
typedef struct message
{
    long mtype;
    int val[VAL_NUM];
    int str;
    int strnum;
    int numb;
}connect2;

typedef struct threadarg
{
    long mtype;
    int* val;
    int str;
    int strnum;
    int id;
    int numb;
    int clnum;
}threadarg;

typedef struct mes1
{
    long mtype;
    char text[60];
    int numb;
}connect1;

char* intochar(int num);
