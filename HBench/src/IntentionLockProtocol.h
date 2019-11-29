#ifndef IntentionLockProtocol_H
#define IntentionLockProtocol_H
#include"Hierarchy.h"
#include"Node.h"
#include"LockProtocol.h"
#include"IntentionLock.h"
#include<vector>
#include<map>

bool NodeComparator(Node* a, Node* b)
{
	return (a->data < b->data);
}
class IntentionLockProtocol: public LockProtocol
{
	public:
	pthread_rwlock_t StrModLock;	
	
	//Readonly
	vector< vector<int> > ThreadLocalLockCompatibility;

	//Shared 
	map<int, map<Node*, int> > ThreadToLockHoldingsMap;
	//Constructor
	IntentionLockProtocol(Hierarchy *H, int NumThreads)
	{
		int rc = pthread_rwlock_init(&StrModLock, NULL); 
		initThreadLocalLockCompatibility(ThreadLocalLockCompatibility);
		initThreadToLockHoldingsMap(NumThreads);
	}
	
	void initThreadToLockHoldingsMap(int NumThreads)
	{
		for(int i = 0; i < NumThreads; i++)
		{
			map<Node*, int> m;
			ThreadToLockHoldingsMap[i] = m;
		}
	}

	/* 0: No change in lock mode
	 * 1: Lock mode changes to IS
	 * 2: Lock mode changes to IX
	 * 3: Lock mode changes to S
	 * 4: Lock mode changes to X
	 * */
	void initThreadLocalLockCompatibility(vector< vector<int> > &v)
	{
		vector<int> temp(4);
		temp[0] = 0;temp[1] = 2;temp[2] = 3; temp[3] = 4;
		v.push_back(temp);
		temp[0] = 2;temp[1] = 0;temp[2] = 4; temp[3] = 4;
		v.push_back(temp);
		temp[0] = 0;temp[1] = 4;temp[2] = 0; temp[3] = 4;
		v.push_back(temp);
		temp[0] = 4;temp[1] = 4;temp[2] = 4; temp[3] = 0;
		v.push_back(temp);
	
	}

	void LockAll(vector<Node*>&LockSet, int ThreadId)
	{
		//Sort Lockset according to total order
		sort(LockSet.begin(), LockSet.end(), NodeComparator);
		int mode = 4;
		map<Node*, int> LockHoldings;

		pthread_rwlock_rdlock(&StrModLock);

		for(int i = 0 ; i< LockSet.size(); i++)
		{
			Node* p = LockSet[i];
			PRINT(cout<<"Query: "<<p->data<<endl;);
			FindAndLockAllParents(p, mode, LockHoldings);
			LockHoldings[p] = AcquireLock(p, mode);	
		}

		pthread_rwlock_unlock(&StrModLock);
		ThreadToLockHoldingsMap[ThreadId] = LockHoldings;

	}

	void UnLockAll(int ThreadId)
	{
		for (map<Node*, int>::iterator it = ThreadToLockHoldingsMap[ThreadId].begin(); it != ThreadToLockHoldingsMap[ThreadId].end(); it++)
		{
			Node *p = (*it).first;
			int mode = (*it).second;

			ReleaseLock(p, mode);
		}
		
		ThreadToLockHoldingsMap[ThreadId].clear(); 
	}

	int AcquireLock(Node* p , int mode)
	{
		switch(mode){
			case 1:
			p->lock.intentionLock->ISLock();
			break;
			case 2:
			p->lock.intentionLock->SLock();
			break;
			case 3:
			p->lock.intentionLock->IXLock();
			break;
			case 4:
			p->lock.intentionLock->XLock();
			break;
		};
		PRINT(cout<<p->data<<":" <<mode<<endl;);	
		return mode;
	}

	int ReleaseLock(Node* p, int mode)
	{
		switch(mode){
			case 1:
			p->lock.intentionLock->ISUnLock();
			break;
			case 2:
			p->lock.intentionLock->SUnLock();
			break;
			case 3:
			p->lock.intentionLock->IXUnLock();
			break;
			case 4:
			p->lock.intentionLock->XUnLock();
			break;
		};	
		return mode;
	
	
	}

	void FindAndLockAllParents(Node* p, int mode, map<Node*, int> &LockHoldings)
	{
		
		for(int i = 0; i< p->parents.size(); i++)
		{
			if(LockHoldings.find(p->parents[i]) == LockHoldings.end())
			{
				//Note: mode - 1 represents intention lock mode encoding for IS and IX resp
				LockHoldings[p->parents[i]] = AcquireLock(p->parents[i], mode - 1 );
			}
			else
			{
				int CurrentAcquiredLock = LockHoldings[p->parents[i]];

				int LockUpgradeStatus = ThreadLocalLockCompatibility[CurrentAcquiredLock-1][mode-2];
			
				if(LockUpgradeStatus != 0)
				{
					ReleaseLock(p->parents[i], CurrentAcquiredLock);
					
					LockHoldings[p->parents[i]] = AcquireLock(p->parents[i], LockUpgradeStatus);
					
				}
			}
		}
	
	}
};

#endif
