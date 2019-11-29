#ifndef DL_H
#define DL_H
#include<iostream>
using namespace std;

class DL
{
	private:
		bool Explored;
		bool active;
		bool parentUpdated;
		double preNumber;
		double postNumber;
		int LockMode;

	public:
		DL()//initLock()
		{
			Explored = false;
			active = false;
			parentUpdated = false;
			preNumber = 0;
			postNumber = 0;

		}

		double getLowRange()
		{
			return preNumber;
		}

		void setLowRange(double val)
		{
			preNumber = val;
		}
		
		double getHighRange()
		{
			return postNumber;
		}

		void setHighRange(double val)
		{
			postNumber = val;	
		}
		
		void setRange(int low, int high)
		{
			preNumber = low;
			postNumber = high;
		}
		bool IsParentUpdated()
		{
			if(parentUpdated == true)
				return true;
			return false;
		}

		void MarkParentUpdated(bool val)
		{
			parentUpdated = val;
		}

		bool IsExplored()
		{
			return Explored;	
		}

		void MarkExplored()
		{
			Explored = true;
		}

		bool IsActive()
		{
			return active;
		}

		void MarkActive(bool val)
		{
			active = val;
		}

		bool RangeEquals(DL* obj)
		{
			if(this->preNumber == obj->preNumber && this->postNumber == obj->postNumber)
				return true;
			return false;
		}
};
#endif
