#ifndef HiFiLock_H
#define HiFiLock_H

class updates{
public:
	int data;
	double x;
	double y;
	updates(int senderID,double a,double b){
		data=senderID;
		//.printf("constructor called by sender %d \n",data);

		x=a;
		y=b;
	}
};

class HFLock
{
	public:
		int LockMode;
		bool custom;
		bool IsExplored;
		bool active;


		bool processed;
		bool intervalProcessed;
		bool reverseProcessed;

		double preNumber;
		double postNumber;
		double uplimit;
		double lowlimit;
		vector<updates> lowUpdates;
		vector<updates> highUpdates;

		vector<int> cycleNodes;

		//vector <TreeNode* >transposeEdge;

		HFLock()
		{
			custom=false;
			IsExplored = false;
			active = false;
			uplimit = -1;
			lowlimit = 1;
			processed = false;
			intervalProcessed = false;
			reverseProcessed = false;
		}
		
		double getLowRange()
		{
			return preNumber;
		}

		double getHighRange()
		{
			return postNumber;
		}
};

#endif
