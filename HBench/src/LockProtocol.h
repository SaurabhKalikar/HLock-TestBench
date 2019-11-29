#ifndef LOCK_PROTOCOL_H
#define LOCK_PROTOCOL_H
#include<vector>
#include"Node.h"
#include"interval.h"

class LockProtocol
{
	public:
		void GenerateIntervalArrayOfLockSet(vector<Node*>&LockSet, vector<interval*> &IntervalArray, vector<int> &mode)
		{
		
		}
		void GenerateIntervalArrayOfLockSet(vector<Node*>&LockSet, vector<interval*> &IntervalArray, int mode)
		{
		
		
		}

		virtual void LockAll(vector<Node*> &LockSet, int ThreadId)
		{
		
		}

		virtual void UnLockAll(int ThreadId)
		{
		}
		
		virtual void LockAll(vector<Node*> &LockSet, vector<int> &LockMode, int ThreadId)
		{
		}

		virtual void LockAll(vector<Node*> &LockSet, int LockMode, int ThreadId)
		{
		
		}

		virtual void LockAll(Node* node, int LockMode)
		{
		
		}

};

#endif
