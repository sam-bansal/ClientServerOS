#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>

#define PRINT_INFO(MSG, ...) printf ( "%s INFO %d:%d %ld %s %s %d : " MSG ";;\n", \
	"TODO_PRINT_TIME", getpid(), getppid(), pthread_self(), __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define PRINT_ERROR(MSG, ...) printf ( "%s ERROR %d:%d %ld %s %s %d : " MSG ";;\n", \
	"TODO_PRINT_TIME", getpid(), getppid(), pthread_self(), __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)

int main(int argc, char** argv){
	int i;
	PRINT_INFO("No. Args passed = %d", argc);
	for (i =0; i < argc; i++)
		PRINT_INFO("argv[%d] = [%s]", i, argv[i]);
	return 0;
}