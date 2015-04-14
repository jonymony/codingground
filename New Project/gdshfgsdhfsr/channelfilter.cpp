// ChannelFilter.cpp: implementation of the CChannelFilter class.
//
//////////////////////////////////////////////////////////////////////


#include "ChannelFilter.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "../pc_types.h"

#define BIQUAD_SHIFT 9

//***************************************************************************
//*			*** Filter Constants ***     Fixed Point Version 
//***************************************************************************
const tBiquadCoeffs CChannelFilter::m_filterCoeffs[(MAX_SPEC_OCC+2)][FILTER_STAGES] =
{
	{
		{512,211,512,512,-870,382},
		{512,-634,512,512,-856,434},
		{512,-776,512,512,-845,479},
		{512,-809,512,512,-844,503}
	},
	{
		{512,296,512,512,-850,367},
		{512,-571,512,512,-826,427},
		{512,-729,512,512,-809,477},
		{512,-766,512,512,-805,503}
	},
	{
		{512,778,512,512,-689,272},
		{512,63,512,512,-540,376},
		{512,-205,512,512,-428,457},
		{512,-283,512,512,-387,498}
	},
	{
		{512,849,512,512,-653,254},
		{512,243,512,512,-462,364},
		{512,-36,512,512,-318,451},
		{512,-124,512,512,-261,496}
	},
	{
		{512,1003,512,512,-59,113},
		{512,896,512,512,456,367},
		{512,814,512,512,654,466},
		{512,782,512,512,716,501}
	},
	{
		{512,1017,512,512,168,119},
		{512,977,512,512,674,389},
		{512,942,512,512,830,473},
		{512,926,512,512,878,502}
	},
	{
		{512,-578,512,512,-926,425},
		{512,-927,512,512,-948,471},
		{512,-958,512,512,-961,499},
		{512,-964,512,512,-967,509}
	}
};


const tNative CChannelFilter::m_filterGains[(MAX_SPEC_OCC+2)] =
{
	7696,
	2592,
	2184,
	2488,
	2408,
	4032,
	10976

};

const tNative CChannelFilter::m_ShiftList[7][4]={
		{5,	2,	2,	1},
		{4,	2,	2,	0},
		{3,	2,	1,	0},
		{3,	2,	1,	0},
		{1,	0,	1,	0},
		{1,	0,	1,	0},
		{4,	2,	2,	0}
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CChannelFilter::CChannelFilter()
{
	m_filterIndex = 0;

	m_filterState=new tBiquadState[FILTER_STAGES];

	m_numStages = FILTER_STAGES;

	tNative i;

	for (i=0; i<FILTER_STAGES; i++)
	{
		m_filterState[i].w1re=0;
		m_filterState[i].w2re=0;
		m_filterState[i].w1im=0;
		m_filterState[i].w2im=0;
	}
}

CChannelFilter::~CChannelFilter()
{
	delete [] m_filterState;
}

//***************************************************************************
//*
//*			Function name : *** Filter ***
//*			
//*			Description:	
//*
//*
//* Spectral Occupacy 
//* | SpecOcc | filter bandwith |
//* -----------------------------
//* |    0    |     4.5  kHz
//* |    1    |     5.0  kHz
//* |    2    |     9.0  kHz
//* |    3    |     10.0 kHz
//* |    4    |     18.0 kHz
//* |    5    |     20.0 kHz
//* |    6    |     3.0  kHz (was used during mode detection)
//* -----------------------------
//* (These values are valid @ 24kHz sampling frequency)
//***************************************************************************

void CChannelFilter::Filter(tNative *inout, 
							const tNative NumSamples)
{

	tNative i;	

	tNative filterIndex=m_filterIndex;

	// *** Take the level down at the input (could be done elsewhere) ***
	if(filterIndex == 6)
		for(i=0;i<NumSamples*2;i++)
			inout[i]>>=4;
	else
		for(i=0;i<NumSamples*2;i++)
			inout[i]>>=3;
	
	tNative stage;
	tNative internal_down_shift;
	
	tNative * temp=new tNative[NumSamples*2];

	
	// *** First stage *** 
	
	stage=0;
	
	internal_down_shift=m_ShiftList[filterIndex][stage];

	BiquadFilter(inout,
				 temp, 
				 NumSamples, 
				 &m_filterCoeffs[filterIndex][stage], 
				 &m_filterState[stage],
				 internal_down_shift);

	
	// *** Second stage ***

	stage=1;

	internal_down_shift=m_ShiftList[filterIndex][stage];
		
	BiquadFilter(temp, 
				 inout,
				 NumSamples, 
				 &m_filterCoeffs[filterIndex][stage], 
				 &m_filterState[stage],
				 internal_down_shift);
	

	// *** Third stage ***

	stage=2;

	internal_down_shift=m_ShiftList[filterIndex][stage];
	
	BiquadFilter(inout,
					 temp, 
					 NumSamples, 
					 &m_filterCoeffs[filterIndex][stage], 
					 &m_filterState[stage],
					 internal_down_shift);
		

	
	// *** Fourth stage ***

	stage=3;
		
	internal_down_shift=m_ShiftList[filterIndex][stage];

	BiquadFilter(temp, 
					 inout,
					 NumSamples, 
					 &m_filterCoeffs[filterIndex][stage], 
					 &m_filterState[stage],
					 internal_down_shift);
	
	delete [] temp;
		
	tNative gain = m_filterGains[filterIndex];

	for (i=0; i<2*NumSamples; i++)
		inout[i]=(short)((inout[i]*gain)>>9);

}

//***************************************************************************
//*
//*			Function name : *** BiquadFilter ***
//*
//*			Description : 
//*
//*			Returns :
//*
//*			Input :
//*
//*			Output :
//*
//*			Tree : Class::function
//*							\->Class::function
//*										\->Class::function
//***************************************************************************

void CChannelFilter::BiquadFilter(const tNative *in, 
								  tNative *out, 
								  const tNative NumSamples, 
								  const tBiquadCoeffs *coeffs, 
								  tBiquadState *state,
								  tNative shifting)
{
	tNativeAcc w0re,w1re,w2re;
	tNativeAcc w0im,w1im,w2im;
	tNativeAcc a1n, a2n, b1;

	a1n = -coeffs->a1; 
	a2n = -coeffs->a2;
	
	w1re = state->w1re;
	w2re = state->w2re;
	w1im = state->w1im;
	w2im = state->w2im;

	b1 = coeffs->b1;

	for (tNative i=0; i<2*NumSamples; i+=2)
	{
		w0re = ((w1re*a1n + w2re*a2n) >> BIQUAD_SHIFT) + in[i]; 
		
		out[i] = NATIVE_CAST((w0re + ((w1re*b1) >> BIQUAD_SHIFT) + w2re)>>shifting);   // * New output

		

		w0im = ((w1im*a1n + w2im*a2n)>> BIQUAD_SHIFT) + in[i+1]; 

		out[i+1] = NATIVE_CAST((w0im + ((w1im*b1)>> BIQUAD_SHIFT) + w2im)>>shifting); // * New output

		// * Clock delay line *
		w2re = w1re; 
		w2im = w1im;  
		w1re = w0re; 
		w1im = w0im;
		
	}

	state->w1re = w1re;
	state->w2re = w2re;
	state->w1im = w1im;
	state->w2im = w2im;
}


void CChannelFilter::SetSpecOcc(tNative specOcc)
{
	if(specOcc < (MAX_SPEC_OCC+2) )
		m_filterIndex=specOcc;
}

int CChannelFilter::GetFilterBW()
{
	return m_filterIndex;
}
