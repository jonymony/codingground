// ModeDetect.cpp: implementation of the CModeDetect class.
//
//////////////////////////////////////////////////////////////////////

#include "ModeDetect.h"
#include "../message_types.h"
#include <stdio.h>
#include <string.h>

#include "../pc_types.h"


// *** The number of times same mode has to be found to be confirmed to be true ***
#define MAX_MODE_CONF 20 //10 Try this JEE 4/12/02, set back to 20 AEGN 12/02/04 
                                                           
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CModeDetect::CModeDetect()
{
	m_mode_conf=0;
	m_old_mode=unknown;

	m_data_outA = new tNative[TOTAL_SYMBOL_COMPLEX_MALLOC];
	m_data_outB = new tNative[TOTAL_SYMBOL_COMPLEX_MALLOC];
	m_data_outC = new tNative[TOTAL_SYMBOL_COMPLEX_MALLOC];
	m_data_outD = new tNative[TOTAL_SYMBOL_COMPLEX_MALLOC];
}

CModeDetect::~CModeDetect()
{
	delete [] m_data_outA;
	delete [] m_data_outB;
	delete [] m_data_outC;
	delete [] m_data_outD;
}


//**************************************************************************
//*
//*			Function name : *** DetectMode ***
//*
//*			Description :	Returning TRUE if mode is found 
//*							with confidence of 20 symbols
//*							otherwise returning FALSE
//*
//*			Tree : CDRMProc::Pre_FFT_process
//*							\->CModeDetect::DetectMode
//*										
//**************************************************************************

TM_BOOL CModeDetect::DetectMode(tNative* data_in, 
								tNative *monitor, 
								tNative monit_points,
								TMode& mode, 
								CDRMConfig* config,
								tNative* peak)
{
	tNative i;

	TFilt filt_comm=config->filtering();

	TMode mode1,mode2;

	TM_BOOL mode_found=FALSE;

	tNative peakA,peakB,peakC,peakD,peak1,peak2;
	tNative peak_posA, peak_posB, peak_posC, peak_posD;

	tNative monitor_pos = 0;


	// *** MODE A ***

	// ** Correlate **
	m_time_procA.time_correlate_detect(data_in, m_data_outA, FFT_SIZE_A, GUARD_INTERVAL_A, config);

	// ** Filter **
	m_time_procA.filter_detect(m_data_outA, config, FFT_SIZE_A, GUARD_INTERVAL_A);

	// ** Smooth **
	m_time_procA.smoother_detect(m_data_outA, filt_comm, FFT_SIZE_A, GUARD_INTERVAL_A);

	// ** Find Peak **
	peakA=m_time_procA.peak_find(m_data_outA, FFT_SIZE_A, GUARD_INTERVAL_A, config, peak_posA );

	for (i=0; i<TOTAL_SYMBOL_COMPLEX_A; i+=4) //subsample 2:1
	{
		monitor[monitor_pos++] = m_data_outA[i];
		monitor[monitor_pos++] = m_data_outA[i+1];
	}

	monitor[monitor_pos++] = -32766; // marker between modes
	monitor[monitor_pos++] = -32766;




	// *** MODE B ***

	// ** Correlate **
	m_time_procB.time_correlate_detect(data_in, m_data_outB, FFT_SIZE_B, GUARD_INTERVAL_B, config);

	// ** Filter **
	m_time_procB.filter_detect(m_data_outB, config, FFT_SIZE_B, GUARD_INTERVAL_B);

	// ** Smooth **
	m_time_procB.smoother_detect(m_data_outB, filt_comm, FFT_SIZE_B, GUARD_INTERVAL_B);

	// ** Find Peak **
	peakB=m_time_procB.peak_find(m_data_outB, FFT_SIZE_B, GUARD_INTERVAL_B, config, peak_posB);

	for (i=0; i<TOTAL_SYMBOL_COMPLEX_B; i+=4) //subsample 2:1
	{
		monitor[monitor_pos++] = m_data_outB[i];
		monitor[monitor_pos++] = m_data_outB[i+1];
	}
	monitor[monitor_pos++] = -32766; // marker between modes
	monitor[monitor_pos++] = -32766;


	// *** MODE C ***

	// ** Correlate **
	m_time_procC.time_correlate_detect(data_in, m_data_outC, FFT_SIZE_C, GUARD_INTERVAL_C, config);

	// ** Filter **
	m_time_procC.filter_detect(m_data_outC, config, FFT_SIZE_C, GUARD_INTERVAL_C);

	// ** Smooth **
	m_time_procC.smoother_detect(m_data_outC, filt_comm, FFT_SIZE_C, GUARD_INTERVAL_C);

	// ** Find Peak **
	peakC=m_time_procC.peak_find(m_data_outC, FFT_SIZE_C, GUARD_INTERVAL_C, config, peak_posC);

	for (i=0; i<TOTAL_SYMBOL_COMPLEX_C; i+=4) //subsample 2:1
	{
		monitor[monitor_pos++] = m_data_outC[i];
		monitor[monitor_pos++] = m_data_outC[i+1];
	}
	monitor[monitor_pos++] = -32766; // marker between modes
	monitor[monitor_pos++] = -32766;




	// *** MODE D ***

	// ** Correlate **
	m_time_procD.time_correlate_detect(data_in, m_data_outD, FFT_SIZE_D, GUARD_INTERVAL_D, config);

	// ** Filter **
	m_time_procD.filter_detect(m_data_outD, config, FFT_SIZE_D, GUARD_INTERVAL_D);

	// ** Smooth **
	m_time_procD.smoother_detect(m_data_outD, filt_comm, FFT_SIZE_D, GUARD_INTERVAL_D);

	// ** Find Peak **
	peakD=m_time_procD.peak_find(m_data_outD, FFT_SIZE_D, GUARD_INTERVAL_D, config, peak_posD);

	for (i=0; i<TOTAL_SYMBOL_COMPLEX_D; i+=4) //subsample 2:1
	{
		monitor[monitor_pos++] = m_data_outD[i];
		monitor[monitor_pos++] = m_data_outD[i+1];
	}
	monitor[monitor_pos++] = -32766; // marker between modes
	monitor[monitor_pos++] = -32766;

	for (i=monitor_pos; i<monit_points*2; i++) 
		monitor[i]=0;

	// *** And the winner is... ***

	//printf("\n\n#############    A<%d>  B<%d>  C<%d>  D<%d>\n\n",peakA,peakB,peakC,peakD);

	// ** Mode A or B ? (semifinal) **
	if(peakA > peakB)
	{
		peak1=peakA;
		mode1=ground;
	}
	else
	{
		peak1=peakB;
		mode1=sky;
	}

	// ** Mode C or D ? (semifinal) **
	if(peakC > peakD) 
	{
		peak2=peakC;
		mode2=robust1;
	}
	else 
	{
		peak2=peakD;
		mode2=robust2;
	}

	// ** Winner		(final) **
	if(peak1 <= peak2)
	{
		peak1=peak2;
		mode1=mode2;
	}

		
	tNativeAcc mark_pos = 1, mark_pos_div, mark_pos_div2, test1, test2;

	switch(mode1)
	{
	case ground:
		mark_pos_div2 = peak_posA/2;
		mark_pos_div = peak_posA - ((mark_pos_div2) *2);

		test1 = peak_posA & 0x1;
		test2 = mark_pos_div2 & 0x1;
		if((test1 & 0x1) && (test2 & 0x1) || !(test1 & 0x1) && !(test2 & 0x1))
			mark_pos = 0;
		

		monitor[mark_pos_div2 + mark_pos] = 100;
		break;
	case sky:
		mark_pos_div2 = peak_posB/2;
		mark_pos_div = peak_posB - ((mark_pos_div2) *2);

		test1 = peak_posB & 0x1;
		test2 = mark_pos_div2 & 0x1;
		if((test1 & 0x1) && (test2 & 0x1) || !(test1 & 0x1) && !(test2 & 0x1))
			mark_pos = 0;
		monitor[mark_pos_div2 + mark_pos + TOTAL_SYMBOL_A] = 100;
		break;
		
	case robust1:
		mark_pos_div2 = peak_posC/2;
		mark_pos_div = peak_posC - ((mark_pos_div2) *2);

		test1 = peak_posC & 0x1;
		test2 = mark_pos_div2 & 0x1;
		if((test1 & 0x1) && (test2 & 0x1) || !(test1 & 0x1) && !(test2 & 0x1))
			mark_pos = 0;

		monitor[mark_pos_div2 + mark_pos + TOTAL_SYMBOL_A + TOTAL_SYMBOL_B] = 100;
		break;
		
	case robust2:
		mark_pos_div2 = peak_posD/2;
		mark_pos_div = peak_posD - ((mark_pos_div2) *2);

		test1 = peak_posD & 0x1;
		test2 = mark_pos_div2 & 0x1;
		if((test1 & 0x1) && (test2 & 0x1) || !(test1 & 0x1) && !(test2 & 0x1))
			mark_pos = 0;

		monitor[mark_pos_div2 + mark_pos + TOTAL_SYMBOL_A + TOTAL_SYMBOL_B + TOTAL_SYMBOL_C] = 100;
		break;
	}
	
	// *** What is stored in Peak1 and Mode1 is the final mode *** 

#ifdef _SIM

	switch(mode1)
	{
	case ground: DP(("Mode A\n"));break;
	case sky:    DP(("Mode B\n"));break;
	case robust1:DP(("Mode C\n"));break;
	case robust2:DP(("Mode D\n"));break;
	case unknown:default:DP(("Mode UNKNOWN\n"));break;
	}

#endif

//	static int temp =0;

	// *** If current mode is same as previous mode and the found peak is not nil ***
	if(mode1 == m_old_mode && peak1 > 0)
	{
		// ** counter for the number of time the same mode has been detected **
		m_mode_conf++;
		m_old_mode=mode1;

		*peak = peak1;

		// ** If the same mode has been detected 20 times in a row, the mode is confirmed **
		if(m_mode_conf >= MAX_MODE_CONF)
		{
			m_mode_conf=MAX_MODE_CONF;
			mode=m_old_mode;
			mode_found=TRUE;
		}
	}

	// *** The current mode detected is changed from previous mode ***
	else 
	{
		m_mode_conf=0;
		m_old_mode=mode1;
	}

#ifdef _SIM

	if(mode_found)
		DP(("\n\n\nNOW I THINK I FOUND THE RIGHT MODE\nAND IT SEEMS TO BE : %d\n(0=A)\n(1=B)\n(2=C)\n(3=D)\n\n\n",mode1));
#endif

	// *** Returning TRUE if mode is found with confidence, otherwise returning FALSE ***
	return(mode_found);
}

void CModeDetect::Reacquire()
{
	m_mode_conf=0;
	m_old_mode=unknown;
}

void CModeDetect::Reset()
{
	m_time_procA.Reset(FFT_SIZE_A*2);
	m_time_procB.Reset(FFT_SIZE_B*2);
	m_time_procC.Reset(FFT_SIZE_C*2);
	m_time_procD.Reset(FFT_SIZE_D*2);
}
