#include<iostream>
#include"Lock.h"

using namespace std;
int main()
{
	IntentionLock *il = new IntentionLock();



	il->IXLock();
	il->IXLock();
	il->IXLock();
	il->ISLock();
	il->ISLock();
	il->PrintStat();
	il->ISLock();
	il->IXUnLock();
	il->IXUnLock();
	il->IXUnLock();
	il->SLock();
	il->PrintStat();
	return 0;

}
