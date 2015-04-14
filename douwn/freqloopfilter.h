// FreqLoopFilter.h: interface for the FreqLoopFilter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FREQLOOPFILTER_H__A87F4C51_03C1_11D4_818D_00C04F7DD1B3__INCLUDED_)
#define AFX_FREQLOOPFILTER_H__A87F4C51_03C1_11D4_818D_00C04F7DD1B3__INCLUDED_

#include "../bbc_types.h"
#include "../S_Defs.h"
#include "../structurescommon.h"
#include "../configcommands.h"
#include "DRMConfig.h"


//*****************************************************
//*		Defs and Macros for the FreqLoopFilter 
//*		Fixed Point Version
//*****************************************************
#define FREQLOOPFILTER_BITS 23 
#define FREQLOOPFILTER_ONE (1<<FREQLOOPFILTER_BITS) 
#define MAX_FREQ_PULL_FXP 409600    // * To limit the swing of the frequency pull *


class CFreqLoopFilter  
{
public:	
	void Reset();

	void CoarseJump(tNativeAcc jump, CDRMConfig *pConfig);

	tNativeAcc Filter2(tNativeAcc freq_err);

	tNativeAcc Filter(tNative freq_err, TFilt filt_comm);

	CFreqLoopFilter();
	virtual ~CFreqLoopFilter();

private:

	tNative m_KAFC_pre_fft_fxp;
	tNative m_KAFC_post_fft_fxp;
	tNativeAcc m_accumulator_fxp; // * 32 bit *

};

#endif // !defined(AFX_FREQLOOPFILTER_H__A87F4C51_03C1_11D4_818D_00C04F7DD1B3__INCLUDED_)
