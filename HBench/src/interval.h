#ifndef INTERVAL_H_
#define INTERVAL_H_

#include <vector>
#include <stdlib.h>
using namespace std;
#define SIZE 20


class interval{

	public: double pre, post; 
		int mode;
		long MySeq;
		interval( double a, double b, int m){
			pre = a; post = b; mode = m;
		}
};

//Interface class for lock-pool implementations
class LockPool
{
	public: 
		//Lock pool for Multi-DomLock, .i.e., multiple lock node per thread
		vector<interval**>MultiLockPool;

		//Size of thread specific list of intervals (locks)
		int VectorSize[SIZE];
		
		//-----------Constructor--------------------//
		LockPool()
		{
			MultiLockPool.resize(SIZE, NULL);
		}
		//-----------virtual functions as interface------------//
		virtual int Insert(interval *inv, int m, int threadID){}
		
		virtual int Insert(interval **QueryNodes, int Qsize, int m, int threadID){}
		virtual int Delete(int index){}
};

//Implementation of lock-pool using mutex lock
class LockPool_Mutex:public LockPool
{
	private:
		pthread_mutex_t MutexLock;

	public:
		//constructor
		LockPool_Mutex()
		{
			pthread_mutex_init(&MutexLock, NULL);
		}
		
		//Request with single interval or single DomLock
		int Insert(interval *inv, int m, int threadID)
		{
			interval **QueryNodes = new interval*[1];
			QueryNodes[0] = inv;
			Insert(QueryNodes, 1, m, threadID);
		}
	
		//Insert intervals to lock-pool using mutex lock
		int Insert(interval **QueryNodes, int Qsize, int m, int threadID)
		{	
		
		
			RepeatLoop:
				pthread_mutex_lock(&MutexLock);

						
				//for each list in lock-pool
				for(int i=0; i< SIZE; i++)
				{

					interval** ptr = MultiLockPool[i];
					if(ptr != NULL)
					{
						//for each entry in the list
						for(int j=0;j<VectorSize[i];j++)
						{
							//for each interval in the request set
							for(int k=0;k<Qsize;k++)
							{
								/*If intervals overlap?
								1. list is non-empty
								2. My seqNo is smaller AND
								3. At least one is write operation
								4. Intervals overlap
								*/
								if(ptr!=NULL && 
									(m == 1 || (m == 0 && ptr[j]->mode == 1)) && 
									ptr[j]->pre <= QueryNodes[k]->post && ptr[j]->post >= QueryNodes[k]->pre)
								{
									pthread_mutex_unlock(&MutexLock);
									sleep(1000);
									goto RepeatLoop;					
								}
							}

						}
					}
				}
			pthread_mutex_unlock(&MutexLock);	
			return 1;
		}
		
		//-------Delete the entry from lock-pool
		int Delete(int index)
		{
			MultiLockPool[index] = NULL;
		}

		

};

//Implementation of lock pool using pthread reader-writer locks
class LockPool_RWLock:public LockPool
{

};
class LockPool_SeqLock:public LockPool
{
	private:
		long Seq;

		//Sequence number per thread for fairness and less contention
		int MySeq[SIZE];
		
		//Mutex for atomic: increment seq-numebr and insert into pool
		pthread_mutex_t MutexLock;
		
	public:
		LockPool_SeqLock()
		{
			Seq = 0;
			pthread_mutex_init(&MutexLock, NULL);
		}

		int Insert(interval *inv, int m, int threadID)
		{
			interval **QueryNodes = new interval*[1];
			QueryNodes[0] = inv;
			Insert(QueryNodes, 1, m, threadID);
		}

		int Insert(interval **QueryNodes, int Qsize, int m, int threadID)
		{
			//set list size
			VectorSize[threadID] = Qsize;

			//Get next seq. number and insert the list into lock pool: ATOMIC
			pthread_mutex_lock(&MutexLock);//-----lock

			MySeq[threadID] = ++Seq;
			MultiLockPool[threadID] = QueryNodes;

			pthread_mutex_unlock(&MutexLock);//------unlock
			
			//for each list in lock-pool
			for(int i=0; i< SIZE; i++)
			{

				interval** ptr = MultiLockPool[i];
				if(ptr != NULL)
				{
					//for each entry in the list
					for(int j=0;j<VectorSize[i];j++)
					{
						//for each interval in the request set
						for(int k=0;k<Qsize;k++)
						{
							/*Spin while: "overlapping list is not set to null"
							1. list is non-empty
							2. My seqNo is smaller AND
							3. At least one is write operation
							4. Intervals overlap
							*/
							while(ptr!=NULL && MySeq[threadID] > MySeq[i] && 
								(m == 1 || (m == 0 && ptr[j]->mode == 1)) && 
								ptr[j]->pre <= QueryNodes[k]->post && ptr[j]->post >= QueryNodes[k]->pre)
							{
								ptr = MultiLockPool[i];
								if(ptr == NULL){
								k = Qsize;
								break;
								}

							}
						}

					}
				}
			}
			return false;
			//-------------
		}

		//-------Delete the entry from lock-pool
		int Delete(int index)
		{
			MultiLockPool[index] = NULL;
		}

};

class HiFiLockPool_SeqLock:public LockPool
{
	private:
		long Seq;

		//Sequence number per thread for fairness and less contention
		int MySeq[SIZE];
		
		//Mutex for atomic: increment seq-numebr and insert into pool
		pthread_mutex_t MutexLock;
		
	public:
		HiFiLockPool_SeqLock()
		{
			Seq = 0;
			pthread_mutex_init(&MutexLock, NULL);
		}

		int Insert(interval *inv, int m, int threadID)
		{
			interval **QueryNodes = new interval*[1];
			QueryNodes[0] = inv;
			Insert(QueryNodes, 1, m, threadID);
		}

		int Insert(interval **QueryNodes, int Qsize, int m, int threadID)
		{
			// overlap checking for HiFi 
			VectorSize[threadID] = Qsize;

			pthread_mutex_lock(&MutexLock);
			
			//Atomic exclusive access
			MySeq[threadID] = ++Seq;
			MultiLockPool[threadID] = QueryNodes;

			pthread_mutex_unlock(&MutexLock);
			for(int i=0; i< SIZE; i++)
			{

				interval** ptr = MultiLockPool[i];
				//Lock free implementation using pointer updation (implicit atomic update)
				if(ptr != NULL)
				{		
					for(int j=0;j<VectorSize[i];j++)
					{
						for(int k=0;k<Qsize;k++)
						{ 
							// Interval overlap conditions for HiFi compatibility matric						
							while(ptr!=NULL && MySeq[threadID] > MySeq[i] && 
								((ptr[j]->pre <= QueryNodes[k]->post && ptr[j]->post >= QueryNodes[k]->pre && ptr[j]->mode==1 && QueryNodes[k]->mode==1)||
								(ptr[j]->mode==1 && QueryNodes[k]->mode==0 && ptr[j]->pre<=QueryNodes[k]->pre && ptr[j]->post>=QueryNodes[k]->post)||
								(ptr[j]->mode==0 && QueryNodes[k]->mode==1 && ptr[j]->pre>=QueryNodes[k]->pre && ptr[j]->post<=QueryNodes[k]->post)||
								(ptr[j]->mode==0 && QueryNodes[k]->mode==0 && ptr[j]->pre==QueryNodes[k]->pre && ptr[j]->post==QueryNodes[k]->post)
								)
								)
							{

								
								ptr = MultiLockPool[i];
								if(ptr == NULL){
								k = Qsize;
								break;
								}


							}
						}

					}
				}

			}
				return false;
		}	
			
		//-------Delete the entry from lock-pool
		int Delete(int index)
		{
			MultiLockPool[index] = NULL;
		}


};
//**************************************************************************************************************
/*
class IntervalCheck
{
	public:
		interval *Pool[SIZE];
		int VectorSize[SIZE];
		//Lock variable, one per lock-pool location
		pthread_rwlock_t PoolLock[SIZE];

		//Lock pool for Multi-DomLock, .i.e., multiple lock node per thread
		vector<interval**>MultiLockPool;

		//Sequence number per thread for fairness and less contention
		int MySeq[SIZE];

		//*****************************************************************************
		//Constructor for initialization of class variables
		//*****************************************************************************
		IntervalCheck()
		{
			Seq = 0;
			for(int i = 0;i<SIZE; i++)
			{

				Pool[i] = NULL;
				pthread_rwlock_t PoolLock[i];
			}
			MultiLockPool.resize(SIZE, NULL);



		}

		bool IsOverlap(interval *inv, int m, int threadID)
		{
			//cout<<"m=1";
			pthread_mutex_lock(&mutex);
			inv->MySeq = ++Seq;
			Pool[threadID] = inv;
			pthread_mutex_unlock(&mutex);

			if(m == 1)
			{	
				for(int i=0; i< SIZE; i++)
				{
					if(Pool[i] != NULL)
					{	interval *ptr = Pool[i];	
						//wait untill there is an overlap and my sequence number is greater
						while(ptr !=NULL && (ptr->pre <= inv->post && ptr->post >= inv->pre) && ptr->MySeq < inv->MySeq)
						{
							ptr = Pool[i];
						}
					}
				}
				return false;
			} 
		}

		void Insert(interval *inv, int index)
		{
			//pthread_rwlock_wrlock(&PoolLock[index]);	
			Pool[index] = inv;
			//pthread_rwlock_unlock(&PoolLock[index]);
		}

		void Delete(int index)
		{
			Pool[index] = NULL;
		}

		//*****************************************************************************
		//This function checks whether the multiple query nodes overlap with the lock pool
		//*****************************************************************************
		bool MultiOverlap(interval **QueryNodes, int Qsize, int m, int threadID)
		{

			VectorSize[threadID] = Qsize;

			pthread_mutex_lock(&mutex);
			MySeq[threadID] = ++Seq;
			MultiLockPool[threadID] = QueryNodes;
			pthread_mutex_unlock(&mutex);

			//--------------
			for(int i=0; i< SIZE; i++)
			{

				interval** ptr = MultiLockPool[i];
				if(ptr != NULL)
				{
					for(int j=0;j<VectorSize[i];j++)
					{
						for(int k=0;k<Qsize;k++)
						{
							while(ptr!=NULL && MySeq[threadID] > MySeq[i] && 
								(m == 1 || (m == 0 && ptr[j]->mode == 1)) && 
								ptr[j]->pre <= QueryNodes[k]->post && ptr[j]->post >= QueryNodes[k]->pre)
							{
								ptr = MultiLockPool[i];
								if(ptr == NULL){
								k = Qsize;
								break;
								}

							}
						}

					}
				}

			}
			return false;
			//-------------

		}
		//This function deletes the vector entry from MultiLockPool, i.e., UnLock
		void MultiDelete(int index)
		{
			MultiLockPool[index] = NULL;
		}
};
*/
#endif
