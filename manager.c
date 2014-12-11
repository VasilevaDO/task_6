#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <string.h>

//Пользоваться itoa — не по хардкору :)

char* intochar(int num)
{
    int i = 0;
    int stay = 1;
    char* ans = (char*)calloc(2, sizeof(char));
    int max = 0;
    int current = num;
    if(current == 0)
        return "0 ";
    while(current != 0)
    {
        current = current / 10;
        max++;
    }
    if(num == 0)
    {
        ans[0] = '0';
        ans[1] = ' ';
    }
    for(i = 1; num != 0; i++)
    {
        ans = (char*)realloc(ans, i + 1);
        stay = num % 10;
        num = num / 10;
        ans[max - i] = stay + '0';
    }
    strcat(ans, " ");
    return ans;
}

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        printf("Wrong arguments\n");
        exit(-1);
    }

    int res = fork();
    if(res == 0)
    {
        char* snsser = (char*)calloc(40, sizeof(char));

        strcat(snsser, "./ser.out ");
        strcat(snsser, argv[1]);

        execlp("sh", "sh", "-c", snsser, NULL);
    }
    int i = 0;
    for(i = 1; i <= atoi(argv[1]); i++)
    {
        res = fork();

        if(res == 0)
        {
            char* sndcli = (char*)calloc(40, sizeof(char));
            strcat(sndcli, "./cli.out ");
            strcat(sndcli, intochar(i));
            strcat(sndcli, argv[1]);

            execlp("sh", "sh", "-c", sndcli, NULL);
            printf("error\n");
            exit(-1);
        }
    }
    exit(0);

}
