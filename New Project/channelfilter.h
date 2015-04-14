// ChannelFilter.h: interface for the CChannelFilter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHANNELFILTER_H__696F4310_9C7F_11D5_8D6F_00C04FA11AF6__INCLUDED_)
#define AFX_CHANNELFILTER_H__696F4310_9C7F_11D5_8D6F_00C04FA11AF6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../S_defs.h"
#include "../bbc_types.h"

#include "DRMConfig.h"

//******************************
//* Spectral Occupacy (SpecOcc)
//* (filter bandwith)
//* 0 -> 4.5  kHz
//* 1 -> 5.0  kHz
//* 2 -> 9.0  kHz
//* 3 -> 10.0 kHz
//* 4 -> 18.0 kHz
//* 5 -> 20.0 kHz
//* 6 -> 3.0  kHz (was used during mode detection)
//******************************

// *** What filter to use during mode detection ***
//#define MODE_SEARCH_FILTER MAX_SPEC_OCC+1
#define MODE_SEARCH_FILTER 0

#define FILTER_STAGES 4

//***************************************************************************
//*			*** CChannelFilter ***     Fixed Point Version 
//***************************************************************************

struct tBiquadState
{
	tNative w1re;
	tNative w2re;
	tNative w1im;
	tNative w2im;
};

struct tBiquadCoeffs
{
	tNative b0;
	tNative b1;
	tNative b2;
	tNative a0;
	tNative a1;
	tNative a2;
};

class CChannelFilter  
{
public:

	void Filter(tNative *outinfxp , 
				const tNative NumSamples);
	
	int GetFilterBW();
	void SetSpecOcc(tNative specOcc);

	CChannelFilter();
	virtual ~CChannelFilter();

private:
	static const tNative m_filterGains[(MAX_SPEC_OCC+2)];
	static const tNative m_ShiftList[7][4];
	tNative m_numStages;

	void BiquadFilter(const tNative *in, 
					tNative *out, 
					const tNative NumSamples, 
					const tBiquadCoeffs *coeffs, 
					tBiquadState *state,
					tNative shifting);

	tNative m_filterIndex;
	
	static const tBiquadCoeffs m_filterCoeffs[(MAX_SPEC_OCC+2)][FILTER_STAGES];

	tBiquadState* m_filterState;
};


#endif // !defined(AFX_CHANNELFILTER_H__696F4310_9C7F_11D5_8D6F_00C04FA11AF6__INCLUDED_)
