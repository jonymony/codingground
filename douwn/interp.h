// Interp.h: interface for the CInterp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INTERP_H__6A2311B1_E51C_11D3_8AF1_00C04FA11AF6__INCLUDED_)
#define AFX_INTERP_H__6A2311B1_E51C_11D3_8AF1_00C04FA11AF6__INCLUDED_

#include "../S_Defs.h"

#include "DRMConfig.h"

#include "../bbc_types.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
 
//***************************************************************************
//*			*** Cinterp ***     Fixed Point Version 
//***************************************************************************

#define TAU_BITS 10

class CInterp 
{
public:
	void interp(tNative	*data, 
				tNativeAcc		freq, 
				tNative   jump, 
				TM_BOOL &symbol_valid, 
				tNative   total_symbol);
	CInterp();
	virtual ~CInterp();

private:
	tNative* m_del_interp;
	tNativeAcc    m_tau; //* 32 bit *
	tNative  m_out_sel;
	tNative  m_state;
	tNative  m_out_ptr;
	TM_BOOL  m_jump_flag;
	tNative (*m_frame_store)[2*TOTAL_SYMBOL_MALLOC]; 

};


#endif // !defined(AFX_INTERP_H__6A2311B1_E51C_11D3_8AF1_00C04FA11AF6__INCLUDED_)
