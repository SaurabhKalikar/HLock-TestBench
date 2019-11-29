#ifndef OPERATION_H
#define OPERATION_H

#include"Hierarchy.h"
#include"DomLockProtocol.h"
#include"NumLockProtocol.h"
#include"IntentionLockProtocol.h"

#include"Parameters.h"
#include"threadData.h"
#include<set>
class Operations
{
	private:
		void NullOperation(char CSSize)
		{
			int itr;
			if(CSSize == 's')
				itr = 1000;
			else if(CSSize == 'm')
				itr = 10000;
			else if(CSSize == 'l')
				itr = 1000000;

			for(int i=0; i<itr; i++)
			{		
				int x = 5, y;
				y = x;
			}

			asm("");
		}
	public:

		LockProtocol *protocol;

		Operations(LockProtocol *protocol)
		{
			this->protocol = protocol;
		}

		void OperateLeafNodes(threadData *td)
		{
			int qs = td->pC->getQuerySize();			 
			vector<Node*> LockSet;	
			set<int> RemoveDuplicates;

			int NumLeafNodes = td->pH->leafList.size();
			int Skewness = td->pC->getSkewness(); 
			int SlotSize = ceil((double)NumLeafNodes/Skewness);
			int SlotNo = rand()% Skewness;
			int Offset = SlotSize*SlotNo;
			
			while(RemoveDuplicates.size() != qs)
			{
				int n  = Offset + rand() % SlotSize;

				if(RemoveDuplicates.find(n) == RemoveDuplicates.end())
				{
					LockSet.push_back(td->pH->leafList[n]);
					RemoveDuplicates.insert(n);
				}
			}
			//Lock
			PRINT(cout<<"Protocol obj"<<this->protocol<<endl;)
			protocol->LockAll(LockSet, td->threadID);

			PRINT(cout<<"Working on operation\n";)
			//Perform operations
			NullOperation(td->pC->getCSSize());	

			protocol->UnLockAll(td->threadID);

		}
};

#endif

