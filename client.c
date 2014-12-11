#include "librdwr.h"

#define P(arg) arg.sem_op = -1;\
arg.sem_flg = 0;\
arg.sem_num = 0;\
semop(idsem, &arg, 1)

#define V(arg) arg.sem_op = 1;\
arg.sem_flg = 0;\
arg.sem_num = 0;\
semop(idsem, &arg, 1)


int main(int argc, char* argv[])
{
    int i = 0;
    connect2* rcv = (connect2*)calloc(1, sizeof(connect2));
    connect2* snd = (connect2*)calloc(1, sizeof(connect2));
    snd->mtype = atoi(argv[1]);
    connect1* first = (connect1*)calloc(1, sizeof(connect1));

    key_t key = ftok("server.c", 0);
    if(key < 0)
    {
        printf("Can't get key\n");
        exit(-1);
    }

    int id = msgget(key, IPC_CREAT | 0666);
    if(id < 0)
    {
        printf("Can't get id for msg\n");
        exit(-1);
    }

    int res = 0;
    int msgsize1 = sizeof(connect1) - sizeof(long);

    strcpy(first->text, "Hello\n");
    first->mtype = atoi(argv[1]);

    res = msgsnd(id, first, msgsize1, 0);
    if(res != 0)
    {
        printf("Can't send message \"Hello\"\n");
        exit(-1);
    }

    res = msgrcv(id, first, msgsize1, atoi(argv[2]) + atoi(argv[1]), 0);
    if(res < 0)
    {
        printf("client didn't receive message\n");
        exit(-1);
    }

    int numb = first->numb;
    int msgsize2 = sizeof(connect2) - sizeof(long);
    int* massiv = (int*)calloc(numb * numb * 2, sizeof(int));


    int j = 0;
    int k = 0;
    char dowork[7] = "DoWork";
    for(j = 0; j < 6; j++)
    {
        if(dowork[j] == first->text[j]) 
			k++;
    }
    if(k != 6)
        printf(">>%s<< not dowork\n", first->text);

    int current = 0;
    while(current < numb * numb * 2)
    {
        res = msgrcv(id, rcv, msgsize2, atoi(argv[1]) + atoi(argv[2]), 0);
        if(res < 0)
        {
            printf("Can't receive work\n");
            exit(-1);
        }
        for(i = current; (i < numb * numb * 2) && (i < current + VAL_NUM); i++)
        {
            massiv[i] = rcv->val[i - current];
        }
        current = i;
    }

    //Собственно наши вычисления
    int* answer = (int*)calloc(numb * numb, sizeof(int));
    for(i = rcv->str; i < rcv->strnum + rcv->str; i++)
    {
        for(j = 0; j < numb; j++)
        {
            answer[numb * i + j] = 0;
            int k = 0;
            for(k = 0; k < numb; k++)
            {
                answer[numb * i + j] += massiv[i * numb + k] * massiv[numb * numb + k * numb + j];
            }
        }
    }
	//Всё вычислили, отправляем

    current = 0;
    while(current < numb * numb)
    {
        for(i = current; (i < numb * numb) && (i < current + VAL_NUM); i++)
        {
            snd->val[i - current] = answer[i];
        }
        res = msgsnd(id, snd, msgsize2, 0);
        if(res != 0)
        {
            printf("Can't send work that was done\n");
            exit(-1);
        }
        current = i;
    }

    key_t keysem = ftok("client.c", 0);
    if(keysem < 0)
        printf("Can't get key\n");
    int idsem = semget(keysem, 1, 0666 | IPC_CREAT);
    if(idsem < 0)
    {
        printf("Can't get sem\n");
        exit(-1);
    }
    struct sembuf sem;
    struct timeval* mytime = (struct timeval*)calloc(1, sizeof(struct timeval));
	gettimeofday(mytime, NULL);

    char* writeinlog = (char*)calloc(300, sizeof(char));
    strcat(writeinlog, "Hi, I'm a client number ");
    strcat(writeinlog, argv[1]);
    strcat(writeinlog, " from process number");
    int pid = getpid();
    strcat(writeinlog, intochar(pid));
    strcat(writeinlog, ". My work was done in ");
    strcat(writeinlog, intochar(mytime->tv_usec));
    strcat(writeinlog, " ms\n");

    P(sem);

    int fd = open("log.txt", O_WRONLY , 0666);
    if(fd < 0)
    {
        printf("Can't open file client\n");
        exit(-1);
    }
    if(lseek(fd, 0, SEEK_END) < 0)
        printf("bad lseek\n");
    write_buf(fd, writeinlog, strlen(writeinlog));
    V(sem);
    connect1* last = (connect1*)calloc(1, sizeof(connect1));
    last->mtype = atoi(argv[1]);
    if(msgsnd(id, last, msgsize1, 0) != 0)
    {
        printf("Can't send\n");
        exit(-1);
    }
    free(last);
    free(rcv);
    free(snd);
    exit(0);

}
