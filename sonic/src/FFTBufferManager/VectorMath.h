
#ifndef VectorMath_h
#define VectorMath_h

#include <stddef.h>

// Defines the interface for several vector math functions whose implementation will ideally be optimized.

namespace WebCore {

namespace VectorMath {

    typedef struct Complex {
        float  real;
        float  imag;
    } Complex;
    
// Vector scalar multiply and then add.
void vsma(const float* sourceP, int sourceStride, const float* scale, float* destP, int destStride, size_t framesToProcess);

void vsmul(const float* sourceP, int sourceStride, const float* scale, float* destP, int destStride, size_t framesToProcess);

// add by linyehui 2014-03-28 11:30
void ctoz (const Complex* sourceComplex, int sourceStride, float* destSplitComplexRe, float* destSplitComplexIm, int destStride, size_t framesToProcess);
void vsadd(const float* sourceP, int sourceStride, const float* scale, float* destP, int destStride, size_t framesToProcess);
void zvmags(const float* sourceRe, const float* sourceIm, int sourceStride, float* destP, int destStride, size_t framesToProcess);
void vdbcon(const float* sourceP, int sourceStride, const float* scale, float* destP, int destStride, size_t framesToProcess, unsigned int flag);
    
void vadd(const float* source1P, int sourceStride1, const float* source2P, int sourceStride2, float* destP, int destStride, size_t framesToProcess);

// Finds the maximum magnitude of a float vector.
void vmaxmgv(const float* sourceP, int sourceStride, float* maxP, size_t framesToProcess);

// Sums the squares of a float vector's elements.
void vsvesq(const float* sourceP, int sourceStride, float* sumP, size_t framesToProcess);

// For an element-by-element multiply of two float vectors.
void vmul(const float* source1P, int sourceStride1, const float* source2P, int sourceStride2, float* destP, int destStride, size_t framesToProcess);

// Multiplies two complex vectors.
void zvmul(const float* real1P, const float* imag1P, const float* real2P, const float* imag2P, float* realDestP, float* imagDestP, size_t framesToProcess);

// Copies elements while clipping values to the threshold inputs.
void vclip(const float* sourceP, int sourceStride, const float* lowThresholdP, const float* highThresholdP, float* destP, int destStride, size_t framesToProcess);

} // namespace VectorMath

} // namespace WebCore

#endif // VectorMath_h
