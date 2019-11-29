#ifndef NODE_H
#define NODE_H
#include <pthread.h>
#include <vector>
#include"IntentionLock.h"
#include"DomLock.h"
#include"HiFiLock.h"

//#include"Lock.h"
//#include"interval.h"
using namespace std;

class Node {
public:
	
//	bool IsExplored;
//	bool active;
//	bool parentUpdated;
//	int IsIx;
//	int refCounter;
//	double preNumber;
//	double postNumber;
//	Lock *nodeLock;
	
	int data;
	
	union
	{
		IL *intentionLock;
		DL *domLock;
		HFLock *hifiLock;
	} lock;

	vector <Node* >neighbour;
	vector <Node* >parents;
	vector<pair<double,double> >multiInterval;
	Node(int key, int lockType);


	void insertChild(Node*);
	
	void insertParent(Node*);

	int getOutDegree();

	int getInDegree();

	Node* getChildAt(int i);

	Node* getParentAt(int i);
};


int Node::getInDegree()
{
	return parents.size();
}
int Node::getOutDegree()
{
	return neighbour.size();
}
void Node::insertChild(Node *child)
{
	neighbour.push_back(child);
}

void Node::insertParent(Node *parent)
{
	parents.push_back(parent);
}

Node::Node(int key, int lockType)
{
//	int rc = pthread_rwlock_init(&rwlock, NULL);
	data=key;

	//Assign lock pointer to Intention lock type
	if(lockType == 4)
		lock.intentionLock = new IL();
	//Assign lock pointer to DomLock type
	else if(lockType == 5 || lockType == 7)
		lock.domLock = new DL();
	else if(lockType == 9)
		lock.hifiLock = new HFLock();
	
}

Node* Node::getChildAt(int i)
{
	return this->neighbour[i];
}

Node* Node::getParentAt(int i)
{
	return this->parents[i];
}
#endif
