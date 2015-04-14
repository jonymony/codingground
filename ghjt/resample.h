// UpsampData.h: interface for the CUpsampData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UPSAMPDATA_H__BBA832C1_EA9A_11D3_8AF6_00C04FA11AF6__INCLUDED_)
#define AFX_UPSAMPDATA_H__BBA832C1_EA9A_11D3_8AF6_00C04FA11AF6__INCLUDED_

#include "DRMConfig.h"
#include "../bbc_types.h"


#include "Interp.h"

#define TAU_BITS 10

//***************************************************************************
//*			*** CResample ***     Fixed Point Version 
//***************************************************************************

class CResample  
{
public:
	TM_BOOL resamp(tNativeAcc freq, 
				tNative jump, 
				tNative* data, 
				tNative length);
	CResample();
	virtual ~CResample();

private:

	//CInterp m_interp;

	tNative* m_del_interp;
	tNativeAcc    m_tau; //* 32 bit *
	tNative  m_out_sel;
	tNative  m_state;
	tNative  m_out_ptr;
	TM_BOOL  m_jump_flag;
	tNative (*m_frame_store)[2*TOTAL_SYMBOL_MALLOC]; 


};

#endif // !defined(AFX_UPSAMPDATA_H__BBA832C1_EA9A_11D3_8AF6_00C04FA11AF6__INCLUDED_)
