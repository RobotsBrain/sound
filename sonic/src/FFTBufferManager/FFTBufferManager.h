

#include "../kiss_fft/kiss_fftr.h"
#include "Filter.h"

class FFTBufferManager
{
public:
	FFTBufferManager(unsigned long inNumberFrames);
	~FFTBufferManager();
	
	int					GrabAudioData(const short* pcmData, int sample_num);
	bool				ComputeFFT(int32_t *outFFTData);
  	bool                Reset();

private:
	volatile int32_t	mNeedsAudioData;
	volatile int32_t	mHasAudioData;
	
    kiss_fftr_cfg       mKissFFTCfg;
    kiss_fft_cpx*		mComplexResult;
    float*              mSplitComplexRe;
    float*              mSplitComplexIm;
    float*              mTmpData;
    
    float               mFFTNormFactor;
    float               mAdjust0DB;
    float               m24BitFracScale;
	
    unsigned long       mFFTLength;
	float*              mAudioBuffer;
	unsigned long		mNumberFrames;
	unsigned long		mAudioBufferCount;
	int32_t				mAudioBufferCurrentIndex;

    DCRejectionFilter   mFilter;
};
