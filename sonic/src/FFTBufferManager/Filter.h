#ifndef __DCJECTION_FILTER_H__
#define __DCJECTION_FILTER_H__

class DCRejectionFilter
{
public:
	DCRejectionFilter(float poleDist = DCRejectionFilter::kDefaultPoleDist);
    
	void InplaceFilter(float* ioData, int numFrames);
	void Reset();
    
protected:
	
	// State variables
	float mY1;
	float mX1;
	
	static const float kDefaultPoleDist;
};

#endif // __DCJECTION_FILTER_H__

