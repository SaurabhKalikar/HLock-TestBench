#ifndef TD_H
#define TD_H
class Hierarchy;
class Parameters;
class Operations;
class threadData
{
	public:
		Hierarchy *pH;
		Parameters *pC;
		Operations *op;
		int threadID;

};
#endif
