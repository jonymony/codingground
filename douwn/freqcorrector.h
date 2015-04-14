// FreqCorrector.h: interface for the FreqCorrector class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FREQCORRECTOR_H__3F24B0AE_EA01_11D3_8188_00C04F7DD1B3__INCLUDED_)
#define AFX_FREQCORRECTOR_H__3F24B0AE_EA01_11D3_8188_00C04F7DD1B3__INCLUDED_

#include "../configcommands.h"	
#include "../bbc_types.h"

//***************************************************************************
//*			*** CFreqCorrector ***     Fixed Point Version 
//***************************************************************************

class CFreqCorrector  
{
public:
	void Correct(tNative *data_fxp, 
				 tNativeAcc ffreq, 
				 tNativeAcc total_symbol, 
				 TSpecInvert spec_invert);

	CFreqCorrector();
	virtual ~CFreqCorrector();

private:
	static tNative m_InstanceCount;
	tNativeAcc m_phase;
	tNativeAcc m_quadrant;
	static tNative* m_SinTable;
};



#endif // !defined(AFX_FREQCORRECTOR_H__3F24B0AE_EA01_11D3_8188_00C04F7DD1B3__INCLUDED_)
