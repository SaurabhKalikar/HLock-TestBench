#ifndef HiFiProtocol_H
#define HiFiProtocol_H
#include <math.h>
#include"Hierarchy.h"
#include"Node.h"
#include"LockProtocol.h"
#include"interval.h"

#define SheredHierarchical 0
#define ExclusiveHierarchical 1
#define SharedFine 2
#define ExclusiveFine 3
class HiFiProtocol: public LockProtocol
{
	public:

		HiFiProtocol(Hierarchy *H)
		{
			counter = 0;
	
			lp = new HiFiLockPool_SeqLock();
			
			AssignHiFiIntervals(H);
		}
		//default lock mode 4: Exclusive fine
		void LockAll(vector<Node*>&LockSet, int ThreadId)
		{
			vector<interval* > IntervalArray;	
			//Create request set as a vector of intervals
			for(int i = 0; i<LockSet.size(); i++)
			{
				double low, high;
				low = LockSet[i]->lock.hifiLock->getLowRange();
				high = LockSet[i]->lock.hifiLock->getHighRange();
				
				interval *it = new interval(low, high, ExclusiveFine);
				IntervalArray.push_back(it);
			}
			
			lp->Insert(&IntervalArray[0], IntervalArray.size(),  ExclusiveFine, ThreadId);
			PRINT(cout<<"HiFi lock all called"<<endl;)	
		}

		void LockAll(vector<Node*> &LockSet, vector<int> &LockMode, int ThreadId)
		{
			vector<interval* > IntervalArray;	
			//Create request set as a vector of intervals
			for(int i = 0; i<LockSet.size(); i++)
			{
				double low, high;
				low = LockSet[i]->lock.hifiLock->getLowRange();
				high = LockSet[i]->lock.hifiLock->getHighRange();
				
				interval *it = new interval(low, high, LockMode[i]);
				IntervalArray.push_back(it);
			}
			
			lp->Insert(&IntervalArray[0], IntervalArray.size(),  ExclusiveFine, ThreadId); //TODO: make lockpool generic for mixed requests
			PRINT(cout<<"HiFi lock all called"<<endl;)	
		}

		void LockAll(vector<Node*> &LockSet, int LockMode, int ThreadId)
		{
			vector<interval* > IntervalArray;	
			//Create request set as a vector of intervals
			for(int i = 0; i<LockSet.size(); i++)
			{
				double low, high;
				low = LockSet[i]->lock.hifiLock->getLowRange();
				high = LockSet[i]->lock.hifiLock->getHighRange();
				
				interval *it = new interval(low, high, LockMode);
				IntervalArray.push_back(it);
			}
			
			lp->Insert(&IntervalArray[0], IntervalArray.size(),  ExclusiveFine, ThreadId); //TODO: make lockpool generic for mixed requests
			PRINT(cout<<"HiFi lock all called"<<endl;)	
		
		}
		void UnLockAll(int ThreadId)
		{
			PRINT(cout<<"HiFi Unlock all called"<<endl;)	
		}

	private:
		//lock pool pointer
		LockPool* lp;
		int counter;
		
		void AssignHiFiIntervals(Hierarchy* H)
		{
			_ModifiedDFS(H->getHierarchyRoot());			
			_resetCustomFlag(H->getHierarchyRoot());

                	_bottomUp(H->getHierarchyRoot());
                	_resetCustomFlag(H->getHierarchyRoot());
		}
		
		void _ModifiedDFS(Node* node);
		void _resetCustomFlag(Node *ptr);
		void _mergeIntervals(Node *root);
		void _partitionLimitSend(double lowl, double upl ,Node *root);
		void _sendToParent(double ll, double ul, double l , double h, int ID,Node *root,int senderID );
		void _limitSend(double ll, double ul ,Node *root);
		void _sendUpdates(double ll,double ul,Node *root);
		void _bottomUp(Node *root);
};

void HiFiProtocol::_ModifiedDFS(Node* node)
{

	HFLock *x = node->lock.hifiLock;
	if(x->processed==true)
	{
		return;	
	}
	else if(x->processed==false && x->active==false)
	{

		//  processed and active = false , meaning this is the first time this node is being visited  
		x->active=true;
		counter++;
		if(node->getOutDegree() > 0)
			x->preNumber = -1;
		else
			x->preNumber = counter;


		for(int j=0; j < node->neighbour.size(); j++)
		{
			_ModifiedDFS(node->neighbour[j]);
		}
		x->active=false;
		x->processed=true;
		counter++;

		
		if(node->getOutDegree() > 0)
			x->postNumber = 1;
		else
			x->postNumber=counter;
	}

}

void HiFiProtocol::_resetCustomFlag(Node *ptr){
	if(ptr == NULL)
		return;

	for (int i = 0; i < ptr->neighbour.size(); i++)
	{	
		_resetCustomFlag(ptr->neighbour[i]);

	}
	ptr->lock.hifiLock->custom = false;

}

//
//
//*********************************************************
//Merges interval updates
//*********************************************************
void HiFiProtocol::_mergeIntervals(Node *root)
{


	HFLock *lock = root->lock.hifiLock;
	if(root->getOutDegree() == 1)
	{
		//single child , just copy 

		lock->preNumber=lock->lowUpdates[0].x;
		lock->lowlimit=lock->lowUpdates[0].y;

		lock->postNumber=lock->highUpdates[0].x;
		lock->uplimit=lock->highUpdates[0].y;
	}

	else
	{
		// more than 1 child case 
		int lowIndex=0;
		int highIndex=0;
		for(int i=1;i<lock->lowUpdates.size();i++)
		{
			if(lock->lowUpdates[i].x<lock->lowUpdates[lowIndex].x)
			{
				lowIndex=i;
			}
		}

		for(int i=1;i<lock->highUpdates.size();i++){
			if(lock->highUpdates[i].x>lock->highUpdates[highIndex].x){
				highIndex=i;
			}
		}

		lock->preNumber=lock->lowUpdates[lowIndex].x;
		lock->lowlimit=lock->lowUpdates[lowIndex].y;

		lock->postNumber=lock->highUpdates[highIndex].x;
		lock->uplimit=lock->highUpdates[highIndex].y;		
	}
}



//*********************************************************
//Partition intervals and sends to parents
//*********************************************************
void HiFiProtocol::_partitionLimitSend(double lowl, double upl ,Node *root){

	double dl,dh,ldash,hdash;
	// had to make 'np' a floating point number , there was loss in floating point precision of it was of type  int .
	double np = root->getInDegree();
	HFLock *lock = root->lock.hifiLock;	

	if(lowl == -1 && upl == -1){
		dl=1/np;
		dh=1/np;
		ldash=lock->preNumber-1;
		hdash=lock->postNumber+1;

	}
	else if(lowl==-1 && upl!=-1){
		dl=1/np;

		dh=(upl-lock->postNumber)/(2*np);

		ldash=lock->preNumber-1;
		hdash=(lock->postNumber+upl)/2;


	}
	else if(lowl!=-1 && upl==-1){
		dl=(lock->preNumber-lowl)/(2*np);
		dh=1/np;
		ldash=(lock->preNumber+lowl)/2;
		hdash=lock->postNumber+1;

	}
	else{
		dl=(lock->preNumber-lowl)/(2*np);
		dh=(upl-lock->postNumber)/(2*np);
		ldash=(lock->preNumber+lowl)/2;
		hdash=(lock->postNumber+upl)/2;
	}

	// for first Parent

	_sendToParent(lowl,lock->postNumber+(2*dh),ldash,lock->postNumber+dh, root->parents[0]->data, root->parents[0], root->data);

	//for parents ranging from 2 to np-1
	if(np>2){
		for(int i=1;i<(np-1);i++){
			_sendToParent(ldash+(i-2)*dl,lock->postNumber+(i-1)*dh,ldash+(i-1)*dl,lock->postNumber+i*dh,root->parents[i]->data,root->parents[i],root->data);

		}
	}

	// for np^th Parent 

	_sendToParent(ldash+(np-2)*dl, upl, ldash+(np-1)*dl, hdash, root->parents[np-1]->data, root->parents[np-1], root->data);


}

//*********************************************************
// Sends intervals to parent
//*********************************************************
void HiFiProtocol::_sendToParent(double ll, double ul, double l , double h, int ID,Node *root,int senderID )
{
	HFLock *lock = root->lock.hifiLock;
	lock->lowUpdates.push_back(updates(senderID,l,ll));
	lock->highUpdates.push_back(updates(senderID,h,ul));

}
//*********************************************************
// sends limits and intervals , single parent case 
//*********************************************************
void HiFiProtocol::_limitSend(double ll, double ul ,Node *root){
	
	HFLock *lock = root->lock.hifiLock;

	/* single parent method */
	if(ll==-1 && ul==-1){
		_sendToParent(ll, ul, lock->preNumber-1, lock->postNumber+1, root->parents[0]->data, root->parents[0],root->data);

	}
	else if(ul==-1 && ll!=-1){

		_sendToParent(ll,ul,(lock->preNumber+ll)/2,lock->postNumber+1,root->parents[0]->data,root->parents[0],root->data);

	}
	else if(ul!=-1 && ll==-1){
		_sendToParent(ll,ul,lock->preNumber-1,(lock->postNumber+ul)/2,root->parents[0]->data,root->parents[0],root->data);

	}
	else {
		_sendToParent(ll,ul,(lock->preNumber+ll)/2,(lock->postNumber+ul)/2,root->parents[0]->data,root->parents[0],root->data);

	}
}

//*********************************************************
// invokes partioning methods based on numbr of parents
//*********************************************************
void HiFiProtocol::_sendUpdates(double ll,double ul,Node *root){


	if(root->getInDegree() > 1){
		_partitionLimitSend(ll,ul,root);
	}
	else if(root->getInDegree() == 1){
		_limitSend(ll,ul,root);
	}

}
//*********************************************************
// bottom up method that invokes partitioning intervals
//*********************************************************

void HiFiProtocol::_bottomUp(Node *root){
	//. cout<<"inside _bottomUp for node "<<root->data<<endl;
	HFLock *lock = root->lock.hifiLock;
	if(root->getOutDegree() == 0){
		lock->lowlimit=-1;
		lock->uplimit=-1;

	}

	else{

		for(int i=0;i < root->neighbour.size();i++){
			if(root->neighbour[i]->lock.hifiLock->intervalProcessed == false){
				_bottomUp(root->neighbour[i]);

			}
		}

		_mergeIntervals(root);



	}
	_sendUpdates(lock->lowlimit,lock->uplimit,root);
	lock->intervalProcessed = true;
}

#endif
////*********************************************************
////Partition sends for structural updates
////*********************************************************
//void HiFiProtocol::_partitionLimitSendNew(double lowl, double upl ,Node *root){
//
//	double dl,dh,ldash,hdash;
//	// had to make 'np' a floating point number , there was loss in floating point precision of it was of type  int .
//	double np=root->parents.size();
//
//	if(lowl==-1 && upl==-1){
//		dl=1/np;
//		dh=1/np;
//		ldash=root->preNumber-1;
//		hdash=root->postNumber+1;
//
//	}
//	else if(lowl==-1 && upl!=-1){
//		dl=1/np;
//
//		dh=(upl-root->postNumber)/(2*np);
//
//		ldash=root->preNumber-1;
//		hdash=(root->postNumber+upl)/2;
//
//
//	}
//	else if(lowl!=-1 && upl==-1){
//		dl=(root->preNumber-lowl)/(2*np);
//		dh=1/np;
//		ldash=(root->preNumber+lowl)/2;
//		hdash=root->postNumber+1;
//
//	}
//	else{
//		dl=(root->preNumber-lowl)/(2*np);
//		dh=(upl-root->postNumber)/(2*np);
//		ldash=(root->preNumber+lowl)/2;
//		hdash=(root->postNumber+upl)/2;
//	}
//
//	// for first Parent
//
//	_sendToParentNew(lowl,root->postNumber+(2*dh),ldash,root->postNumber+dh,root->parents[0]->data,root->parents[0],root->data);
//
//	//for parents ranging from 2 to np-1
//	if(np>2){
//		for(int i=1;i<(np-1);i++){
//			_sendToParentNew(ldash+(i-2)*dl,root->postNumber+(i-1)*dh,ldash+(i-1)*dl,root->postNumber+i*dh,root->parents[i]->data,root->parents[i],root->data);
//
//		}
//	}
//
//	// for np^th Parent 
//
//	_sendToParentNew(ldash+(np-2)*dl,upl,ldash+(np-1)*dl,hdash,root->parents[np-1]->data,root->parents[np-1],root->data);
//
//
//}
//
//
//
////*********************************************
//// structural updates , single parent , interval updates
////*********************************************************
//void HiFiProtocol::_limitSendNew(double ll, double ul ,Node *root){
//	//. cout<<"inside _limitSend "<<endl;
//	/* single parent method */
//	if(ll==-1 && ul==-1){
//		_sendToParentNew(ll,ul,root->preNumber-1,root->postNumber+1,root->parents[0]->data,root->parents[0],root->data);
//
//	}
//	else if(ul==-1 && ll!=-1){
//
//		_sendToParentNew(ll,ul,(root->preNumber+ll)/2,root->postNumber+1,root->parents[0]->data,root->parents[0],root->data);
//
//	}
//	else if(ul!=-1 && ll==-1){
//		_sendToParentNew(ll,ul,root->preNumber-1,(root->postNumber+ul)/2,root->parents[0]->data,root->parents[0],root->data);
//
//	}
//	else {
//		_sendToParentNew(ll,ul,(root->preNumber+ll)/2,(root->postNumber+ul)/2,root->parents[0]->data,root->parents[0],root->data);
//
//	}
//}
//

////*********************************************************
//// structural update : sends intervals to parent
////*********************************************************
//void HiFiProtocol::_sendToParentNew(double ll, double ul, double l , double h, int ID,Node *root,int senderID ){
//	bool flag=false;
//
//	for(int i=0;i<root->lowUpdates.size() && flag!=true;i++){
//		if(root->lowUpdates[i].data==senderID){
//			root->lowUpdates[i].x=l;
//			root->lowUpdates[i].y=ll;
//			flag=true;
//
//		}
//
//	}
//	if(flag){
//		for(int i=0;i<root->highUpdates.size();i++){
//			if(root->highUpdates[i].data==senderID){
//				root->highUpdates[i].x=h;
//				root->highUpdates[i].y=ul;
//				break;
//
//
//			}
//
//		}
//
//	}
//	else{
//		root->lowUpdates.push_back(updates(senderID,l,ll));
//		root->highUpdates.push_back(updates(senderID,h,ul));
//	}
//
//}
////*********************************************************
//// calls partition method based on number of parents for structural updates
////*********************************************************
//void HiFiProtocol::_sendUpdatesNew(double ll,double ul,Node *root){
//
//
//	if(root->parents.size()>1){
//		_partitionLimitSendNew(ll,ul,root);
//	}
//	else if(root->parents.size()==1){
//		_limitSendNew(ll,ul,root);
//	}
//
//}
//
////***********************************
////***********************************
//void HiFiProtocol::_fDFS(Node *x){
//
//	if(x->processed==true){
//
//		//. cout<<"CROSS-EDGE: node with nodeID "<<x->data<<" has already been processed "<<endl;
//
//	}
//	else if(x->active==true){
//		//. cout<<"BACKEDGE: node with nodeID "<<x->data<<"is alredy active"<<endl;
//	}
//	else if(x->processed==false && x->active==false){
//
//
//		x->active=true;
//
//		for(int j=0;j<x->neighbour.size();j++){
//			_fDFS(x->neighbour[j]);
//		}
//
//		x->active=false;
//		x->processed=true;
//		st.push(x);
//
//
//	}
//
//
//}
//
//void HiFiProtocol::_bDFS(Node *root){
//	root->reverseProcessed=true;
//	cout<<"going to process transposeEdges of "<<root->data<<" number of elements in transposeEdge list "<<root->transposeEdge.size()<<endl;
//	for(int i=0;i<root->transposeEdge.size();i++){
//		if(root->transposeEdge[i]->reverseProcessed==false){
//			vv[vvtracker].push_back(root->transposeEdge[i]->data);
//			cout<<"node "<<root->data<<" is in the same component as "<<root->transposeEdge[i]->data<<endl;
//			collapseMap[root->transposeEdge[i]->data]=vvtracker+1;
//			_bDFS(root->transposeEdge[i]);
//		}
//	}
//}
//
//
////*********************************************************
//// creates cycle-collapsed graph
////*********************************************************
//void HiFiProtocol::_createCollapseGraph(){
//
//	for(int i=0;i<vv.size();i++){
//		//cout<<"doing for component number "<<i+1<<endl;
//		for(int j=0;j<vv[i].size();j++){
//
//			// have to push in  collapseArray[i+1].neighbour
//			Node* temp=Array[(vv[i])[j]];
//			//cout<<" got node "<<temp->data <<endl;
//			for(int k=0;k<temp->neighbour.size();k++){
//				int newc=collapseMap[temp->neighbour[k]->data];
//				//cout<<" new component is"<<newc<<endl;
//				if((i+1)!=newc){
//					if(find(collapseArray[i+1]->neighbour.begin(),collapseArray[i+1]->neighbour.end(),collapseArray[newc])!=collapseArray[i+1]->neighbour.end()){
//					}else{
//						cout<<"adding edge from component "<<i+1<<" to "<<newc<<endl;
//						collapseArray[i+1]->neighbour.push_back(collapseArray[newc]);
//						collapseArray[newc]->parents.push_back(collapseArray[i+1]);
//					}
//				}
//				else{
//					//cout<<" DAG bro DAG  , why try cycle when you DAG ??"<<endl;
//				}
//
//			}
//
//
//		}
//		cout<<endl;
//	}
//
//}
//
////*********************************************************
//// unit testing method
////*********************************************************
//void HiFiProtocol::_printCollapsedNodes()
//{
//	int i,j;
//	for(i=0;i<vv.size();i++){
//		cout<<"{ ";
//		for(j=0;j<vv[i].size();j++){
//			cout<<(vv[i])[j]<<" ";
//		}
//		cout<<"}"<<endl;
//	}
//
//}
//int HiFiProtocol::_bDFSStack(){
//	cout<<"beginning _bDFSStack"<<endl;
//
//	while(!st.empty()){
//
//		cout<<"stack not empty , have to pop and check "<<endl;
//
//		Node* ptr =st.top();
//		st.pop();
//		if(ptr->reverseProcessed==false){
//			vector<int> temp;
//			vv.push_back(temp);
//			vv[vvtracker].push_back(ptr->data);
//			collapseMap[ptr->data]=vvtracker+1;
//			_bDFS(ptr);
//			vvtracker++;
//		}
//
//	}
//
//	cout<<"Effective number of nodes "<<vvtracker<<endl;
//	cout<<"Effective number of nodes "<<vv.size()<<endl;
//	_printCollapsedNodes();
//	return vv.size();
//
//
//}
