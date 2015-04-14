// ImpulseResp.cpp: implementation of the CImpulseResp class.
//
//////////////////////////////////////////////////////////////////////


#include "ImpulseResp.h"
#include <math.h>
#include "../common/sample_rate.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../pc_types.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CImpulseResp::CImpulseResp()
{
	m_cir_old=new tNative[4*FFT_SIZE_MALLOC]; 
	memset(m_cir_old, 0, 4*FFT_SIZE_MALLOC*sizeof(tNative) );

	m_ir=new tNative[10*FFT_SIZE_MALLOC];
	memset(m_ir, 0, 10*FFT_SIZE_MALLOC*sizeof(tNative) );
	
	m_fft_out = new tNative[4*FFT_SIZE_MALLOC];

/*	m_WindowTable = new float[WINDOW_TABLE_SIZE];
	for (int i=0; i<WINDOW_TABLE_SIZE; i++)
	{
		m_WindowTable[i] = 0.5f+0.5f*cos(2.0f*PI*(i-WINDOW_TABLE_SIZE/2)/WINDOW_TABLE_SIZE);
	}*/
	
}

CImpulseResp::~CImpulseResp()
{
	delete [] m_cir_old;
	delete [] m_ir;
	delete [] m_fft_out;
	//delete [] m_WindowTable;
}




//***************************************************************************
//*
//*			Function name : *** FindImpResp ***
//*
//*			Description :	function for obtaining the channel impulse response. 
//*							Channel response is in data_in, impulse response placed in data_out.
//*							Length is number of complex points to work on. 
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

//***************************************************************************
//*			*** FindImpResp ***     Fixed Point Version 
//***************************************************************************

void CImpulseResp::FindImpResp(tNative		*data_in, 
							   tNative		*data_out, 
							   tNativeAcc			&phase_error, // * FreqErr *
							   tNativeAcc			length,
							   CDRMConfig	*config,  
							   tNative		&doppler)
{	
	tNative* cir_old=m_cir_old;

	tNative firstpilot			= 2 * ( config->dc_bin() + config->kmin() );
	tNative lastpilot				= 2 * ( config->dc_bin() + config->kmax() );
	tNative numpilotsonleft		= 2 * ( 1 + ( ( -config->pilot_index_symb0() - config->kmin() ) / config->fstep() ));
	tNative fft_size				= config->fftsize();
	tNative total_symbol			= fft_size + config->guard_interval(); 
	tNative total_symbol_complex	= 2 * total_symbol;
	tNative fstep					= 2 * config->fstep();
	tNative i,k;
		
//	tNative fft_out[4*FFT_SIZE_MALLOC];

	tNative* fft_out = m_fft_out;

	memset(data_out, 0, sizeof(tNative)* FFT_SIZE_MALLOC*4 );
	memset(fft_out , 0, sizeof(tNative)* FFT_SIZE_MALLOC*4 );

	// *** pack the time interpolated pilots next to each other ***
	// *** this is for modes B,C and D ***
	for( i=firstpilot , k=m_correct_factor * fft_size-numpilotsonleft ; i<=lastpilot ; i+=fstep , k+=2 )
	{
		data_out[k]   = NATIVE_CAST((data_in[i])>>3);//shift down input level 3 bits
		data_out[k+1] = NATIVE_CAST((data_in[i+1])>>3);//shift down input level 3 bits	
	}

	// *** window the frequency response
//	Window(&data_out[m_correct_factor*fft_size-numpilotsonleft], (lastpilot-firstpilot)/fstep+1);


	// *** swap the side bands before doing the FFT ***
	m_cir_fourier.SideBandSwap(data_out,length*m_correct_factor);

	(m_cir_fourier.*m_FFTFuncPtr)((fftw_complex*)data_out, (fftw_complex*)fft_out);


	if(config->mode() !=ground)
		memcpy(data_out, fft_out, sizeof(tNative) * fft_size*4);
	else
		for(i=0;i<2*m_correct_factor*fft_size;i+=2) 
		{
			data_out[2*i]=fft_out[i]; 
			data_out[2*i+1]=fft_out[i+1];
			data_out[2*i+2]=fft_out[i]; 
			data_out[2*i+3]=fft_out[i+1];
		}

	//	*** swap the sidebands back again to put t=0 in the middle of the data array ***
	m_cir_fourier.SideBandSwap(data_out,length*2);

	tNative re_old;
	tNative im_old;
	
	tNative re_new; 
	tNative im_new;

	tNative diff_demod_re;
	tNative diff_demod_im;

	tNativeAcc sum_diff_demod_re=0; 
	tNativeAcc sum_diff_demod_im=0; 

	// for monitoring
	// * For doppler spread calc. only for display. 
	// * lines marked " //disp "  are only for display purpose and not crucial to the DRM demodulation*
	tNative diff_re;		//disp
	tNative diff_im;		//disp
	tNativeAcc mag = 0;		//disp
	tNativeAcc total_mass = 0; //disp

	for(i=0; i< 4*fft_size; i+=2)
	{
		// *** do a diff demod between this and the previous ***
		re_old=cir_old[i];
		im_old=cir_old[i+1];

		re_new=data_out[i];
		im_new=data_out[i+1];

		// * Used to calculate the AFC error at the end *
		diff_demod_re = NATIVE_CAST ( ( re_new * re_old + im_new * im_old )/(1<<16) ) ;
		diff_demod_im = NATIVE_CAST ( ( -re_new* im_old + im_new * re_old )/(1<<16) ) ;

		sum_diff_demod_re += diff_demod_re;
		sum_diff_demod_im += diff_demod_im;

		// *** Used to estimate the doppler spread *** //for monitoring
		diff_re = re_new - re_old; //disp
		diff_im = im_new - im_old; //disp

		// * take the mag and add *
		mag        += NATIVE_CAST_ACC((diff_re * diff_re + diff_im * diff_im)>>2); //disp
		total_mass += NATIVE_CAST_ACC((re_new  * re_new  + im_new  * im_new ));    //disp
	}

	// * Check for div by 0 *
	if(mag==0) //disp
		mag=1;

	doppler = total_mass / mag ; //disp
	
	// * put a check on total mass  *
	if(total_mass == 0) //disp
		total_mass=1;	//disp

	// *** Use these lines to display the doppler value in Hz ***
	
//	float doppler_value = ((OFDM_SAMPLE_RATE/total_symbol) * sqrt((4.0f/(doppler)))/PI);
//	DP(("\nDoppler < %f Hz >\n",doppler_value));
	

	memcpy(cir_old, data_out, 4*fft_size*sizeof(tNative) );

//	sum_diff_demod_im <<= 4;
//	sum_diff_demod_re <<= 4;


	// *** Calculate the AFC error ***
	phase_error = NATIVE_CAST_ACC arctan2_fxp( sum_diff_demod_im , 
									sum_diff_demod_re );

//	printf("phase err %i %i %i \n", phase_error, sum_diff_demod_im, sum_diff_demod_re);
	
	// * convert from fixed point base 14 to base 23 *
	phase_error <<= 9 ;
}

//***************************************************************************
//*
//*			Function name : *** arctan2_fxp ***
//*
//*			Description : fast approximate arctan function
//*
//*			Returns : Returns arctan(im/re) in base 14 fixed point (rad * 2^14)
//*
//*			Input : 
//*
//*			Output :
//*
//*			Tree : Class::function
//*							\->Class::function
//*										\->Class::function
//***************************************************************************

tNative CImpulseResp::arctan2_fxp(tNativeAcc y, tNativeAcc x)
{
#define AT2_MULT(A,B) (NATIVE_CAST((A*B)>>14))
	
	tNative	c1				= 3216; //(tNative)((0.1963)*(1<<14));
	tNative	c2				= 16084;//(tNative)((0.9817)*(1<<14));
	tNative	pi_over_4		= 12867;//(tNative)((0.785398163)*(1<<14));
	tNativeAcc		three_pi_over_4	= 38603;//(int)((2.35619449)*(1<<14));

	//tNative abs_y = SHORT_B (abs(y)) + 1;      // * kludge to prevent 0/0 condition * 
	tNativeAcc abs_y = (abs(y)) + 1;      // * kludge to prevent 0/0 condition * 

	tNative r;
	tNative angle;

	if (x>=0)
	{
		r = NATIVE_CAST(((x - abs_y)<<14) / (x + abs_y));
		angle = NATIVE_CAST( AT2_MULT( AT2_MULT(c1 , r) , AT2_MULT(r , r)) - AT2_MULT( c2 , r ) + pi_over_4);
	}
	else
	{
		r = NATIVE_CAST(((x + abs_y)<<14) / (abs_y - x));
		angle = NATIVE_CAST( AT2_MULT( AT2_MULT(c1 , r) , AT2_MULT(r , r)) - AT2_MULT( c2 , r ) + three_pi_over_4);
	}

	if (y < 0)
		return(-angle);     // * negate if in quad III or IV *
	else
		return(angle);
}


//***************************************************************************
//*
//*			Function name : *** Search ***
//*
//*			Description : 
//*
//*	tNative			*data_in,			 * Channel impulse response (in) *
//*	tNative			*data_out,			 * Output for monitoring *
//*	int				&best_pos,			 * m_best_search_pos *
//*	CDRMConfig		*config, 
//*	tNative			&start,				 * for monitoring * 
//*	tNative			&end,				 * for monitoring * 
//*	tNative			&middle				 * for monitoring * 
//*
//***************************************************************************

void CImpulseResp::Search(tNative			*data_in, 
						  tNative			*data_out, 
						  tNativeAcc			&best_pos,     
						  CDRMConfig	*config,
						  tNative			&start,
						  tNative			&end,
						  tNative			&middle)
{
	tNative outlev=5;// * this is to set the level out (for display)
	tNative res=4;   // * this is to set the resolution of the input (4 is every 4 sample)

	tNative i=0;
	tNative fft_size=config->fftsize();
	tNative guard_interval=config->guard_interval();
	tNative fraction=(guard_interval*2*config->fstep()); 
	tNative j;

	// *** y is 32 bit unsigned (could be a problem with only 24 bits available) ***
	tUNativeAcc y=0; 
	int   a_up=0,a_down=0;
	tNative b_up=0,b_down=0;
	tNative temp;
	
	tNative* ir = m_ir;

	const tNative Nq=(4*config->fftsize())/res;

	memset(ir, 0, fft_size*sizeof(tNative) );

	// * here the input is taken down by 4 bits 
	// * (use this to change the input level to this block)
	for(i=fft_size,j=0;i<3*fft_size;i++,j+=2)
		ir[i]=NATIVE_CAST(data_in[j]>>4);
	

	memset(&ir[3*fft_size], 0, fft_size*sizeof(tNative)*6 );

	for(i=0;i<4*fft_size;i+=res)
	{
		b_up += ir[i];
		a_up += ir[i] + b_up;
		y    += ir[i] + 2*a_up - b_up;
	}

	tNativeAcc max_pos=0;
	tNativeAcc k;
	tNativeAcc pos=0;
	tNativeAcc max=0;

	for(i=0 ; i < fraction ; i+=res)
	{
		b_up  -= ir[i];
		a_up  += -ir[i]*Nq + b_up;
		y     += 2*a_up - b_up;
		
		temp = y>>16;///Nq_div;

		// * fill res nr of samples
		for(k=0;k<res;k++)
			data_out[pos+k] = temp;
		
		if(max < temp)
		{
			max=temp;
			max_pos=pos;
		}

		pos+=res;
	}
	
	
	for(i=fraction, j=0; i < 4*fft_size ; i+=res , j+=res )
	{
		b_up   -= ir[i];
		b_down += ir[j];
		a_up   += -ir[i] * Nq + b_up;
		a_down += ir[j]*Nq - b_down;

		y += -2 * a_down - b_down + 2 * a_up - b_up;
		
		temp = y>>16;

		// * fill res nr of samples
		for(k=0;k<res;k++)
			data_out[pos+k] = (temp);

		if(max < temp)
		{
			max=temp;
			max_pos=pos;
		}

		pos+=res;
	}


	// *** extra bit ***
	for( i=4*fft_size-fraction ; i<4*fft_size; i+=res )
	{
		b_down += ir[i];
		a_down += Nq * ir[i]  - b_down;
		y      += -2 * a_down - b_down;
		
		temp = y>>16;

		// * fill res nr of samples
		for(k=0;k<res;k++)
			data_out[pos+k] = temp; 
	
		if(max < temp)
		{
			max=temp;
			max_pos=pos;
		}

		pos+=res;
	}

	tUNative end_pos=0,start_pos=0;
 
	// * this allow you to change the noise threshold (for dev)
	tNative NOISE_THRES=max>>8; // (0.005*max)
	
	// *** find width and centre ***
	start_pos = max_pos;
	end_pos   = max_pos;

	while(data_out[start_pos] > (max - NOISE_THRES) && start_pos > 0)
	{
		start_pos--;
	}

	while(data_out[end_pos] > (max - NOISE_THRES) && end_pos < ( (FFT_SIZE_MALLOC * 6)) -1)
	{
		end_pos++;
	}
	
	best_pos= NATIVE_CAST_ACC( (1<<9) * (( (end_pos+start_pos)/2 )-(2*config->fftsize()+fraction/2 )));

	best_pos /= m_upsample_factor;

	// * for monitoring *
	start=start_pos;
	end=end_pos;
	middle=(end_pos+start_pos)/2;
}


void CImpulseResp::Config(CDRMConfig* config)
{
	int fft_size=config->fftsize();
	int guard_size=config->guard_interval();
//	int i;

	if(fft_size == 512)
	{
		m_upsample_factor=2*config->fstep();
		m_correct_factor=2;
		m_FFTFuncPtr=&(CFourier::ifft1024); 
	}
	else if(fft_size == 576)
	{
		// * This is 2* because we repeat the samples after the IFFT
		m_upsample_factor=2*config->fstep(); 
		m_correct_factor=1;
		m_FFTFuncPtr=&(CFourier::ifft576); 
	}
	else if (fft_size == 224)
	{
		m_upsample_factor=2*config->fstep();
		m_correct_factor=2;
		m_FFTFuncPtr=&(CFourier::ifft448); 
	}
	else
	{
		m_upsample_factor=2*config->fstep();
		m_correct_factor=2;
		m_FFTFuncPtr=&(CFourier::ifft704); 
	}

}

//***************************************************************************
//*
//*			Function name : *** FindImpRespMag ***
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

//***************************************************************************
//*			*** FindImpRespMag ***     Fixed Point Version 
//***************************************************************************

void CImpulseResp::FindImpRespMag(tNative *data_out, 
								  CDRMConfig* config, 
								  tNative& cir_peak, // for monitoring
								  tNative& cir_peak_pos)
{
	tNative bits=13;
	
	tNative i;
	
	tNative tempr1, tempi1, tempr2, tempi2;
	tNative tempr3, tempi3, tempr4, tempi4;

	tNative fft_size=config->fftsize();

	
	// *** take magnitude ***
	for(i=0;i<4*fft_size;i+=8)
	{
		tempr1=data_out[i];   tempr2=data_out[i+2];
		tempi1=data_out[i+1]; tempi2=data_out[i+3];
		tempr3=data_out[i+4]; tempr4=data_out[i+6];
		tempi3=data_out[i+5]; tempi4=data_out[i+7];

		data_out[i]  =NATIVE_CAST((tempr1*tempr1+tempi1*tempi1)>>bits);
		data_out[i+2]=NATIVE_CAST((tempr2*tempr2+tempi2*tempi2)>>bits);
		data_out[i+4]=NATIVE_CAST((tempr3*tempr3+tempi3*tempi3)>>bits);
		data_out[i+6]=NATIVE_CAST((tempr4*tempr4+tempi4*tempi4)>>bits);

		data_out[i+1]=0;
		data_out[i+3]=0;
		data_out[i+5]=0;
		data_out[i+7]=0;
	}

	// *** Just for monitoring ***
	tNative cir_peak_local=0;
	tNative cir_peak_pos_local=0;

	// *  find the peak *
	for(i=0;i< 4*fft_size;i+=2)
	{
		if(data_out[i] > cir_peak_local )
		{
			cir_peak_local =  data_out[i];
			cir_peak_pos_local = i;
		}
	}

	cir_peak_pos_local -= 2*fft_size;

	if(cir_peak_local > cir_peak )
	{
		cir_peak = cir_peak_local;
		cir_peak_pos= cir_peak_pos_local;
	}
	
	//**************************
}

/*
void CImpulseResp::Window(float *data, int NumComplex)
{
	float step = (float)(WINDOW_TABLE_SIZE-1)/(float)NumComplex;
	float pos = 0.0f;
	for (int i=0; i<NumComplex; i++)
	{
		float w = m_WindowTable[(int)pos];
		data[2*i] *= w;
		data[2*i+1] *= w;
		pos += step;
	}

} */
