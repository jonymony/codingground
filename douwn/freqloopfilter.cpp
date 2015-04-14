// FreqLoopFilter.cpp: implementation of the FreqLoopFilter class.
//
//////////////////////////////////////////////////////////////////////

#include "FreqLoopFilter.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFreqLoopFilter::CFreqLoopFilter()
{
	m_accumulator_fxp  = 0;	
	
	// *** Decides the filter BW for the pre fft frequency loop (fxp base 23) ***
	//m_KAFC_pre_fft_fxp = 50;//300;  
	m_KAFC_pre_fft_fxp = 167;//300;  
	
	// *** Decides the filter BW for the post fft frequency loop (fxp base 23) ***
	//m_KAFC_post_fft_fxp= 50;//300;  
	m_KAFC_post_fft_fxp= 167;//300;  
		
}

CFreqLoopFilter::~CFreqLoopFilter()
{

}


//***************************************************************************
//*
//*			Function name : *** Filter ***
//*
//*			Description : Pre FFT frequency loop filter
//*
//***************************************************************************

tNativeAcc CFreqLoopFilter::Filter(tNative freq_err, // * this is base 13 fxp *
							TFilt filt_comm)
{
	if(filt_comm==clear_filter) 
		m_accumulator_fxp = 0;

	// *** Single integrator loop with negative polarity ***
	m_accumulator_fxp -= NATIVE_CAST_ACC((m_KAFC_pre_fft_fxp * freq_err)/(1<<13));

	// *** Clip to keep in range ***
	if (m_accumulator_fxp < -MAX_FREQ_PULL_FXP) 
		m_accumulator_fxp = -MAX_FREQ_PULL_FXP;
	if (m_accumulator_fxp >  MAX_FREQ_PULL_FXP) 
		m_accumulator_fxp =  MAX_FREQ_PULL_FXP;

	return (m_accumulator_fxp);	
}


//***************************************************************************
//*
//*			Function name : *** CoarseJump ***
//*
//*			Description :  Jump by a whole number of carriers
//*
//***************************************************************************

void CFreqLoopFilter::CoarseJump(tNativeAcc		jump, 
								 CDRMConfig *pConfig)
{
	
	// *** FFT_SIZE carriers equals the sampling frequency ***
	tNativeAcc new_accu_val = m_accumulator_fxp + jump * (FREQLOOPFILTER_ONE/(pConfig->fftsize()));

	// *** Only allow the jump if frequency will end up inside the allowed range ***
	if (new_accu_val > -MAX_FREQ_PULL_FXP && new_accu_val < MAX_FREQ_PULL_FXP)
		m_accumulator_fxp = new_accu_val;
}


//***************************************************************************
//*
//*			Function name : *** Filter2 ***
//*
//*			Description : Post FFT frequency loop filter
//*
//***************************************************************************

tNativeAcc CFreqLoopFilter::Filter2(tNativeAcc freq_err)
{	
	// *** Using divide instead of shift to gain accuracy (this was neccessary) *** 
	m_accumulator_fxp -= NATIVE_CAST_ACC ((m_KAFC_post_fft_fxp * freq_err)/FREQLOOPFILTER_ONE);

	// *** Clip to keep in range ***
	if( m_accumulator_fxp < -MAX_FREQ_PULL_FXP) 
		m_accumulator_fxp = -MAX_FREQ_PULL_FXP;
	
	if( m_accumulator_fxp > MAX_FREQ_PULL_FXP) 
		m_accumulator_fxp = MAX_FREQ_PULL_FXP;

	return ( m_accumulator_fxp );
}


void CFreqLoopFilter::Reset()
{
	m_accumulator_fxp=0;
}

