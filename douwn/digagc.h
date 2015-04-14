// DigAGC.h: interface for the CDigAGC class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIGAGC_H__AB4673F1_7955_11D6_8DEF_00C04FA11AF6__INCLUDED_)
#define AFX_DIGAGC_H__AB4673F1_7955_11D6_8DEF_00C04FA11AF6__INCLUDED_

#include "../bbc_types.h"

#include "DRMConfig.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDigAGC  
{
public:
	float AGC(float *data, int numComplex, float target, float kAGC);
	CDigAGC();
	virtual ~CDigAGC();

private:
	float m_mean_power;
	UInt32* m_test1;
};

#endif // !defined(AFX_DIGAGC_H__AB4673F1_7955_11D6_8DEF_00C04FA11AF6__INCLUDED_)
