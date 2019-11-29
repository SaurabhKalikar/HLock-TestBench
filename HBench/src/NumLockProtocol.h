#ifndef NumLockProtocol_H
#define NumLockProtocol_H
#include <math.h>
#include"Hierarchy.h"
#include"Node.h"
#include"LockProtocol.h"
#include"interval.h"
#include"DomLockProtocol.h"

class NumLockProtocol: public DomLockProtocol
{
	public:

		int NumberOfLeafNodes;
	
		NumLockProtocol(int n, Hierarchy* H):DomLockProtocol(H)
		{
			NumberOfLeafNodes = n;
		}

		//Default lock mode 1: exclusive
		void LockAll(vector<Node*>&LockSet, int ThreadId)
		{
			vector<interval* > IntervalArray;	
			vector<interval* > OptimalLockSet;
		
			//Create request set as a vector of intervals
			for(int i = 0; i<LockSet.size(); i++)
			{
				double low, high;
				low = LockSet[i]->lock.domLock->getLowRange();
				high = LockSet[i]->lock.domLock->getHighRange();
				
				interval *it = new interval(low, high, 1);
				IntervalArray.push_back(it);
			}
			
			//Find optimal locking combination
			OptimalLockSet = GetOptimalLockingOptions(IntervalArray);


			lp->Insert(&OptimalLockSet[0], OptimalLockSet.size(),  1, ThreadId);
		}

		void LockAll(vector<Node*> &LockSet, vector<int> &LockMode, int ThreadId)
		{
			vector<interval* > IntervalArray;	
			vector<interval* > OptimalLockSet;
		
			//Create request set as a vector of intervals
			for(int i = 0; i<LockSet.size(); i++)
			{
				double low, high;
				low = LockSet[i]->lock.domLock->getLowRange();
				high = LockSet[i]->lock.domLock->getHighRange();
				
				interval *it = new interval(low, high, LockMode[i]);
				IntervalArray.push_back(it);
			}
			
			//Find optimal locking combination
			OptimalLockSet = GetOptimalLockingOptions(IntervalArray);


			lp->Insert(&OptimalLockSet[0], OptimalLockSet.size(),  1, ThreadId);
		
		
		}

		void LockAll(vector<Node*> &LockSet, int LockMode, int ThreadId)
		{
			vector<interval* > IntervalArray;	
			vector<interval* > OptimalLockSet;
		
			//Create request set as a vector of intervals
			for(int i = 0; i<LockSet.size(); i++)
			{
				double low, high;
				low = LockSet[i]->lock.domLock->getLowRange();
				high = LockSet[i]->lock.domLock->getHighRange();
				
				interval *it = new interval(low, high, LockMode);
				IntervalArray.push_back(it);
			}
			
			//Find optimal locking combination
			OptimalLockSet = GetOptimalLockingOptions(IntervalArray);


			lp->Insert(&OptimalLockSet[0], OptimalLockSet.size(),  1, ThreadId);
		
		}
		void UnLockAll(int ThreadId)
		{
			lp->Delete(ThreadId);	
		}

		//Generate locking options
		vector<interval*> GetOptimalLockingOptions( vector<interval *>&IntervalArray)
		{
			int LocalNoOfRequestedNode = IntervalArray.size();
			int accessType = 1;	
			sort(IntervalArray.begin(), IntervalArray.end(), comp);

			//Merge overlapping intervals inline function
			//***************************************************
			int top = 0;
			for(int i=0;i<IntervalArray.size();i++)
			{
				if(IntervalArray[top]->pre <= IntervalArray[i]->post && IntervalArray[top]->post >= IntervalArray[i]->pre)
				{	
					IntervalArray[top]->pre = min(IntervalArray[top]->pre, IntervalArray[i]->pre);
					IntervalArray[top]->post = max(IntervalArray[top]->post, IntervalArray[i]->post);

					IntervalArray[top]->mode = max(IntervalArray[top]->mode, IntervalArray[i]->mode);

				}
				else
					IntervalArray[++top] = IntervalArray[i];

			}	

			IntervalArray.resize(top+1);
			LocalNoOfRequestedNode = min (top+1 , LocalNoOfRequestedNode);
			//***************************************************		

			//Maintain the information of empty holes in the sorted list of requested nodes in a vector "Holes"
			vector<pair<int, int> >Holes(LocalNoOfRequestedNode - 1);

			//Maintains the total number of nodes locked by an option
			vector<int>K(LocalNoOfRequestedNode);
			K[0] = IntervalArray[LocalNoOfRequestedNode-1]->post - IntervalArray[0]->pre + 1;


			for(int i=0;i<IntervalArray.size() -1;i++)
			{
				Holes[i] = make_pair((IntervalArray[i+1]->pre - IntervalArray[i]->post), i);

			}

			sort(Holes.rbegin(), Holes.rend());
			//fill vector K with all entries of locked nodes by an option
			for(int i=1;i<LocalNoOfRequestedNode;i++)
			{
				K[i] = K[i-1] - Holes[i-1].first + 1;	
			}

			//Call to the function whih chooses the best locking option	
			int bestOption = PinPoint(K);

			sort(Holes.begin(),Holes.begin()+bestOption,Vcomp);	

			int L,R, LockMode1, LockMode2;
			vector<interval*>Pinned(bestOption + 1);

			L = IntervalArray[0]->pre;
			LockMode1 = IntervalArray[0]->mode;
			for(int i = 1; i<=bestOption; i++)
			{
				R = IntervalArray[Holes[i-1].second]->post;
				LockMode2 = IntervalArray[Holes[i-1].second]->mode;
				
				interval* iv1 = new interval(L,R, max(LockMode1, LockMode2));
				Pinned[i-1] = iv1;
				L = IntervalArray[Holes[i-1].second + 1]->pre;
			}	
			R = IntervalArray[LocalNoOfRequestedNode - 1]->post;
			LockMode2 = IntervalArray[LocalNoOfRequestedNode - 1]->mode;
			interval* iv1 = new interval(L,R, max(LockMode1, LockMode2));
			Pinned[bestOption] = iv1;


			return Pinned;


		}

		//Pinpoint the best locking option
		int PinPoint(vector<int>&K)
		{
			int n = NumberOfLeafNodes;
			int k;
			int K_Size = K.size();
			vector<double>Batches(K_Size);
			int minIndex;
			double minExecTime = 10000.0;
			for(int i_k = 0; i_k<K_Size;i_k++)
			{
				k = K[i_k];

				double perThreadTime = ((double)i_k*0.00004201) + 0.000551;
				double fr=1;
				int i,j;
				/*		for(i = n-k,j=n; i>n-k-k,j>n-k; i--,j--)
						{
				//nu[c++] = i;
				fr = fr * ((double)i/j);

				}
				Batches[i_k] = ceil(15 * (double)(1-fr));
				*/
				
				Batches[i_k] = 1 + ceil(15 * ((double)k/(4*n)));
				double newExec = Batches[i_k]*perThreadTime;
				if(minExecTime > newExec)
				{	
					minExecTime = newExec;
					minIndex = i_k;
				}
			}

			return minIndex;

		}
};

#endif
