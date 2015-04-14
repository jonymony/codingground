// PostInterpAFC.cpp: implementation of the CPostInterpAFC class.
//
//////////////////////////////////////////////////////////////////////

#include "PostInterpAFC.h"
#include <stdio.h>
#include <stdlib.h>
#include "../pc_types.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPostInterpAFC::CPostInterpAFC()
{
	m_oldChannel = new tNative[2*FFT_SIZE_MALLOC];

	for(int i=0; i< 2*FFT_SIZE_MALLOC; i++)
		m_oldChannel[i]=0;
}

CPostInterpAFC::~CPostInterpAFC()
{
	delete [] m_oldChannel;
}

//***************************************************************************
//*
//*			Function name : *** FindFreqErr ***
//*
//*			Description : 
//*
//*			Returns :
//*
//*			Input :
//*
//*			Output :
//*
//***************************************************************************



void CPostInterpAFC::FindFreqErr(tNative		* newChannel, 
					 CDRMConfig *config, 
					 tNative		&clock_err, // * Clock Frequency Error *
					 tNativeAcc		interferer_freq)// * fxp base 23 *
{
	tNative i;

	tNative accu_re_lhs=0,accu_im_lhs=0,accu_re_rhs=0,accu_im_rhs=0;
	tNative newre,newim,oldre,oldim; 
	tNative prod_re, prod_im;
	
	tNative fftsize = config->fftsize();
	tNative fspacing = config->fspacing();
	tNative fstep = config->fstep();
	
	tNative * oldChannel = m_oldChannel;

	tNative notch_start=-1000, notch_end=-1000;

	tNative AFC_NOTCH_RANGE = 82; // * 0.01 base 13 // +-240Hz 

	tNative interferer = NATIVE_CAST(interferer_freq>>10); 

	if(interferer > -(1<<13))
	{
		notch_start = 2*(fftsize/2 + NATIVE_CAST((fftsize * (interferer-AFC_NOTCH_RANGE))>>13) );
		notch_end   = 2*(fftsize/2 + NATIVE_CAST((fftsize * (interferer+AFC_NOTCH_RANGE))>>13) );
	}


	// * interpolated so use all pilot bearers *
	tNative imin = 2*(config->kmin() + config->dc_bin() ); 
	tNative imax = 2*(config->kmax()+ config->dc_bin() );
	tNative imid = 2*((config->kmax()+config->kmin())/2 + config->dc_bin() );

	// * Left hand side *
	for(i = imin;i < imid;i += 2*fstep)
	{
			newre = (newChannel[i]) >> 1;  // * input level down 1 bit *
			newim = (newChannel[i+1]) >> 1;// * input level down 1 bit *
			oldre = oldChannel[i];
			oldim = oldChannel[i+1];

			// * Multiply by conjugate *
			prod_re = NATIVE_CAST((newre * oldre + newim * oldim)>>15);
			prod_im = NATIVE_CAST((newim * oldre - newre * oldim)>>15);

			// * Accumulate the products *
			if (i<notch_start || i>notch_end)
			{
				accu_re_lhs += NATIVE_CAST prod_re;
				accu_im_lhs += NATIVE_CAST prod_im;	
			}
	}

	// * Right hand side *
	for(;i<imax;i+=2*fstep)
	{
			newre = (newChannel[i])>>1;  // * input level down 1 bit *
			newim = (newChannel[i+1])>>1;// * input level down 1 bit *
			oldre = oldChannel[i];
			oldim = oldChannel[i+1];

			// * Multiply by conjugate *
			prod_re = NATIVE_CAST((newre * oldre + newim * oldim)>>15);
			prod_im = NATIVE_CAST((newim * oldre - newre * oldim)>>15);

			// * Accumulate the products *
			if (i<notch_start || i>notch_end)
			{
				accu_re_rhs += NATIVE_CAST prod_re;
				accu_im_rhs += NATIVE_CAST prod_im;
			}
	}

	clock_err  = arctan2_fxp(accu_im_rhs, accu_re_rhs);
	clock_err -= arctan2_fxp(accu_im_lhs, accu_re_lhs);

	if( accu_im_lhs == 0 && accu_re_lhs ==0)
		clock_err=0;

	if( accu_im_rhs == 0 && accu_re_rhs ==0)
		clock_err=0;
	
	clock_err = NATIVE_CAST(( 163 * clock_err)>>9 ); 
	clock_err *= -4;


	for(i=0;i<2*fftsize;i++)
		oldChannel[i]=(newChannel[i]>>1);
}



//***************************************************************************
//*
//*			Function name : *** arctan2_fxp ***
//*
//*			Description : fast approximate arctan function
//*
//*			Returns : Returns arctan(im/re) in base 9 fixed point (rad * 2^9)
//*
//*			Input : 
//*
//*			Output :
//*
//*			Tree : Class::function
//*							\->Class::function
//*										\->Class::function
//***************************************************************************

tNative CPostInterpAFC::arctan2_fxp(tNative y, tNative x/*,short n*/)
{
//#define BITS 9
#define AT2_MULT(A,B) (NATIVE_CAST (((A*B)>>9)) )
	
	tNative	c1				= 101;//(tNative)((0.1963)*(1<<n));
	tNative	c2				= 503;//NATIVE_CAST((0.9817)*(1<<n));
	tNative	pi_over_4		= 402;//NATIVE_CAST((0.785398163)*(1<<n));
	tNative	three_pi_over_4	= 1206;//NATIVE_CAST((2.35619449)*(1<<n));

	tNative abs_y = SHORT_B(abs(y)) + 1;      // * kludge to prevent 0/0 condition * 

	tNative r;
	tNative angle;

	if (x>=0)
	{
		r = NATIVE_CAST(((x - abs_y)<<9) / (x + abs_y));
		angle = NATIVE_CAST (AT2_MULT( AT2_MULT(c1 , r) , AT2_MULT(r , r) ) - AT2_MULT( c2 , r ) + pi_over_4);
	}
	else
	{
		r = NATIVE_CAST(((x + abs_y)<<9) / (abs_y - x));
		angle = NATIVE_CAST ( AT2_MULT( AT2_MULT(c1 , r) , AT2_MULT(r , r)) - AT2_MULT( c2 , r ) + three_pi_over_4);
	}

	if (y < 0)
		return(-angle);     // * negate if in quad III or IV *
	else
		return(angle);
}
