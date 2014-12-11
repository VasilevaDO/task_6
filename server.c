#include "librdwr.h"


#define P(arg) (arg).sem_op = -1;\
(arg).sem_flg = 0;\
(arg).sem_num = 0;\
semop(idSem, &arg, 1)

#define V(arg) arg.sem_op = 1;\
arg.sem_flg = 0;\
arg.sem_num = 0;\
semop(idSem, &arg, 1)

struct matr{
	int numb;
	int str;
	int* val;
};

void* clwork(void* arg)
{

    int res = 0;
    int i = 0;

    threadarg* tharg = (threadarg*)arg;
    connect2* send = (connect2*)calloc(1, sizeof(connect2));
    connect1* dowork = (connect1*)calloc(1, sizeof(connect1));

    send->mtype = tharg->mtype + tharg->clnum;
    tharg->numb = tharg->numb;
    send->str = tharg->str;
    send->strnum = tharg->strnum;

    int msgsize1 = sizeof(connect1) - sizeof(long);
    int msgsize2 = sizeof(connect2) - sizeof(long);

    dowork->numb = tharg->numb;
    dowork->mtype = tharg->clnum + tharg->mtype;
    strcat(dowork->text, "DoWork");

    res = msgsnd(tharg->id, dowork, msgsize1, 0);
    if(res != 0)
    {
        printf("Can't send message with DoWork\n");
        exit(-1);
    }


    int cur = 0;
    while(cur < tharg->numb * tharg->numb * 2)
    {
        for(i = cur; (i < tharg->numb * tharg->numb * 2) && (i < cur + VAL_NUM); i++)
        {
            send->val[i - cur] = tharg->val[i];
        }

        cur = i;
        res = msgsnd(tharg->id, send, msgsize2 , 0);
        if(res != 0)
        {
            printf("Can't send message with work\n");
            exit(-1);
        }
    }


    int* answer = (int*)calloc(tharg->numb * tharg->numb, sizeof(int));
    cur = 0;

    while(cur < tharg->numb * tharg->numb)
    {
        res = msgrcv(tharg->id, send, msgsize2, tharg->mtype, 0);
        if(res < 0)
        {
            printf("Can't receive message with work\n");
            exit(-1);
        }
        int i = 0;
        for(i = cur; (i < tharg->numb * tharg->numb) && (i < cur + VAL_NUM); i++)
        {
            answer[i] = send->val[i - cur];
        }
        cur = i;

    }
    return (void*)answer;

}

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        printf("Wrong arguments\n");
        exit(-1);
    }

    int fdlog = open("log.txt", O_CREAT | O_TRUNC , 0666);
    if(fdlog < 0)
    {
        printf("Can't open file (server)\n");
        exit(-1);
    }
    close(fdlog);

    int clnum = atoi(argv[1]);
    int i, j;

    int fd = open("file", O_RDONLY);
	if(fd < 0)
		perror("Cant open file\n");
	int numb = 0;
	int* nnumb = (int*)calloc(1, sizeof(int));
	nnumb = (int*)read_buf(fd, (char*)nnumb, sizeof(int));
    numb = nnumb[0];
	struct matr matrix;
    matrix.val = (int*)calloc(2 * numb * numb, sizeof(int));
    matrix.val = (int*)read_buf(fd, (char*)(matrix.val), 0);
    close(fd);


    connect1* first = (connect1*)calloc(1, sizeof(connect1));
    threadarg* message = (threadarg*)calloc(clnum, sizeof(threadarg));

    for(i = 0; i < clnum; i++)
    {
        message[i].val = (int*)calloc(numb * numb * 2, sizeof(int));
        for(j = 0; j < numb * numb * 2; j++)
            message[i].val[j] = matrix.val[j];
    }

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

    for(i = 1; i <= clnum; i++)
    {
        res = msgrcv(id, first, msgsize1, i, 0);
        if(res < 0)
        {
            printf("Can't get message\n");
            exit(-1);
        }
        int k = 0;
        char hello[7] = "Hello\n";
        for(j = 0; j < 7; j++)
        {
            if(hello[j] == first->text[j])
                k++;
        }
        if(k != 7)
            printf("Not hello\n%d %s", i, first->text);
    }

    int strQuant = numb / clnum;
    int cur = 0;
    pthread_t thread[clnum];


    key_t keySem = ftok("client.c", 0);
    int idSem = semget(keySem, 1, 0666 | IPC_CREAT);
    if(idSem < 0)
    {
        printf("can't get sem\n");
        exit(-1);
    }
    struct sembuf sem;
    V(sem);

    for(i = 0; i < clnum; i++)
    {
        message[i].mtype = i + 1;
        message[i].str = cur;
        message[i].numb = numb;
        message[i].clnum = clnum;
        message[i].id = id;
        if(i == clnum - 1)
            message[i].strnum = numb - strQuant * (clnum - 1);
        else
            message[i].strnum =  strQuant;
        cur += strQuant;

        threadarg* inthread = message + i;
        if(pthread_create(thread + i, NULL, clwork, (void*)inthread) != 0)
            printf("bad creating of threads\n");
	}
	
    int* answer = (int*)calloc(numb * numb, sizeof(int));
    void** ansvoidzz = (void**)calloc(1, sizeof(void*));
    for(i = 0; i < clnum; i++)
    {
        if(pthread_join(thread[i], ansvoidzz) != 0)
        {
            printf("Fault with joining threads\n");
        }
        void* ansvoidz = ansvoidzz[0];
        int* ansint = (int*)ansvoidz;
        int ansstr = 0;
        for(ansstr = 0; ansstr < message[i].strnum; ansstr++)
            for(j = 0; j < numb; j ++)
            {
                answer[numb * message[i].str + numb * ansstr + j] = ansint[numb * message[i].str + numb * ansstr + j];
            }

    }

    P(sem);

    int fd1 = open("log.txt", O_WRONLY , 0666);
    if(fd1 < 0)
    {
        printf("Can't open file to write\n");
        exit(-1);
    }
    char** charans = (char**)calloc(numb * numb, sizeof(char*));
    for(i = 0; i < numb * numb; i++)
    {
        charans[i] = intochar(answer[i]);
    }
    for(i = 0; i < numb* numb; i++)
    {
        if(i % numb == 0)
        {
            if(lseek(fd, 0, SEEK_END) < 0)
                printf("Bad lseek\n");
            write_buf(fd1, "\n", 1);
        }
        if(lseek(fd, 0, SEEK_END) < 0)
            printf("Bad lseek\n");
        write_buf(fd1, charans[i], strlen(charans[i]));    }
    V(sem);

    connect1* last = (connect1*)calloc(1, sizeof(connect1));
    for(i = 1; i <= atoi(argv[1]); i++)
    {
        msgrcv(id, last, sizeof(connect1) - sizeof(long), i, 0);
    }

    int f = msgctl(id, IPC_RMID, NULL);
    if(f != 0)
    {
        printf("msgctl = %d\n", f);
        printf("Can't delete messages\n");
        exit(-1);
    }

    if(semctl(idSem, 0, IPC_RMID, NULL) < 0)
    {
        printf("Can't delete sem\n");
        exit(-1);
    }
    close(fd1);

    free(last);
    free(matrix.val);
    free(answer);
    free(message);
    free(first);
    free(charans);
    exit(0);
}
