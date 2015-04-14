// NotchFilter.h: interface for the CNotchFilter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NOTCHFILTER_H__00D5FA31_2139_11D6_8DC2_00C04FA11AF6__INCLUDED_)
#define AFX_NOTCHFILTER_H__00D5FA31_2139_11D6_8DC2_00C04FA11AF6__INCLUDED_

#include "FreqCorrector.h"	// Added by ClassView
#include "DRMConfig.h"


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define NOTCH_FILTER_TAPS 5

class CNotchFilter  
{
public:

	void Filter(tNative *data, 
				tNativeAcc freq, 
				tNative NumSamples,
				CDRMConfig *config);

	CNotchFilter();
	virtual ~CNotchFilter();

private:

	void RecursiveHPF(tNative *data, tNativeAcc NumSamples);
	tNative m_IIRStoreRe;
	tNative m_IIRStoreIm;
	
	CFreqCorrector m_OutputFreqShifter;
	CFreqCorrector m_InputFreqShifter;
};

#endif // !defined(AFX_NOTCHFILTER_H__00D5FA31_2139_11D6_8DC2_00C04FA11AF6__INCLUDED_)
