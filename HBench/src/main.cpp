#ifdef DEBUG
#define PRINT(x) x;
#else
#define PRINT(x)
#endif

#include"Benchmark.h"
#include<stdlib.h>
#include<stdio.h>
#include <sys/time.h>

int main(int argc, char *argv[])
{
	srand(time(NULL));


	Benchmark *bb = new Benchmark(argc, argv);
	bb->Init();
	bb->GenerateDotFile();
	bb->run();

}

