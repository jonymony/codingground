// ImpulseResp.h: interface for the CImpulseResp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMPULSERESP_H__8DFD5311_4D96_11D4_8BCE_00C04FA11AF6__INCLUDED_)
#define AFX_IMPULSERESP_H__8DFD5311_4D96_11D4_8BCE_00C04FA11AF6__INCLUDED_

#include "../bbc_types.h"
#include "../S_Defs.h"
#include "Fourier.h"	
#include "DRMConfig.h"


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define TOTAL_MASS_THRES 80
#define PEAK_AVERAGE_THRES 10
//#define WINDOW_TABLE_SIZE 1024

class CImpulseResp  
{
public:
	
	void FindImpRespMag(tNative		*imp_resp_in, 
						CDRMConfig	*config, 
						tNative&		cir_peak, 
						tNative&		cir_peak_pos);
	
	void Config(		CDRMConfig* config);


	void Search(		tNative		*data_input, 
						tNative		*data_out, 
						tNativeAcc         &best_pos, 
						CDRMConfig	*config,
						tNative& start,
						tNative& end,
						tNative& middle);

	void FindImpResp(	tNative		 *data_in, 
						tNative		 *data_out, 
						tNativeAcc		     &phase_error, // * 32 bit *
						tNativeAcc			 length, 
						CDRMConfig   *config, 
						tNative		 &doppler);	

	CImpulseResp();
	virtual ~CImpulseResp();

private:
//	float * m_WindowTable;
//	void Window(float *data, tNativeAcc NumComplex);
	void (CFourier::* m_FFTFuncPtr) (const fftw_complex*, fftw_complex*);
	tNative		arctan2_fxp(tNativeAcc y, tNativeAcc x);
	CFourier	m_cir_fourier;
	tNativeAcc			m_upsample_factor;
	tNative		m_correct_factor;
	tNative		*m_cir_old;
	tNative		*m_ir;
	tNative		*m_fft_out;
};

#endif // !defined(AFX_IMPULSERESP_H__8DFD5311_4D96_11D4_8BCE_00C04FA11AF6__INCLUDED_)
