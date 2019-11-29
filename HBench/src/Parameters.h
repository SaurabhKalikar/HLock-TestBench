#ifndef PARAMETERS_H
#define PARAMETERS_H
#include<stdlib.h>
#include<iostream>
#include<string>
#include<getopt.h>
using namespace std;

class Parameters
{
	private:		
		int N; //-n
		int numThreads; //-t
		int caseParameter; //-l
		int NoOfRequestedNode; //-q
		int distribution; //-s
		char CSSize; //-c
		int option; //-k
		string edgeListFile;
		char hierarchyType;
		int treeDegree;
		
		void SetDefaultParameters();	
		void ParseInput(int argc, char *argv[]);
	public:
		//Returns total number of nodes present in Data structure
		int getMaxSize()
		{	
			return N;
		}		
		void setMaxSize(int m)
		{
			N = m;
		}

		//Returns number of threads 
		int getNumThreads()
		{
			return numThreads;
		}
		//Returns locking technique
		int getLockType()
		{
			return caseParameter;
		}
		//Returns number of requested nodes
		int getQuerySize()
		{
			return NoOfRequestedNode;
		}
		//Returns skewness distribution of queries
		int getSkewness()
		{
			return distribution;
		}
		//Ruturns critical section size "Small: s, Medium: m, Large:l" for experiments
		char getCSSize()
		{
			return CSSize;
		}
		//Returns numebr of locks to be acquired, Use ONLY for testing optimal locking behaviour
		int getOptionNumber()
		{
			return option;
		}

		string getEdgeListFileName()
		{
			return edgeListFile;
		}
		
		char getHierarchyType()
		{
			return hierarchyType;
		}

		int getTreeDegree()
		{
			return treeDegree;
		}

		//Prints input parameters on stdout
		void PrintInputParameters();
		//Constructure
		Parameters(int argc, char *argv[])
		{	
			SetDefaultParameters();	
			ParseInput(argc, argv);
			PrintInputParameters();
		}
};

void Parameters::SetDefaultParameters()
{
	N = 1000;
	numThreads = 1;
	caseParameter = 7;
	NoOfRequestedNode = 2;
	distribution = 1;
	CSSize = 'm';
	option = 0;
	hierarchyType = 't';
	treeDegree = 2;
}

void Parameters::ParseInput(int argc, char *argv[])
{
	int c;
	while((c = getopt(argc, argv, "n:t:l:q:s:c:k:f:h:d:H")) != -1)
	{	
	
		
		switch(c)
		{
			case 'n':
				N = atoi(optarg);
				break;
			case 't':
				numThreads = atoi(optarg);
				break;
			case 'l':
				caseParameter = atoi(optarg);
				break;
			case 'q':
				NoOfRequestedNode = atoi(optarg);
				break;
			case 's':
				distribution = atoi(optarg);
				break;
			case 'c':
				CSSize = optarg[0];
				break;
			case 'k':
				option = atoi(optarg);
				break;
			case 'f':
				edgeListFile = optarg;
				break;
			case 'h':
				hierarchyType = optarg[0];
				break;
			case 'd':
				treeDegree = atoi(optarg);
				break;
			case 'H':
				cout<<"Command-line flags:"<<endl<<
				"-n: The number of nodes in the data strecture"<<endl<<
				"-t: The number of paralle threads"<<endl<<
				"-l: Yhe choice of locking protocol, [4: IL, 5: DL, 7: NL"<<endl<<
				"-s: The skewness in the query nodes"<<endl<<
				"-q: The number of nods in the request set"<<endl<<
				"-c: The size of critical section"<<endl<<
				"-k: The choice of specific locking option"<<endl<<
				"-f: Input file name, if input hierarchy to be built from external file (edge-list format)"
				"-h: Hierarchy type, [t,g,x]"<<endl<<
				"-d: The degree of k-ary tree"<<endl;
				
		};
	}
}

void Parameters::PrintInputParameters()
{
	cout<<"\n ************** Benchmark Parameters ****************\n\n";	
	cout<<"Data Structure Size:"<<getMaxSize()<<endl;

	cout<<"Number of parallel threads: "<<getNumThreads()<<endl;

	cout<<"Locking technique: "<<getLockType()<<endl;

	cout<<"Number of nodes to be locked: "<<getQuerySize()<<endl;

	cout<<"Skewness distrubution: "<<getSkewness()<<endl;

	cout<<"Critical section size: "<<getCSSize()<<endl;

	cout<<"Number of locks to be acquired: "<<getOptionNumber()<<endl;
	
	cout<<"Hierarchy type: "<<getHierarchyType()<<endl;

	cout<<"Degree of tree node: "<<getTreeDegree()<<endl;
}
#endif
