#ifndef DomLockProtocol_H
#define DomLockProtocol_H
#include <math.h>
#include"Hierarchy.h"
#include"Node.h"
#include"LockProtocol.h"
#include"interval.h"

bool comp (interval *i,interval *j) { return (i->pre<j->pre); }
bool Vcomp (pair<int,int>a, pair<int,int>b){return (a.second < b.second);  }

class DomLockProtocol: public LockProtocol
{

	public:

		//Lock pool pointer
		LockPool* lp;

		//Constructor
		DomLockProtocol(Hierarchy *H)
		{
			counter = 0;
			
			lp = new LockPool_SeqLock(); 
			AssignDomLockIntervals(H->getHierarchyRoot());			
		}

		void LockAll(vector<Node*>&LockSet, int ThreadId)
		{
			double min = 0, max = 0;
			for(int i = 0; i<LockSet.size(); i++)
			{
				double low, high;
				low = LockSet[i]->lock.domLock->getLowRange();
				high = LockSet[i]->lock.domLock->getHighRange();
				if(min == 0 || low < min)
				{
					min = low;
				}
				if(max == 0 || high > max)
				{
					max = high;
				}
			}

			interval *it = new interval(min, max, 1);
			//Acquire lock
			lp->Insert(it, 1, ThreadId);
		}

		void LockAll(vector<Node*> &LockSet, vector<int> &LockMode, int ThreadId)
		{
			double min = 0, max = 0;
			int mode = 0;
			for(int i = 0; i<LockSet.size(); i++)
			{
				double low, high;
				low = LockSet[i]->lock.domLock->getLowRange();
				high = LockSet[i]->lock.domLock->getHighRange();
				if(min == 0 || low < min)
				{
					min = low;
				}
				if(max == 0 || high > max)
				{
					max = high;
				}

				mode = mode > LockMode[i]?mode:LockMode[i];
			}

			interval *it = new interval(min, max, mode);
			//Acquire lock
			lp->Insert(it, 1, ThreadId);
		}

		void LockAll(vector<Node*> &LockSet, int LockMode, int ThreadId)
		{
			double min = 0, max = 0;
			int mode = LockMode;
			for(int i = 0; i<LockSet.size(); i++)
			{
				double low, high;
				low = LockSet[i]->lock.domLock->getLowRange();
				high = LockSet[i]->lock.domLock->getHighRange();
				if(min == 0 || low < min)
				{
					min = low;
				}
				if(max == 0 || high > max)
				{
					max = high;
				}
			}

			interval *it = new interval(min, max, mode);
			//Acquire lock
			lp->Insert(it, 1, ThreadId);
		
		}
		void UnLockAll(int ThreadId)
		{
			lp->Delete(ThreadId);	
		}

	private:
		double counter;

		//Assigns nested intervals to the hierarchy maintaining subsumption property
		void AssignDomLockIntervals(Node* node)
		{
			// check if root is NULL
			if(node == NULL)
				return;
			
			DL *domlock = node->lock.domLock;

			//send updates to parent node if the node is already explored
			if(domlock->IsExplored())
				return;

			//if node is a leaf node or node is a part of some cycle, consider it as leaf node and assign intervals
			if(node->getOutDegree() == 0  || domlock->IsActive() == true)
			{
				counter+=1;
				domlock->setLowRange(counter);
				domlock->setHighRange(counter);
				
				//insert into vector for multiple intervals
				node->multiInterval.push_back(make_pair(counter,counter));
			}
			else
			{
				domlock->MarkActive(true);
				//call recursively modifiedDfS() function for all the child nodes. 
				//As we use binary tree, here for left and right pointers only.
				for(int i=0; i<node->getOutDegree(); i++)
				{		
					AssignDomLockIntervals(node->getChildAt(i));
				}
			}

			domlock->MarkExplored();
			domlock->MarkActive(false);

			//propagate interval information to all parents	
			for(int i=0;i<node->getInDegree();i++)
				UpdateParent(node->getParentAt(i), node);
			domlock->MarkParentUpdated(true);
			return;

		
		}

		//Propagate the intervals of child to its parents
		bool UpdateParent(Node *par, Node *ptr)
		{


			if(par == NULL || (par->lock.domLock)->RangeEquals(ptr->lock.domLock))
				return true;

			DL *parDL = par->lock.domLock;
			DL *ptrDL = ptr->lock.domLock;

			double parLow, parHigh, ptrLow, ptrHigh;
			parLow = parDL->getLowRange();
			parHigh = parDL->getHighRange();
			ptrLow = ptrDL->getLowRange();
			ptrHigh = ptrDL->getHighRange();

			//Expand the intervals of parent according to the interval of child node
			if(parLow == 0 && parHigh == 0)
			{
				parDL->setRange( ptrLow, ptrHigh);
			}
			else 
			{	
				if(parLow > ptrLow && ptrLow != 0)
					parDL->setLowRange(ptrLow);
				if(parHigh < ptrHigh && ptrHigh != 0)
					parDL->setHighRange(ptrHigh);	

			}
			//Special case for cycles
			if(parDL->IsParentUpdated() == true)
			{
				for(int i=0;i<par->getInDegree();i++)
					UpdateParent(par->getParentAt(i), par);
			}
			return true;

		}//EndOf UpdateParents()



};

#endif
