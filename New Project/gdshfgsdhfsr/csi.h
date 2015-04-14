// CSI.h: interface for the CCSI class.

//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CSI_H__F9F7DEF0_1697_11D4_8B94_00C04FA11AF6__INCLUDED_)
#define AFX_CSI_H__F9F7DEF0_1697_11D4_8B94_00C04FA11AF6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../bbc_types.h"
#include "../S_Defs.h"
#include "../structurescommon.h"
#include "../configcommands.h"
#include "drmconfig.h"


 
//***************************************************************************
//*			*** Class CCSI ***     Fixed Point Version 
//***************************************************************************

class CCSI  
{
public:

	void CalcWeights(const tNative *csi, 
					 const tNative *channel, 
					 tNative *weights, 
					 CDRMConfig* config);

	void update_state(const tNative* data_in, 
					  const tNative* channel, 
					  tNative* channel_state,
					  tNative* mer, 
					  CDRMConfig* config, 
					  tNative symbolnum, 
					  tNative framenum,
					  TFilt filt_comm, 
					  TM_BOOL post_fft_timing,
					  tNative& csi_mean, 
					  tNative& csi_peak, 
					  tNative& csi_peak_pos, 
					  TM_BOOL FAC_ready);
	CCSI();
	virtual ~CCSI();

private:

	// * fixed point: 10 * log10(in) *
	tNative fxp_10_log10(tUNativeAcc in);
	
	tNative m_unwgt_mer_msc; // * convert to flp by dividing with (1<<7) *
	tNative m_wgt_mer_msc;   // * convert to flp by dividing with (1<<7) *
	tNative m_wgt_mer_fac;   // * convert to flp by dividing with (1<<7) *

	tNative m_cell_cnt_msc;

	tNativeAcc m_sum_posteq_sq_err_msc;
	tNativeAcc m_sum_sq_chan_msc;
	tNativeAcc m_sum_preeq_sq_err_msc;
	tNativeAcc m_sum_sq_chan_fac;
	tNativeAcc m_sum_preeq_sq_err_fac;
};



#endif // !defined(AFX_CSI_H__F9F7DEF0_1697_11D4_8B94_00C04FA11AF6__INCLUDED_)
