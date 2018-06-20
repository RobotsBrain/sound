#include "Filter.h"


const float DCRejectionFilter::kDefaultPoleDist = 0.975f;

DCRejectionFilter::DCRejectionFilter(float poleDist)
{
	Reset();
}

void DCRejectionFilter::Reset()
{
	mY1 = mX1 = 0;	
}

void DCRejectionFilter::InplaceFilter(float* ioData, int numFrames)
{
	for (int i=0; i < numFrames; i++)
	{
        float xCurr = ioData[i];
		ioData[i] = ioData[i] - mX1 + (kDefaultPoleDist * mY1);
        mX1 = xCurr;
        mY1 = ioData[i];
	}
}

