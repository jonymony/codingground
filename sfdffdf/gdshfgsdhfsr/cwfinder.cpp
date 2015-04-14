// CWFinder.cpp: implementation of the CCWFinder class.
//
//////////////////////////////////////////////////////////////////////

#include <math.h>
#include <stdio.h>
#include "CWFinder.h"


#include "../pc_types.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCWFinder::CCWFinder()
{
}

CCWFinder::~CCWFinder()
{
}

//***************************************************************************
//*
//*			Function name : *** FindCW ***
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

int CCWFinder::FindCW(tNative *pFFTOut, 
					  tNative *pFilteredSpectrum, 
					  CDRMConfig *pConfig,
					  Int16& CWBin)
{
	tNative i;
	tNative kmin = pConfig->kmin();
	tNative kmax = pConfig->kmax();
	tNative dc_bin = pConfig->dc_bin();
	tNative fftsize = pConfig->fftsize();
	tNative startbin = 0; 
	tNative endbin = fftsize*2;	

	tNativeAcc sum_sq = 0;
	
	tNative max_sq = 0;
	tNative max_sq_pos = 0;

	tNative re, im;
	tNative power,filt;

	// * Update the filtered power spectrum *
	for (i=startbin; i<endbin; i+=2)
	{
		re = NATIVE_CAST((pFFTOut[i])>>3);
		im = NATIVE_CAST((pFFTOut[i+1])>>3);
		power = NATIVE_CAST(( re*re + im*im )>>13);//watch this space
		
		filt = pFilteredSpectrum[i];
		filt += NATIVE_CAST((power-filt)>>3); //   * 0.125 = >>3
		pFilteredSpectrum[i] = filt;
		
		sum_sq += NATIVE_CAST_ACC filt;

		if (filt>max_sq)
		{
			max_sq = filt;
			max_sq_pos = i;
		}

	}

	max_sq_pos = max_sq_pos>>1;

	// * Now do some slicing to detect interferers *
	tNative thresh = NATIVE_CAST(sum_sq/(kmax-kmin+1)); // the mean power
	thresh = thresh*6; // something times the mean power sets the threshold level 

	// * For display purposes: show all bins that exceed the threshold *
	for (i=startbin; i<endbin; i+=2)
	{
		power = pFilteredSpectrum[i];
		pFilteredSpectrum[i+1] = (power>thresh ? NATIVE_CAST(-(1<<15)): thresh);
	}

	// * Is the biggest bin big enough to need filtering? *
	tNativeAcc InterfererFreq;
	
	if(max_sq>thresh)
	{
		tNativeAcc constant;

		switch(fftsize)
		{
		case(576): constant=14564; break;//(1<<23)*(1/fftsize)
		case(512): constant=16384; break;
		case(352): constant=23831; break;
		case(224): constant=37449; break;
		default  : constant=16384;
		}
				 

		InterfererFreq = (int)((max_sq_pos - (fftsize>>1)) * constant);//this should be base 23 fxp
		CWBin=max_sq_pos-fftsize/2;
		// NOTE we subtract fftsize/2 because this corresponds to DC in the complex
		// baseband signal. (But not necessarily to carrier k=0)
	}
	else 
		InterfererFreq = -(1<<23); // Means no interferer detected (valid range is +-1/2)

	
	return InterfererFreq;
}



