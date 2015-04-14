// DCFilter.cpp: implementation of the CDCFilter class.
//
//////////////////////////////////////////////////////////////////////

#include "DCFilter.h"
#include <math.h>

#define DNF_INOUT_BITS 16	
#define DNF_MULTIPLY_SHIFT DNF_INOUT_BITS-1
#define DNF_ONE ((1<<DNF_MULTIPLY_SHIFT)-1)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDCFilter::CDCFilter()
{
	m_accumulator_re = 0;
	m_accumulator_im = 0;
}

CDCFilter::~CDCFilter()
{

}

//***************************************************************************
//*
//*			Function name : *** filter ***
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

void CDCFilter::filter(tNative *datafxp, tNative n)
{	
	tNative i=0;
	
	tNative k = 3277;       // *** 0.01*DNF_ONE; 
	tNative kc = DNF_ONE-k; // *** 1.0f-k;
	tNative acc_re = m_accumulator_re;
	tNative acc_im = m_accumulator_im;
	tNative re, im;
	
	for (i=0; i<2*n; i+=8)
	{
		re = datafxp[i];
		im = datafxp[i+1];
		acc_re = NATIVE_CAST((acc_re*kc + re * k)>>DNF_MULTIPLY_SHIFT);
		acc_im = NATIVE_CAST((acc_im*kc + im * k)>>DNF_MULTIPLY_SHIFT);
		datafxp[i] = re-acc_re;
		datafxp[i+1] = im-acc_im;

		re = -datafxp[i+3];
		im = datafxp[i+2];
		acc_re = NATIVE_CAST((acc_re*kc + re * k)>>DNF_MULTIPLY_SHIFT);
		acc_im = NATIVE_CAST((acc_im*kc + im * k)>>DNF_MULTIPLY_SHIFT);
		datafxp[i+3] = -(re-acc_re);
		datafxp[i+2] = im-acc_im;

		re = -datafxp[i+4];
		im = -datafxp[i+5];
		acc_re = NATIVE_CAST((acc_re*kc + re * k)>>DNF_MULTIPLY_SHIFT);
		acc_im = NATIVE_CAST((acc_im*kc + im * k)>>DNF_MULTIPLY_SHIFT);
		datafxp[i+4] = -(re-acc_re);
		datafxp[i+5] = -(im-acc_im);

		re = datafxp[i+7];
		im = -datafxp[i+6];
		acc_re = NATIVE_CAST((acc_re*kc + re * k)>>DNF_MULTIPLY_SHIFT);
		acc_im = NATIVE_CAST((acc_im*kc + im * k)>>DNF_MULTIPLY_SHIFT);
		datafxp[i+7] = re-acc_re;
		datafxp[i+6] = -(im-acc_im);


	}

	m_accumulator_re = acc_re;
	m_accumulator_im = acc_im;
}



