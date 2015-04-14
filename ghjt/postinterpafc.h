// PostInterpAFC.h: interface for the CPostInterpAFC class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_POSTINTERPAFC_H__D70C2BF0_7D41_11D6_8DF2_00C04FA11AF6__INCLUDED_)
#define AFX_POSTINTERPAFC_H__D70C2BF0_7D41_11D6_8DF2_00C04FA11AF6__INCLUDED_

#include "DRMConfig.h"
#include "../S_Defs.h"
#include <math.h>
#include <string.h>

#include "../bbc_types.h"


//***************************************************************************
//*			*** CPostInterpAFC ***     Fixed Point Version 
//***************************************************************************

class CPostInterpAFC  
{
public:

	void FindFreqErr(tNative		* newChannel, 
					 CDRMConfig *config, 
					 tNative		&clock_err, 
					 tNativeAcc		interferer_freq);

	CPostInterpAFC();
	virtual ~CPostInterpAFC();

private:

	tNative * m_oldChannel;
	tNative arctan2_fxp(tNative a, tNative b);

};


#endif // !defined(AFX_POSTINTERPAFC_H__D70C2BF0_7D41_11D6_8DF2_00C04FA11AF6__INCLUDED_)
