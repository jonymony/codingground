// FreqCorrector.cpp: implementation of the FreqCorrector class.
//
//////////////////////////////////////////////////////////////////////

#include "FreqCorrector.h"
#include <math.h>
#include <stdio.h>
#include "../bbc_types.h"
#include "../S_Defs.h"
#include "../pc_types.h"


tNative CFreqCorrector::m_InstanceCount = 0; 
tNative *CFreqCorrector::m_SinTable = NULL;

#define EXTRABITS 13 
#define SIN_TABLE_BITS 15 

#define LOG2_SIN_TABLE_SIZE 10 
#define SIN_TABLE_SIZE (1<<LOG2_SIN_TABLE_SIZE)


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//***************************************************************************
//*			*** CFreqCorrector constructor ***     Fixed Point Version 
//***************************************************************************

CFreqCorrector::CFreqCorrector()
{
	// * m_SinTable is static: we only want one table *
	if (m_SinTable == NULL) 
	{
		m_SinTable = new tNative[SIN_TABLE_SIZE+3]; //m_SinTable[1024+3]

		tNative i;
		// *** Create a lookup-table for sin ***
		for (i=0; i<SIN_TABLE_SIZE; i++)  
			m_SinTable[i] = SHORT_B(((1<<SIN_TABLE_BITS)-1)*sin(PI/2.0f * (((float)(2*i+1)) / ((float)2*SIN_TABLE_SIZE)))); 
	}
	m_phase=0; m_quadrant=0;
	m_InstanceCount++;
}

CFreqCorrector::~CFreqCorrector()
{
	m_InstanceCount--;
	if (m_InstanceCount==0)
	{
		delete [] m_SinTable;
		m_SinTable = NULL;
	}
}

//***************************************************************************
//*
//*			Function name : *** Correct ***
//*
//*			Decription : Function that shifts an iq signal in frequency by multiplying
//*						 with a rotating unit vector.
//*			
//*						 ffreq decides how much to shift the input array data_fxp
//*						 2^23 is a shift by the samplings frequency
//*						 e.g. Sampling frequency = 24kHz
//*							  and we want to shift 6kHz
//*							  then ffreq = 6000/24000 * (1<<23) ( = 2097152 )
//*					
//***************************************************************************

//***************************************************************************
//*			*** Correct ***     Fixed Point Version 
//***************************************************************************

void CFreqCorrector::Correct(tNative *data_fxp,    // * Array to be shifteed in frequency *
							 tNativeAcc   ffreq,		 // * Shift value Base 23 *
							 tNativeAcc   total_symbol, // * Symbol length *
							 TSpecInvert  spec_invert)  // * Is inverted spectrum used? *
{ 
	tNative* restrict SinTable = m_SinTable;
	tNative i;
	tNative invert = 1;
	register tNativeAcc re,im;
	
	register tNativeAcc wre, wim;
	register tNativeAcc w0,w1;
	tNativeAcc quad;
	tNativeAcc phase;
	tNativeAcc phase_hires;
	tNativeAcc freq;
	
	quad = m_quadrant;
	phase_hires=m_phase;
	
//	invert = (spec_invert == on ? -1 : 1);

	if(spec_invert == pre_invert || spec_invert == post_invert )
		invert = -1;
	//pre_invert = (spec_invert == pre ? -1.0f : 1.0f);
	//pre_invert = (spec_invert == post ? -1.0f : 1.0f);

	//float freq = ffreq;

	if(spec_invert == post_invert) ffreq = -ffreq;



	//freq = (int) (ffreq * (SIN_TABLE_SIZE*4*(1<<EXTRABITS))); 
				//ffreq *     2^10       *4*      2^13        =ffreq * 33 554 432

	freq = NATIVE_CAST_ACC(ffreq * 4); // 1.0 * 2^23 represents the sampling frequency

	for (i=0; i < total_symbol; i++) //1600
	{
		phase = (phase_hires >> EXTRABITS) & (SIN_TABLE_SIZE-1);
		quad  = (phase_hires >> (LOG2_SIN_TABLE_SIZE + EXTRABITS)) & 0x3;
		
		re = data_fxp[2*i];
		w0 = SinTable[phase];
		im = invert*data_fxp[2*i+1];
		w1 = SinTable[SIN_TABLE_SIZE-1-phase];
		

		
		
		wre = ( (quad & 1) ? w0 : w1);
		wre = ( ((quad ^ quad>>1) & 1) ? -wre : wre);
		wim = ( (quad & 1) ? w1 : w0);
		wim = ( (quad & 2) ? -wim: wim);
		

		data_fxp[2*i] = (wre*re - wim*im) >> SIN_TABLE_BITS;
		data_fxp[2*i+1] = (wim*re + wre*im) >> SIN_TABLE_BITS;
		phase_hires += freq;
		phase_hires &= (1<<(2+LOG2_SIN_TABLE_SIZE + EXTRABITS))-1;
		
	}
	m_quadrant = quad;
	m_phase = phase_hires;
}

