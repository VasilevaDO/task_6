#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

int main(int argc, char** argv)
{
	if(argc != 2)
		{
			perror("Wrong arguments\n");
			exit(-1);
		}

	int numb = atoi(argv[1]);
	int* array = (int*)calloc(2 * numb * numb, sizeof(int));
	int i = 0;
	int k = 0;
	for(i = 0; i < 2 * numb; i++)
	{
		for(k = 0; k < numb; k++)
		{
			array[i * numb + k] = rand();
		}
	}
	
	int wr = 0;
	
	int fd = open("file", O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if(fd < 0)
		{
			perror("Can't create/open file\n");
			exit(-1);
		}
		
	if(write(fd, &numb, sizeof(int)) != sizeof(int))
		{
			perror("Can't write to file\n");
			exit(-1);
		}
	
	if((wr = write(fd, array, 2 * numb * numb * sizeof(int))) != 2 * numb * numb * sizeof(int))
		{
			printf("Was written only %d\n", wr);
			exit(-1);
		}
			
	if(close(fd) < 0)
		{
			perror("Can't close file\n");
			exit(-1);
		}
	free(array);
	return 0;
}
