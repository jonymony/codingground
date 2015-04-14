// CWFinder.h: interface for the CCWFinder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CWFINDER_H__CBD6B5A1_1F9B_11D6_8DC1_00C04FA11AF6__INCLUDED_)
#define AFX_CWFINDER_H__CBD6B5A1_1F9B_11D6_8DC1_00C04FA11AF6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "drmconfig.h"


class CCWFinder  
{
public:

	int FindCW(tNative *pFFTOut, 
			   tNative *pFilteredSpectrum, 
			   CDRMConfig *pConfig,
			   Int16& CWBin);

	CCWFinder();
	virtual ~CCWFinder();

};

#endif // !defined(AFX_CWFINDER_H__CBD6B5A1_1F9B_11D6_8DC1_00C04FA11AF6__INCLUDED_)
