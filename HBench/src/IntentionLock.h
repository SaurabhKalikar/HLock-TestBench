#ifndef IL_H
#define IL_H

/* IntentionLock type
 * 4 lock modes: 
	1:IS
	2:IX
	3:S
	4:X
*/
class IL
{
	public:
		/*lock modes: 
		1:IS
		2:IX
		3:S
		4:X
		*/
		int IsIx;

		//Counter for multiple readers/intention lock holders
		int SCounter;
		int ISCounter;
		int IXCounter;

		pthread_mutex_t mlock;
		pthread_rwlock_t rwlock;
	
		//constructor
		IL()
		{
			IsIx=0;
			SCounter = 0;
			ISCounter = 0;
			IXCounter = 0;
			int rc = pthread_rwlock_init(&rwlock, NULL); 
			pthread_mutex_init ( &mlock, NULL);
		}	

		//Acquire intention shared lock
		int ISLock()
		{
			//wait till the mode is Exclusive
			check : while(IsIx == 4);

			pthread_mutex_lock(&mlock);
			if(IsIx != 4)
			{
				pthread_rwlock_rdlock(& rwlock);
				if(IsIx == 0)
					IsIx = 1;
				ISCounter++;
			}
			else
			{
				 pthread_mutex_unlock(&mlock);
				goto check;
			}

 			pthread_mutex_unlock(&mlock);

			return 0;
		}


		//Acquire intention exclusive lock
		int IXLock()
		{
			//wait till the lock mode is S or X
			check : while(IsIx > 2);

			pthread_mutex_lock(&mlock);
			if(IsIx <= 2)
			{
				pthread_rwlock_rdlock(&rwlock);
				//Lock upgradation from IS to IX
				if(IsIx < 2)
					IsIx = 2;
				IXCounter++;
			}
			else
			{
				pthread_mutex_unlock(&mlock);
				goto check;
			}

			pthread_mutex_unlock(&mlock);

			return 0;
		}

		//Acquire shared lock
		int SLock()
		{
		check : while(IsIx == 2 ||  IsIx == 4);

			pthread_mutex_lock(&mlock);
			if(IsIx != 2 && IsIx != 4)
			{
				pthread_rwlock_rdlock(& rwlock);
				if(IsIx < 3)
					IsIx=3;
				SCounter++;
			}
			else
			{
				pthread_mutex_unlock(&mlock);
				goto check;
			}

			pthread_mutex_unlock(&mlock);
			return 0;
		}

		//Acquire exclusive lock
		int XLock()
		{
			pthread_rwlock_wrlock(&rwlock);
			IsIx=4;
			return 0;
		}

		//Unlock intention shared lock
		int ISUnLock()
		{
			pthread_mutex_lock(&mlock);

			//Error code 1: Unlocking already unlocked lock
			if(ISCounter == 0)
				return 1;
			ISCounter--;
			if( ISCounter == 0 && IXCounter == 0 && SCounter == 0)
				IsIx = 0;

			pthread_rwlock_unlock(&rwlock);
			
			pthread_mutex_unlock(&mlock);

			return 0;
		}
		
		//Unlock intention exclusive lock
		int IXUnLock()
		{
			pthread_mutex_lock(&mlock);
			//Error code 1: Unlocking already unlocked lock
			if(IXCounter == 0)
				return 1;
			IXCounter--;
		
			//Downgrade from IX->IS
			if(IXCounter == 0 && ISCounter != 0)
				IsIx = 1;
			
			if( ISCounter == 0 && IXCounter == 0 && SCounter == 0)
				IsIx = 0;

			pthread_rwlock_unlock(&rwlock);
			
			pthread_mutex_unlock(&mlock);
			return 0;

		}
 		//Unlock shared lock
		int SUnLock()
		{
			pthread_mutex_lock(&mlock);
			//Error code 1: Unlocking already unlocked lock
			if(SCounter == 0)
				return 1;
			SCounter--;
		
			//Downgrade from S->IX
			if(SCounter == 0 && IXCounter != 0)  
				IsIx = 2;
			else if(SCounter == 0 && ISCounter != 0)//Downgrade from S->IS
				IsIx = 1;
			else if(SCounter == 0 && IXCounter == 0 && ISCounter == 0)//reset
				IsIx = 0;

			pthread_rwlock_unlock(&rwlock);
			
			pthread_mutex_unlock(&mlock);
				
			return 0;
		}

		//Unlock exclusive lock
		int XUnLock()
		{
			IsIx = 0;
			//Error code 1: Unlocking already unlocked write-lock
			if(pthread_rwlock_trywrlock(&rwlock) == 0)
				return 1;
			pthread_rwlock_unlock(&rwlock);
			return 0;
		}
		
		//Print stats 
		void PrintStat()
		{
			std::cout<<"------------Stats-----------\n";
			std::cout<<"Intention Shared: \t"<<ISCounter<<endl;
			std::cout<<"Intention Exclusive: \t"<<IXCounter<<endl;
			std::cout<<"Shared: \t"<<SCounter<<endl;
			
		}

};

#endif
