// Fourier.h: interface for the CFourier class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FOURIER_H__97D39183_5719_11D4_8BDE_00C04FA11AF6__INCLUDED_)
#define AFX_FOURIER_H__97D39183_5719_11D4_8BDE_00C04FA11AF6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../S_Defs.h"
#include "../FFTTypes.h"
#include "../bbc_types.h"

#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr
#define ISWAP(a,b) itemp=(a);(a)=(b);(b)=itemp //integer swap

class CFourier  
{
public:
	void ifft448(const fftw_complex *input, fftw_complex *output);
	void ifft704(const fftw_complex *input, fftw_complex *output);
	void ifft1024(const fftw_complex* input, fftw_complex* output);
	void ifft1152(const fftw_complex* input, fftw_complex* output);
	void ifft576(const fftw_complex *input, fftw_complex *output);
	void fft224(const fftw_complex *input, fftw_complex *output);
	void fft352(const fftw_complex *input, fftw_complex *output);
	void fft512(const fftw_complex* input, fftw_complex* output);
	void fft576(const fftw_complex* input, fftw_complex* output);

	void SideBandSwap(tNative* data, tNativeAcc nn);


	CFourier();
	virtual ~CFourier();


private:
	fftw_complex* GenerateTwiddles(tNativeAcc n, tNativeAcc length);
	fftw_complex* W224_16;
	fftw_complex* W14_7;
	fftw_complex* W512_16;
	fftw_complex* W32_8; 
	fftw_complex* W576_16; 
	fftw_complex* W36_9; 
	fftw_complex* W64_4;
	fftw_complex* W128_8;
	fftw_complex* W1024_8;
	fftw_complex* W576_9;
	fftw_complex* W1152_9;
	fftw_complex* W112_7;
	fftw_complex* W448_4;
	fftw_complex* W352_8;
	fftw_complex* W44_4;
	fftw_complex* W88_8;
	fftw_complex* W704_8;
	void fftw_twiddle_4(fftw_complex *A, const fftw_complex *W, tNativeAcc iostride, tNativeAcc m, tNativeAcc dist);
	void fftw_twiddle_7(fftw_complex *A, const fftw_complex *W, tNativeAcc iostride, tNativeAcc m, tNativeAcc dist);
	void fftw_twiddle_8(fftw_complex* A, const fftw_complex* W, tNativeAcc iostride, tNativeAcc m, tNativeAcc dist);
	void fftw_twiddle_9(fftw_complex* A, const fftw_complex* W, tNativeAcc iostride, tNativeAcc m, tNativeAcc dist);
	void fftw_twiddle_16(fftw_complex* A, const fftw_complex* W, tNativeAcc iostride, tNativeAcc m, tNativeAcc dist);
	void fftw_no_twiddle_4(const fftw_complex *input, fftw_complex *output, tNativeAcc istride, tNativeAcc ostride);
	void fftw_no_twiddle_2(const fftw_complex *input, fftw_complex *output,tNativeAcc istride, tNativeAcc ostride);
	void fftw_no_twiddle_11(const fftw_complex *input, fftw_complex *output, tNativeAcc istride, tNativeAcc ostride);
	void fftwi_no_twiddle_16(const fftw_complex* input, fftw_complex* output, tNativeAcc istride, tNativeAcc ostride);
	void fftwi_no_twiddle_11(const fftw_complex *input, fftw_complex *output, tNativeAcc istride, tNativeAcc ostride);
	void fftwi_twiddle_4(fftw_complex *A, const fftw_complex *W, tNativeAcc iostride, tNativeAcc m, tNativeAcc dist);
	void fftwi_twiddle_7(fftw_complex *A, const fftw_complex *W, tNativeAcc iostride, tNativeAcc m, tNativeAcc dist);
	void fftwi_twiddle_8(fftw_complex *A, const fftw_complex *W, tNativeAcc iostride, tNativeAcc m, tNativeAcc dist);
	void fftwi_twiddle_9(fftw_complex *A, const fftw_complex *W, tNativeAcc iostride, tNativeAcc m, tNativeAcc dist);

};

#endif // !defined(AFX_FOURIER_H__97D39183_5719_11D4_8BDE_00C04FA11AF6__INCLUDED_)
