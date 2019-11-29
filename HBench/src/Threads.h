#ifndef Threads_H
#define Threads_H
#include"Hierarchy.h"
#include"Parameters.h"
#include"Operations.h"
#include <pthread.h>

using namespace std;

class threadData;
class Thread
{
	private:
		int NUM_THREADS;
	public:
		Operations *op;

		Thread(Operations *op);
		void CreateThreads(Parameters* ,Hierarchy*);
		
		static void* ParallelTask(void *TD)
		{
			threadData *data = (threadData *)TD;
		
			//Operation on hierarchy
//			data->pH->ParallelTask(data);
//			Operations *op = new Operations();	
			data->op->OperateLeafNodes(data);
			
			return NULL;
		}		
};

Thread::Thread(Operations *op)
{
	this->op = op;
}

void Thread::CreateThreads(Parameters *pC, Hierarchy *pH)
{
	NUM_THREADS = pC->getNumThreads();

	threadData *TD[NUM_THREADS];

	pthread_t threadArray[NUM_THREADS];

	for(int i=0; i < NUM_THREADS; i++ )
	{
		rand();
		TD[i] = new threadData();
		TD[i]->pH = pH;
		TD[i]->pC = pC;
		TD[i]->threadID = i;
		TD[i]->op = this->op;
		int TCS = pthread_create(&threadArray[i], NULL,&Thread::ParallelTask,(void *)TD[i]);
		if (TCS!=0)
		{
			cout << "Error:unable to create thread," << TCS << endl;
			exit(-1);
		}
	}

	for(int i = 0; i < NUM_THREADS; i++)
		pthread_join( threadArray[i], NULL );

}
#endif



