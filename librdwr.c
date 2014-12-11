#include "librdwr.h"

#define VAL_NUM 968

#define BUFF 1024

char* intochar(int num)
{
    int i = 0;
    int minus= 0;
    int stay = 1;
    char* ans = (char*)calloc(50, sizeof(char));
    int max = 0;
    if(num < 0)
    {
        num = num - 2 * num;
        minus = 1;
    }
    int current = num;
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
    if(minus == 0)
    {
        for(i = 1; num != 0; i++)
        {
            ans = (char*)realloc(ans, i + 1);
            stay = num % 10;
            num = num / 10;
            ans[max - i] = stay + '0';
        }
    }
    else
    {
        ans[0] = '-';
        for(i = 1; num != 0; i++)
        {
            stay = num % 10;
            num = num / 10;
            ans[max - i + 1] = stay + '0';
        }
    }
    strcat(ans, " ");
    return ans;
}

char* read_buf(int fd, char* message, int size)
{
    if(size == 0)
    {
        int current = 0;
        int len = read(fd, message, BUFF);
        if(len < 0)
        {
            printf("can't read\n");
        }
        while(len > 0)
        {
            current += len;
            message = (char*)realloc(message, current + BUFF * sizeof(char));
            len = read(fd, message + current, BUFF);

        }
    }
    else
    {
        int current = 0;
        while(current != size)
        {
            int byte = read(fd, message, size);
            if(byte < 0)
            {
                perror("f");
                printf("mistake in read\n");
                exit(-1);
            }
            current += byte;
        }
    }
    return message;
}

void write_buf(int fd, char* message, int size)
{
    int current = 0;
    if(size < BUFF)
    {
        int len = write(fd, message, size);
        if(len < 0)
        {
            printf("can't write\n");
        }
        while(len > 0)
        {
            current += len;
            len = write(fd, message + current, size - current);
        }
    }
    else
    {
        int len = write(fd, message, BUFF);
        current = len;
        if(len < 0)
        {
            printf("can't write\n");
        }
        while((len > 0) && (current != size))
        {
            if(size - current > BUFF)
            {
                len =  write(fd, message + current, BUFF);
            }
            else
            {
                len = write(fd, message + current, size - current);
            }
            current += len;
        }
    }
}
