#ifndef BENCHMARK_H
#define BENCHMARK_H

#include"Parameters.h"
#include"Hierarchy.h"
#include"Threads.h"
#include"Node.h"
#include"DomLockProtocol.h"
#include"NumLockProtocol.h"
#include"IntentionLockProtocol.h"
#include"HiFiProtocol.h"
//#include"LockProtocol.h"


#include<fstream>
#include<vector>
#include<set>
#include <cstdlib>
class Benchmark
{
	public:
		Parameters *CommandLineInputs;
		Hierarchy *H;	
		Thread *T;
		Node** Array;	
		vector<Node*> leafList;	
		LockProtocol *protocol;
		Operations *op;


		Benchmark(int argc, char *argv[])
		{		
			CommandLineInputs = new Parameters(argc, argv);
			
			H = new Hierarchy();

//			T = new Thread(this->op);

			PRINT(cout<<"Benchmark start...\n";)	
		}

		//Initialize and build data structure
		void Init();

		//Build data structure using edgelist file
		void BuildDataStructure();

		//Build data structure with DAG structure
		void BuildDataStructure(int size);

		//Build tree data structure 
		void BuildDataStructure(int size, int nary);

		void BuildAuxillaryLeafList();

		//Generate dot file for pictorial representation
		void GenerateDotFile();

		//Run benchmark according to Parameters
		void run();

};

void Benchmark::Init()
{
	//Build hierarchy
	if(CommandLineInputs->getHierarchyType() == 'f')	
		BuildDataStructure();	
	else if(CommandLineInputs->getHierarchyType() == 'g')
		BuildDataStructure(CommandLineInputs->getMaxSize()); 
	else
		BuildDataStructure(CommandLineInputs->getMaxSize(), CommandLineInputs->getTreeDegree());	

	//------------------------------------------------------------//
	//Create separate list of leaf nodes


	BuildAuxillaryLeafList();

//	Init lock protocol
//	4: Iintention Locks
//	5: DomLock
//	7: NumLock
	
	if(CommandLineInputs->getLockType() == 4)
	{	
		PRINT(cout<<"Intention lock"<<endl;)
		protocol = new IntentionLockProtocol(H, CommandLineInputs->getNumThreads());
	}
	else if(CommandLineInputs->getLockType() == 5)
	{
		PRINT(cout<<"DomLock lock "<<CommandLineInputs->getLockType()<<endl;)
		protocol = new DomLockProtocol(H);
	}	
	else if(CommandLineInputs->getLockType() == 7)
	{
		PRINT(cout<<"NumLock"<<endl;)
		PRINT(cout<<"NL "<<CommandLineInputs->getLockType()<<endl;)
		protocol = new NumLockProtocol(this->leafList.size(), H);		
	}
	else if(CommandLineInputs->getLockType() == 9)
	{
		PRINT(cout<<"HiFi lock "<<CommandLineInputs->getLockType()<<endl;)
		protocol = new HiFiProtocol(H);
	}	
	else 
	{
		PRINT(cout<<"\nLock protocol id not valid\n";)
	}
	//Initialise operations with the adopted locking protocol
	this->op = new Operations(protocol);
	T = new Thread(this->op);
	PRINT(cout<<"Benchmark initialization...\n";)

}
/*	
Creates a hierarchy according to edge list given input file
*/
void Benchmark::BuildDataStructure()
{
	int MAX_NODES;
	int source, dest, lockType;
	
	//Adopted locking technique-
	//4: Intention lock
	//5: DomLock
	lockType = CommandLineInputs->getLockType(); 
	//open file containing datastructure in edgelist format
	ifstream FP(CommandLineInputs->getEdgeListFileName());
	
	//read from file: total number of nodes in the data-structure
	FP>>MAX_NODES;
 	
	//Create and allocate N nodes 
	Array = new Node*[MAX_NODES + 1];
	for(int i = 1; i <= MAX_NODES; i++)
		Array[i] = new Node(i, lockType);
	
	//create links according to edge list source->dest
	while(FP>>source)
	{
		FP>>dest;
		Array[source]->insertChild(Array[dest]);
		Array[dest]->insertParent(Array[source]);
	}
	
	H->setHierarchyRoot(Array[1]);
	H->setHierarchySize(MAX_NODES);
	CommandLineInputs->setMaxSize(MAX_NODES);
}

/*	
Creates a random hierarchy with Directed Acyclic Graph structure.
For every node, all neighbours are at same height and linked to
next level.
*/
void Benchmark::BuildDataStructure(int size)
{
	int MAX_NODES = size, idx = 0;

	//Adopted locking technique-
	//4: Intention lock
	//5: DomLock
	int lockType = CommandLineInputs->getLockType(); 
	
	vector<vector<Node*> >V(1,vector<Node*>(1, new Node(++idx, lockType)));

	do{
		int current_size = V.size();
		int current_level = V[current_size -1].size();
		int next_level = 2 * current_level + rand()%5;
		
		V.push_back(vector<Node*>(next_level));
		for(int i = 0; i < next_level; i++)
		{
			V[current_size][i] = new Node(++idx, lockType);
		}
		//connect nodes from current level to next level
		for(int i = 0; i < current_level; i++)
		{
			Node* source = V[current_size -1][i];
			int out_degree = 2 + min(25 * (rand()%10), rand() % next_level);
			set<int> chooseNeighbors;
			while(out_degree--)
			{
				chooseNeighbors.insert(rand()%next_level);
			}
			set<int>::iterator it;
			for(it = chooseNeighbors.begin(); it !=chooseNeighbors.end(); it++)
			{
				Node* dest = V[current_size][*it];
				source->insertChild(dest);
				dest->insertParent(source);		
			}
		}
	}while(idx<MAX_NODES);

	Array = new Node*[idx + 1];
	int index = 0;	
	for(int i = 0; i<V.size(); i++)
		for(int j = 0; j < V[i].size(); j++)
		{
			Node* source = V[i][j];
			Array[++index] = source;
			if(source->data != 1 && source->getInDegree() == 0)
			{
				Node* parent = V[i-1][rand()%(V[i-1].size())];
				source->insertParent(parent);
				parent->insertChild(source); 	
			}
		}	

	H->setHierarchyRoot(Array[1]);
	H->setHierarchySize(idx);


	//Final DAG may contain more number of nodes than that of mentioned in commandline args.
	CommandLineInputs->setMaxSize(idx);
}

/*
Creates an N-ary tree with degree of each node is "treeDegree"
provided as an input argument. Default degree is 2, i.e., binary tree
*/
void Benchmark::BuildDataStructure(int size, int treeDegree)
{
	int MAX_NODES = size;
	int dest = 2;
	//Adopted locking technique-
	//4: Intention lock
	//5: DomLock
	int lockType = CommandLineInputs->getLockType(); 
	//create all tree nodes at once and store their addresses in "Array"	
	Array = new Node*[MAX_NODES + 1];
	for(int i = 1; i<=MAX_NODES;i++)
		Array[i] = new Node(i, lockType);
	
	//create links according to N_ary
	for(int i=1;(i<=MAX_NODES && dest<=MAX_NODES);i++)
		for(int j=0; (j<treeDegree && dest<=MAX_NODES); j++)
		{
			Array[i]->insertChild(Array[dest]);
			Array[dest]->insertParent(Array[i]);
			dest++;
		}

	H->setHierarchyRoot(Array[1]);
	H->setHierarchySize(MAX_NODES);

	PRINT(cout<<"SIZE="<<H->getHierarchySize();)
}

/*
Create auxillary list of leaf nodes in the hierarchy
*/
void Benchmark::BuildAuxillaryLeafList()
{
	PRINT(cout<<"leaf list";);
	for(int i=1; i<=H->getHierarchySize(); i++)
	{
		if(Array[i]->getOutDegree() == 0)
		{
			leafList.push_back(Array[i]);
			PRINT(cout<<Array[i]->data<<" ";);
		}
	}
	//-----------Copy reference of auxillary ds into Hierarchy object---//
	H->Array = this->Array;
	H->leafList = this->leafList;
}

//Create parallel threads and run concurrent operations on hierarchy
void Benchmark::run()
{
	PRINT(cout<<"Execution starts....\n";)
	T->CreateThreads(CommandLineInputs, H);


}

void Benchmark::GenerateDotFile()
{
	ofstream fp("img.dot");	
	fp<<"digraph G {"<<endl;
	for(int i=1; i<=H->getHierarchySize(); i++)
	{

		for(int j=0; j< Array[i]->getOutDegree(); j++)
		{
			string source = "";
			string destination = "";
			//DL *lock = Array[i]->lock.domLock;
			HFLock *lock = Array[i]->lock.hifiLock;
			source+= to_string((int)lock->getLowRange())+","+to_string((int)lock->getHighRange());
			
			//lock = Array[i]->getChildAt(j)->lock.domLock;
			lock = Array[i]->getChildAt(j)->lock.hifiLock;
			destination+=to_string((int)lock->getLowRange())+","+to_string((int)lock->getHighRange());

			fp<<"\""<<i<<":["<<source<<"]\"->\""<<Array[i]->getChildAt(j)->data<<":["<<destination<<"]\""<<endl;
		}

	}
	fp<<"}";
	fp.close();
	system("dot -Teps img.dot > output.eps");
	system("open output.eps");
}
#endif
