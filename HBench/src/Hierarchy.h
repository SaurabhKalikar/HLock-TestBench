#ifndef HIERARCHY_H
#define HIERARCHY_H

#include"threadData.h"
#include"Node.h"
#include"IntentionLock.h"
#include"DomLock.h"
#include<vector>

using namespace std;
class Hierarchy
{
	public:

		int size;
		
		Node* pRoot;

		Node** Array;
		vector<Node*>leafList; 

		void* LP;	
		//Constructor
		Hierarchy()
		{
			size = 0;
			pRoot = NULL;
			
		}
		void PrintHierarchy(Node*,vector<bool>&);

		void ParallelTask(threadData *pTD);

		int getHierarchySize()
		{
			return size;
		}

		void setHierarchySize(int s)
		{
			size = s;
		}

		Node* getHierarchyRoot()
		{
			return pRoot;
		}

		void setHierarchyRoot(Node* root)
		{
			pRoot = root;
		}
};
//Prints the hierarchy in edgelist format to verify the input
void Hierarchy::PrintHierarchy(Node* root, vector<bool>&visited)
{
	visited[root->data] = true;
	
	for(int i=0; i<root->getOutDegree(); i++)
	{
		Node* child = root->neighbour[i];
		cout<<root->data<<" "<<child->data<<endl;
		if(!visited[child->data])	
			PrintHierarchy(child, visited);
	}
}

void Hierarchy::ParallelTask(threadData *pTD)
{
	printf("Thread ID: %d, Hierarchy Size %d, lock mode: %d test lock inhr %f\n",pTD->threadID, getHierarchySize(), pTD->pC->getLockType(), pRoot->lock.domLock->getLowRange());

}
#endif
