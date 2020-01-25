#include <glm/glm.hpp>

typedef glm::vec2 float2;

int mod(int a, int b)
{
    int n = int(a/b);
    a -= n*b;
    
	if (a < 0)
        a += b;

    return a;
}

float2 cplxMul(const float2 &cmplxA, const float2 &cmplxB)
{
	return float2(cmplxA.x*cmplxB.x - cmplxA.y*cmplxB.y, cmplxA.x*cmplxB.y + cmplxA.y*cmplxB.x);
}

#define PI 3.14159265359f
#define SQRT_1_2 0.707106781188f
#define COS_8 0.923879532511f // cos(Pi/8)
#define SIN_8 0.382683432365f // sin(Pi/8)

// mul_p*q*(a) returns a*EXP(-I*PI*P/Q)
float2 mul_p0q1(float2 a) { return a; }

float2 mul_p0q2(float2 a) { return a; }
float2 mul_p1q2(float2 a) { return float2(a.y,-a.x); }

float2 mul_p0q4(float2 a) { return a; }
float2 mul_p1q4(float2 a) { return float2(SQRT_1_2*(a.x+a.y),SQRT_1_2*(-a.x+a.y)); }
float2 mul_p2q4(float2 a) { return float2(a.y,-a.x); }
float2 mul_p3q4(float2 a) { return float2(SQRT_1_2*(-a.x+a.y),SQRT_1_2*(-a.x-a.y)); }

float2 mul_p0q8(float2 a) { return a; }
float2 mul_p1q8(float2 a) { return cplxMul(float2(COS_8,-SIN_8),a); }
float2 mul_p2q8(float2 a) { return float2(SQRT_1_2*(a.x+a.y),SQRT_1_2*(-a.x+a.y)); }
float2 mul_p3q8(float2 a) { return cplxMul(float2(SIN_8,-COS_8),a); }
float2 mul_p4q8(float2 a) { return float2(a.y,-a.x); }
float2 mul_p5q8(float2 a) { return cplxMul(float2(-SIN_8,-COS_8),a); }
float2 mul_p6q8(float2 a) { return float2(SQRT_1_2*(-a.x+a.y),SQRT_1_2*(-a.x-a.y)); }
float2 mul_p7q8(float2 a) { return cplxMul(float2(-COS_8,-SIN_8),a); }

float2 twiddle(float2 a, int k, float alpha)
{
  float sn = sin(k*alpha);
  float cs = cos(k*alpha);
  return cplxMul(a,float2(cs,sn));
}

void initData(float2* data, int nX, int nZ, int nT)
{
	for(int z=0; z<nT; ++z)
	{
		for(int j=0; j<nZ; ++j)
		{
			for(int i=0; i<nX; ++i)
			{
				data[i+nX*j+nX*nZ*z].x = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				data[i+nX*j+nX*nZ*z].y = 0.0f;
			}
		}
	}
}

void radix2(float2* dataIn, float2* dataOut, int nSample, int offset, int dir)
{
	// out of place radix-2
	int radixSize = 2;
	int nThread = nSample / radixSize;
	int logRadixSize = (int)(log((float)radixSize) / log(2.0f));
	for(int iStage=0; iStage<(int)(log((float)nSample) / log((float)radixSize)); ++iStage)
	{
		int subSeqLength = (1<<(logRadixSize*iStage));
		for(int iThread=0; iThread<nThread; ++iThread)
		{
			int inSeqIdx = iThread&(subSeqLength-1);
			int outIdx = ((iThread-inSeqIdx)<<1) + inSeqIdx;
			float alpha = dir * -PI * inSeqIdx / (1.0f*subSeqLength);

			// read and twiddle input
			float2 u0 = dataIn[iThread*offset];
			float2 u1 = twiddle(dataIn[(iThread+nThread)*offset], 1, alpha);

			// 1x DFT-2 and write
			dataOut[outIdx*offset] = u0 + u1;
			dataOut[(outIdx+subSeqLength)*offset] = u0 - u1;
		}

		// swap
		for(int i=0; i<nSample; ++i)
		{
			dataIn[i*offset] = dataOut[i*offset];
		}
		
	}
}