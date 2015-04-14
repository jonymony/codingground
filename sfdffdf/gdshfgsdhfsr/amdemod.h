// AMDemod.h: interface for the CAMDemod class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AMDEMOD_H__15524AD1_C2ED_11D5_9754_00C04FA11197__INCLUDED_)
#define AFX_AMDEMOD_H__15524AD1_C2ED_11D5_9754_00C04FA11197__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <stdio.h>




#ifdef _PC_VER
#include "../bbc_types.h"
#else
#include <tmLib/tmTypes.h>
#endif

#include "../common_flp/resample.h"
#include "../common_flp/upsampler.h"
//#include "../common/DiagHandler.h"

//#include "XIO.h"
#include "ChannelFilter_flp.h"	// Added by ClassView
#include "FreqCorrector_flp.h"

#define FILTER_COEFFICIENTS_3K 8  //FILTER_COEFFICIENTS<=15!!! (stability problems because of coefficients rounding)
#define FILTER_COEFFICIENTS_5K 8  //FILTER_COEFFICIENTS<=15!!! (stability problems because of coefficients rounding)



class CAMDemod  
{
public:
/*	void SetChannelFilter(int type, UInt8 *address, int length,
		int *retType, int *retLen, UInt8 **retAdd);*/

	void Reset();
	CAMDemod();
	virtual ~CAMDemod();
	TM_BOOL ProcessBlock(tUNativeAcc pull_down, tNative* data_in_fxp, float* data_out, Int16* OutMemAudio, 
		Int16* OutMemData, Int16* pOutMemSpdo, TM_BOOL& monit_ready, int& audio_ready);
	float* ReadSignalLevel();
	int ReadFilterBW();
	void SetVolume(int VolL, int VolR);
	void SetFilt(int filt_value);
	char ReadVULeft();
	char ReadVURight();


private:
	float *m_data_in;
	int m_spdif_frame;
	float m_right_volume;
	float m_left_volume;
	UInt16 m_leftPeak;
	UInt16 m_rightPeak;
	CFreqCorrector_flp m_freq_corrector;
	CChannelFilter_flp m_channel_filter;
	void SignalLevels(float* pFloat, int symbol, int points);
	TM_BOOL envelope_det(float *in_buffer, float *out_buffer, Int16* OutMemAudio,
							Int16* OutMemData, Int16* OutMemSpdo,
							Int32 length);

	

	TM_BOOL Output(float *in, Int16* OutMemAudio,
							Int16* OutMemData, Int16* OutMemSpdo, 
							UInt16& left_peak, UInt16& right_peak,
							Int32 length);
	CResample m_resamp;
	CUpsampler m_upsamp;
	int m_count;
	int m_free_symbolnum;
	float m_offset;

	int m_audio_index;

	float m_dc_removal_filter;
	float m_dc_agc_filter;
	float m_ac_power_filter;


	float* m_pAudio;

//	float m_y_buffer[FILTER_COEFFICIENTS_3K-1]; //buffers  y[DATA_BLOCK_SIZE-FILTER_COEFFICIENTS+1]..y[DATA_BLOCK_SIZE-1] of the LAST data block
//	float m_x_buffer[FILTER_COEFFICIENTS_3K-1]; //buffers  x[DATA_BLOCK_SIZE-FILTER_COEFFICIENTS+1]..x[DATA_BLOCK_SIZE-1] of the LAST data block

	static float m_a_5[FILTER_COEFFICIENTS_5K];
	static float m_b_5[FILTER_COEFFICIENTS_5K];
	static float m_a_3[FILTER_COEFFICIENTS_3K];
	static float m_b_3[FILTER_COEFFICIENTS_3K];
	
//	UInt32* m_test1;

//	float* m_filt_out;

	float* m_local_signal_level;
	float* m_signal_level;
	float* m_peak_level;
	float m_signal_level_av;
	float m_peak_level_av;


};

#endif // !defined(AFX_AMDEMOD_H__15524AD1_C2ED_11D5_9754_00C04FA11197__INCLUDED_)
