// NotchFilter.cpp: implementation of the CNotchFilter class.
//
//////////////////////////////////////////////////////////////////////


#include "NotchFilter.h"
#include <stdio.h>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNotchFilter::CNotchFilter()
{	
	m_IIRStoreRe = m_IIRStoreIm = 0;
}

CNotchFilter::~CNotchFilter()
{
}


//***************************************************************************
//*
//*			Function name : *** Filter ***
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

void CNotchFilter::Filter(tNative *data, 
						  tNativeAcc freq, 
						  tNative NumSamples,
						  CDRMConfig *config)
{
	m_InputFreqShifter.Correct(data, -freq, NumSamples, no_invert);

	RecursiveHPF(data, NumSamples);

	m_OutputFreqShifter.Correct(data, freq, NumSamples, no_invert);
}



//***************************************************************************
//*
//*			Function name : *** RecursiveHPF ***
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

void CNotchFilter::RecursiveHPF(tNative *data, tNativeAcc NumSamples)
{
	tNative n  = 2*NumSamples;
	tNative sc = 16384;// 1<<14
	tNative k  = 500;   // * (short)(0.1f * (1<<14)); 
	tNative kc = sc - k; //14746;  // * (short)((1<<14)-k)
	tNative xin, yin;

	tNative StoreRe = m_IIRStoreRe;
	tNative StoreIm = m_IIRStoreIm;
	for (tNativeAcc i=0; i<n; i+=2)
	{
		xin = data[i];
		yin = data[i+1];
		StoreRe = NATIVE_CAST((k*xin+kc*StoreRe)/16384);
		StoreIm = NATIVE_CAST((k*yin+kc*StoreIm)/16384);
		data[i] =   xin-StoreRe;
		data[i+1] = yin-StoreIm;
	}
	m_IIRStoreRe = StoreRe;
	m_IIRStoreIm = StoreIm;

}





