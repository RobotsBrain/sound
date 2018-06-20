#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "FFTBufferManager.h"
#include "VectorMath.h"


//#define SONIC_DEBUG

#ifdef SONIC_DEBUG
#define DBG(x)  x
#else
#define DBG(x)
#endif // SONIC_DEBUG



#define min(x,y) (x < y) ? x : y


inline uint64_t get_tick_count()
{
    timespec tsnow = {0};
    clock_gettime(CLOCK_MONOTONIC, &tsnow);
    return uint64_t(tsnow.tv_sec) * 1000 + tsnow.tv_nsec / 1000000;
}


FFTBufferManager::FFTBufferManager(unsigned long inNumberFrames) :
mNeedsAudioData(1),
mHasAudioData(0),
mFFTNormFactor(1.0/(2*inNumberFrames)),
mAdjust0DB(1.5849e-13),
m24BitFracScale(16777216.0f),
mFFTLength(inNumberFrames),
mNumberFrames(inNumberFrames),
mAudioBufferCount(inNumberFrames),
mAudioBufferCurrentIndex(0)
{
    mAudioBuffer = (float*) calloc(mNumberFrames,sizeof(float));
    mComplexResult = (kiss_fft_cpx*)calloc(mFFTLength, sizeof(kiss_fft_cpx));
    //mDspSplitComplex.realp = (float*) calloc(mFFTLength,sizeof(float));
    //mDspSplitComplex.imagp = (float*) calloc(mFFTLength, sizeof(float));
    mSplitComplexRe = (float*) calloc(mFFTLength,sizeof(float));
    mSplitComplexIm = (float*) calloc(mFFTLength,sizeof(float));
    mTmpData =(float*) calloc(mFFTLength, sizeof(float));
    
    //mSpectrumAnalysis = vDSP_create_fftsetup(mLog2N, kFFTRadix2);
    mKissFFTCfg = kiss_fftr_alloc(mFFTLength, 0, NULL, NULL);
}

FFTBufferManager::~FFTBufferManager()
{
    //vDSP_destroy_fftsetup(mSpectrumAnalysis);
    kiss_fft_cleanup();
    free(mAudioBuffer);
    //free (mDspSplitComplex.realp);
    //free (mDspSplitComplex.imagp);
    free (mSplitComplexRe);
    free (mSplitComplexIm);
	free(mComplexResult);
    free(mTmpData);
}

template <class InIt, class OutIt>
void buf_short_to_float(InIt first, InIt last, OutIt out)
{
    for (InIt it = first; it != last; ++it)
    {
        double v = (double)(*it) / 32768.0;     // normalize
        if (v > 1) v = 1;
        if (v < -1) v = -1;

        *out++ = (float)v;
    }
}

int FFTBufferManager::GrabAudioData(const short* pcmData, int sample_num)
{
	int remainCount = mAudioBufferCount - mAudioBufferCurrentIndex;
	if (remainCount <= 0)
    {
        return 0;
    }

	unsigned long countToCopy = min(remainCount, sample_num);
	
	short const* pcm_s16 = (short const*)pcmData;
	float* pcm_float = mAudioBuffer+mAudioBufferCurrentIndex;
	buf_short_to_float(pcm_s16, pcm_s16 + countToCopy, pcm_float);

	mAudioBufferCurrentIndex += countToCopy;
	if (mAudioBufferCurrentIndex >= (int)mAudioBufferCount)
	{
        mFilter.InplaceFilter(mAudioBuffer, mAudioBufferCount);
		mHasAudioData = 1;
		mNeedsAudioData = 0;
	}

	return countToCopy;
}

bool FFTBufferManager::Reset()
{
    mFilter.Reset();
    mAudioBufferCurrentIndex = 0;
    mHasAudioData = 0;
    mNeedsAudioData = 1;
    return true;
}

bool FFTBufferManager::ComputeFFT(int32_t *outFFTData)
{
	if (mHasAudioData)
	{
        DBG(uint64_t t0 = get_tick_count());

        //kiss_fft_stride(mKissFFTCfg, (const kiss_fft_cpx *)mAudioBuffer, (kiss_fft_cpx *)mAudioBuffer, 1);
		kiss_fftr(mKissFFTCfg, (kiss_fft_scalar*)mAudioBuffer, mComplexResult);

        //Generate a split complex vector from the real data
        //vDSP_ctoz((COMPLEX *)mAudioBuffer, 2, &mDspSplitComplex, 1, mFFTLength);
        WebCore::VectorMath::ctoz((WebCore::VectorMath::Complex *)mComplexResult, 2, mSplitComplexRe, mSplitComplexIm, 1, mFFTLength);
        
        //Take the fft and scale appropriately
        //vDSP_fft_zrip(mSpectrumAnalysis, &mDspSplitComplex, 1, mLog2N, kFFTDirection_Forward);
        
        //vDSP_vsmul(mDspSplitComplex.realp, 1, &mFFTNormFactor, mDspSplitComplex.realp, 1, mFFTLength);
        WebCore::VectorMath::vsmul(mSplitComplexRe, 1, &mFFTNormFactor, mSplitComplexRe, 1, mFFTLength);
        
        //vDSP_vsmul(mDspSplitComplex.imagp, 1, &mFFTNormFactor, mDspSplitComplex.imagp, 1, mFFTLength);
        WebCore::VectorMath::vsmul(mSplitComplexIm, 1, &mFFTNormFactor, mSplitComplexIm, 1, mFFTLength);

        //Zero out the nyquist value
        mSplitComplexIm[0] = 0.0;
        
        //Convert the fft data to dB
        //vDSP_zvmags(&mDspSplitComplex, 1, tmpData, 1, mFFTLength);
        WebCore::VectorMath::zvmags(mSplitComplexRe, mSplitComplexIm, 1, mTmpData, 1, mFFTLength);
        
        //In order to avoid taking log10 of zero, an adjusting factor is added in to make the minimum value equal -128dB
        //vDSP_vsadd(tmpData, 1, &mAdjust0DB, tmpData, 1, mFFTLength);
        WebCore::VectorMath::vsadd(mTmpData, 1, &mAdjust0DB, mTmpData, 1, mFFTLength);
        
        float one = 1;
        //vDSP_vdbcon(tmpData, 1, &one, tmpData, 1, mFFTLength, 0);
        WebCore::VectorMath::vdbcon(mTmpData, 1, &one, mTmpData, 1, mFFTLength, 0);
        
        //Convert floating point data to integer (Q7.24)
        //vDSP_vsmul(tmpData, 1, &m24BitFracScale, tmpData, 1, mFFTLength);
        WebCore::VectorMath::vsmul(mTmpData, 1, &m24BitFracScale, mTmpData, 1, mFFTLength);
        
        for (unsigned long i=0; i<mFFTLength/2; ++i)
        {
            outFFTData[i] = (signed long) mTmpData[i];
        }

        mHasAudioData = 0;
		mNeedsAudioData = 1;
		mAudioBufferCurrentIndex = 0;

        DBG(uint64_t t1 = get_tick_count());
        DBG(printf("compute fft time(%llums)\n", t1 - t0));
		return true;
	}

	return false;
}

