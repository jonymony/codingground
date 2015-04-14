// CSI.cpp: implementation of the CCSI class.
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <math.h>
#include "CSI.h"


#include "../common/sample_rate.h"
#include "../pc_types.h"

#include "../TimeRefPos.h"
#include "../FACCellPos.h"
#include "../FreqRefPos.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//***************************************************************************
//*			*** Constructor ***     Fixed Point Version 
//***************************************************************************

CCSI::CCSI()
{
	m_unwgt_mer_msc = 0; 
	m_cell_cnt_msc = 0;
	m_sum_posteq_sq_err_msc = 0;
	m_wgt_mer_msc = 0; 
	m_sum_sq_chan_msc = 0;
	m_sum_preeq_sq_err_msc = 0;
	m_wgt_mer_fac = 0;
	m_sum_sq_chan_fac = 0;
	m_sum_preeq_sq_err_fac = 0;
}



CCSI::~CCSI()
{

}



//***************************************************************************
//*
//*			Function name : *** update_state ***
//*
//*			Description : Routine to calculate the channel state, 
//*						  avoid using mod operator
//*						  Works by scaling the constellation and 
//*						  finding the  boundary threshold by
//*						  casting the input data as an int
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
//*			*** update_state ***     Fixed Point Version 
//***************************************************************************

void CCSI::update_state(const tNative* data_in, // TOTAL_SYMBOL_COMPLEX_MALLOC
						const tNative* channel, // TOTAL_SYMBOL_COMPLEX_MALLOC
						tNative* channel_state, // FFT_SIZE_MALLOC
						tNative* mer,			  // SYMS_PER_FRAME_D (24)
						CDRMConfig* config,
						tNative symbolnum, 
						tNative framenum, 
						TFilt filt_comm, 
						TM_BOOL post_fft_timing,
						tNative& csi_mean, 
						tNative& csi_peak, 
						tNative& csi_peak_pos,
						TM_BOOL FAC_ready)
{	
	TMode mode=config->mode();
	tNative imode=(tNative) mode;

	TM_BOOL SDC_symbol=((symbolnum==0) | (symbolnum==1));

	if(mode==robust1 || mode==robust2) 
		SDC_symbol |= (symbolnum==2);
	
	tNative i,k;	
	tNative data_re, data_im;
	tNative thres_x,thres_y;
	tNative error_x,error_y,x,y;
	tNative msc_const_thres,sdc_const_thres;
	tNative average_fact=128; // * (0.125 * (1<<10))
	tNative kmin=config->kmin();
	tNative kmax=config->kmax();
	tNative fspacing=config->fspacing();
	tNative dc_bin=config->dc_bin();
	tNative next_pilot=config->firstpilot(symbolnum);
	tNative next_fac_cell=0, next_time_ref=0;

	// * these might need to be 32 bit (24 bit might be enough) *
	tNativeAcc sum_preeq_sq_err = 0;
	tNativeAcc sum_preeq_sq_carr = 0;
	tNativeAcc sum_posteq_sq_err = 0;
	tNativeAcc sum_posteq_sq_carr = 0;
	tNativeAcc sum_sq_chan = 0;
	tNativeAcc csi_mean_local=0;

	tNative csi_peak_local=0;
	tNative csi_peak_pos_local=0;
	tNative sq_err_cnt = 0;
	tNative fac_scale_factor;
	tNative sdc_scale_factor;
	tNative msc_scale_factor;
	tNative fac_scale_factor_inv;
	tNative sdc_scale_factor_inv;
	tNative msc_scale_factor_inv;

	// * initialize variables *

	// *** MSC ***

	// the scale factors are base 11
	// * 64-QAM *
	if(config->msc_constellation()==qam64)
	{
		msc_scale_factor = 1024;//0.5;
		msc_scale_factor_inv = 4096;//2;
		msc_const_thres = 3584;//(3.5*(1<<10))   3;
	}

	// * 16-QAM *
	else if(config->msc_constellation()==qam16)
	{
		msc_scale_factor = 500;//0.243975018;//sqrt(10)/2.0/sqrt(42);//
		msc_scale_factor_inv = 8394;//4.098780306;
		msc_const_thres = 1536;//(1.5*(1<<10))  1;
	}

	// * 4-QAM *
	else
	{
		msc_scale_factor = 223;//0.109108945;//sqrt(2)/2.0/sqrt(42);//
		msc_scale_factor_inv = 18770;//9.16515139;
		msc_const_thres = 512;//(0.5*(1<<10))   0;
	}

	// *** SDC ***

	// * 16-QAM *
	if(config->sdc_constellation()==qam16)
	{
		sdc_scale_factor = 500;//0.243975018;//sqrt(10)/2.0/sqrt(42);//
		sdc_scale_factor_inv = 8394;//4.098780306;
		sdc_const_thres = 1536;//(1.5*(1<<10))  1;
	}

	// * 4-QAM *
	else
	{
		sdc_scale_factor = 223;//0.109108945;//sqrt(2)/2.0/sqrt(42);//
		sdc_scale_factor_inv = 18770;//9.16515139;
		sdc_const_thres = 512;//(0.5*(1<<10))   0;
	}

	// *** FAC ***

	// * 4-QAM *
	fac_scale_factor = 223;//0.109108945;//sqrt(2)/2.0/sqrt(42);//
	fac_scale_factor_inv = 18770;//9.16515139;

	tNative  sq_carr;
	tUNativeAcc preeq_sq_carr;
	tUNativeAcc  preeq_sq_err, sq_err, preeq_scale;
	tNativeAcc chan_state=0;

	for(i=(dc_bin+kmin),k=kmin;k <= kmax;k++,i++)
	{
		// * ignore pilots *
		if(k!=next_pilot)
		{
			// * ignore dc and freq. refs. *
			if((k!=0) & (k!=freq_ref_pos[imode][0]) & (k!=freq_ref_pos[imode][1]) & (k!=freq_ref_pos[imode][2]))
			{
				if(!( (mode == ground) & ((k==1) | (k==-1)) ) )
				{
					// * ignore time refs *
					if(symbolnum==0 && k==time_ref_pos[imode][next_time_ref]) 
						next_time_ref++;

					else	// * It's FAC, SDC or MSC and we can calculate error *
					{
						tNative chan_re, chan_im;

						chan_re = channel[2*i];		//base 10
						chan_im = channel[2*i+1];	//base 10
						preeq_scale = NATIVE_CAST_UN_ACC((chan_re*chan_re + chan_im*chan_im)/(1<<10));//channel response mag sq
						
						if(k==fac_ref_pos[imode][symbolnum][next_fac_cell])
						{
							// * this is fac data *
							next_fac_cell++;
							
							// * scale the input data *
							data_re=(data_in[2*i]); 
							data_im=(data_in[2*i+1]);

							x=NATIVE_CAST((data_re*fac_scale_factor)>>11);// shift by 11 because scale factor is base 11
							y=NATIVE_CAST((data_im*fac_scale_factor)>>11);

							// * calculate errors *
							error_x=NATIVE_CAST((fac_scale_factor_inv*((x<0?-x:x)-512))>>11);
							error_y=NATIVE_CAST((fac_scale_factor_inv*((y<0?-y:y)-512))>>11);
							
							sq_err = ((error_x*error_x+error_y*error_y)>>10);
							
							m_sum_preeq_sq_err_fac += (sq_err * preeq_scale)>>10;
							m_sum_sq_chan_fac      += preeq_scale;				 

							
						}
						else if(!framenum & SDC_symbol)
						{
							// * this is sdc data *

							// * scale the input data *
							data_re=data_in[2*i]; 
							data_im=data_in[2*i+1];
							
							x=NATIVE_CAST((data_re*sdc_scale_factor)>>11);
							y=NATIVE_CAST((data_im*sdc_scale_factor)>>11);

							// * calculate the thresholds *
							if(sizeof(tNative) == 4)
							{
								thres_x=(x & 0xFFFFFC00)+512;// *mask off below the binary point then add a half* 
								thres_y=(y & 0xFFFFFC00)+512;
							}
							else
							{

							// * calculate the thresholds *
								thres_x=(x&0xFC00)+512;// *mask off below the binary point then add a half*
								thres_y=(y&0xFC00)+512;		
							}


							//* take account of data outside the constellation *
							if(thres_x > sdc_const_thres) 
								thres_x=sdc_const_thres;
							else if(thres_x  < -sdc_const_thres) 
								thres_x=-sdc_const_thres;

							if(thres_y > sdc_const_thres) 
								thres_y=sdc_const_thres;
							else if(thres_y < -sdc_const_thres) 
								thres_y=-sdc_const_thres;

							// * calculate errors *
							error_x = NATIVE_CAST((sdc_scale_factor_inv*(x-thres_x))>>11);
							error_y = NATIVE_CAST((sdc_scale_factor_inv*(y-thres_y))>>11);
							
							sq_err =((error_x*error_x+error_y*error_y)>>10);
						}
						else
						{
							// * must be msc data *
							// * scale the input data *
							data_re=data_in[2*i]; 
							data_im=data_in[2*i+1];
							
							x = NATIVE_CAST((data_re*msc_scale_factor)>>11); // base 11 relates to scale factor answer in base 10 
							y = NATIVE_CAST((data_im*msc_scale_factor)>>11);

							// * calculate the thresholds *
							if(sizeof(tNative) == 2)
							{
								thres_x=(x & 0xFC00)+512;// *mask off below the binary point then add a half*
								thres_y=(y & 0xFC00)+512;		
							}
							else
							{

								thres_x=(x & 0xFFFFFC00)+512;// *mask off below the binary point then add a half*
								thres_y=(y & 0xFFFFFC00)+512;	
							}

							// * take account of data outside the constellation *
							if(thres_x > msc_const_thres) 
								thres_x = msc_const_thres;
							else if(thres_x < -msc_const_thres) 
								thres_x = -msc_const_thres;
							
							if(thres_y > msc_const_thres) 
								thres_y = msc_const_thres;
							else if(thres_y < -msc_const_thres) 
								thres_y = -msc_const_thres;

							// * calculate errors *
							error_x = NATIVE_CAST((msc_scale_factor_inv*(x-thres_x))>>11);
							error_y = NATIVE_CAST((msc_scale_factor_inv*(y-thres_y))>>11);

							//sq_err=((error_x*error_x+error_y*error_y)>>10);

							tUNativeAcc err_x_sq = error_x*error_x;
							tUNativeAcc err_y_sq = error_y*error_y;

							err_x_sq = err_x_sq + err_y_sq;

							//sq_err = err_x_sq >> 12;
							sq_err = err_x_sq >> 10;

							//printf("err x %i err y %i sq %i %i \n", error_x, error_y, err_x_sq, sq_err);

							// * Multiply the error with the estimated channel responce to get the *
							// * noise back from before the channel eq							   *
							m_sum_preeq_sq_err_msc  += (sq_err * preeq_scale)>>10; // total mass of error and channel responce
							m_sum_sq_chan_msc       += preeq_scale;		           // total mass of channel responce
							m_sum_posteq_sq_err_msc += sq_err;					   // total mass of error
							m_cell_cnt_msc ++;									   // counter: nr of msc cells
						}

						// * Now we have error_x and error_y for FAC, SDC or MSC *
						// * square and add the errors then update the averager  *

						//there is an error here, need to shift by 16 for tNative type int
						//sq_carr = NATIVE_CAST((data_re*data_re+data_im*data_im) >> 10);
						//sq_carr = NATIVE_CAST((data_re*data_re+data_im*data_im) >> 16);

						tUNative t1 = data_re *data_re;
						tUNative t2 = data_im *data_im;

						t1 = t1+t2;

						t1 = t1 >>16;
						//t1 = t1 >>10;

						sq_carr = NATIVE_CAST ( t1);

						// * Calculate the squared error scaled by the channel power response *
						// * This gives the noise estimate on the pre-eq signal				  *

						preeq_sq_err  = NATIVE_CAST_ACC((sq_err  * preeq_scale) >> 10);
						preeq_sq_carr = NATIVE_CAST((sq_carr * preeq_scale) >> 10);

						//preeq_sq_err  = NATIVE_CAST_UN_ACC((sq_err  * preeq_scale) >> 12);
						//preeq_sq_carr = NATIVE_CAST_UN_ACC((sq_carr * preeq_scale) >> 12);


						// * Update the per-carrier filter (average across time)				*
						// * but only after we've got time sync or we'll fill the aray with NaN *
						if(!post_fft_timing)
						{
							average_fact=0;
							channel_state[i]=0;
						}
						
						// * channel_state is vital *
						chan_state = NATIVE_CAST_ACC( ( average_fact*preeq_sq_err + ((1<<10)-average_fact)*channel_state[i] )/(1<<10));

						// * clip channel state *
						if(chan_state > 32767)
							chan_state = 32767;
						
						// * base 10 fxp *
						channel_state[i] = chan_state;
						
						csi_mean_local += chan_state;

						if(chan_state > csi_peak_local)
						{
							csi_peak_local = chan_state;
							csi_peak_pos_local=k;
						}

						// * Average across freq (per-symbol) *
						sum_preeq_sq_carr  += preeq_sq_carr;
						sum_preeq_sq_err   += preeq_sq_err;
						sum_posteq_sq_err  += sq_err;
						sum_posteq_sq_carr += sq_carr;
						sum_sq_chan        += preeq_scale;
						sq_err_cnt++;
					
					} // A constellation bearing carrier
				}//if(mode != ground & (k!=1 | k!= -1) )
			}//if(k!=0 && k!=16 && k!=48 && k!=64{
		}//if(k!=next_pilot)
		else
		{
			next_pilot+=fspacing;
		}		
	}//for(i=(dc_bin+kmin)*2,k=kmin;k<=kmax;k++,i+=2)


	csi_mean_local-=csi_peak_local;
	csi_mean_local /= (kmax - kmin);

	short fft_size=config->fftsize();

	// * calculate the csi_peak_pos in Hz (was in kHz for flp version) AEGN *
	tNative carrier_space=OFDM_SAMPLE_RATE/(/*1000.0f*/fft_size);

	if(csi_mean_local > csi_mean) 
		csi_mean=csi_mean_local;

	if(csi_peak_local > csi_peak)
	{
		csi_peak=csi_peak_local;
		csi_peak_pos=csi_peak_pos_local;
		csi_peak_pos*=carrier_space;
	}
     
	
	// * Update the frame averaged MERs at the end of each frame *
	if (symbolnum == config->syms_per_frame()-1)
	{
		// *** NEW checks put in AEGN 13/04/04 ***
		if(m_sum_preeq_sq_err_msc==0)
		{
			m_wgt_mer_msc   = 0;
			m_sum_preeq_sq_err_msc=1;
		}
		else
		{
			m_wgt_mer_msc   = fxp_10_log10( NATIVE_CAST_UN_ACC((m_sum_sq_chan_msc * 42) / m_sum_preeq_sq_err_msc) );
		}
			
		if(m_sum_preeq_sq_err_fac==0)
		{
			m_wgt_mer_fac   = 0;
			m_sum_preeq_sq_err_fac=1;
		}
		else
		{
			m_wgt_mer_fac   = fxp_10_log10( NATIVE_CAST_UN_ACC((m_sum_sq_chan_fac * 42) / m_sum_preeq_sq_err_fac) );
		}
			
		// * fxp version. MER values given in fxp base 7 to get value divide by (1<<7) *
		
		
		if((m_sum_posteq_sq_err_msc>>10)==0)
		{
			m_unwgt_mer_msc =0;
			m_sum_posteq_sq_err_msc=1025;
		}
		else
		{
			m_unwgt_mer_msc = fxp_10_log10( NATIVE_CAST_UN_ACC((m_cell_cnt_msc    * 42) / (m_sum_posteq_sq_err_msc>>10)) );
		}

		mer[0] = m_wgt_mer_fac;
		mer[1] = m_wgt_mer_msc;
		mer[2] = m_unwgt_mer_msc;

		DP(("MER unwgt  %i\n", m_sum_posteq_sq_err_msc));

#ifdef _SIM
		float flp_mer=(float)m_unwgt_mer_msc/(float)(1<<7);
		DP(("\n\n\n\t\t\t\t\t*** MER *** 0"));
		for(int i=1;i<flp_mer;i++){if(!(i%10))DP(("%d",i)); else DP(("|"));}
		DP((" < %f dB >\n\n\n\n",flp_mer)); 
#endif

		m_cell_cnt_msc = 0;
		m_sum_posteq_sq_err_msc = 0;
		m_sum_sq_chan_msc = 0;
		m_sum_preeq_sq_err_msc = 0;
		m_sum_sq_chan_fac = 0;
		m_sum_preeq_sq_err_fac = 0;

	}
	
}



//***************************************************************************
//*
//*			Function name : *** CalcWeights ***
//*
//*			Description :   Calculates "goodness" according to csi and channel measurements.
//*							Currently just uses the csi. 
//*							Future improvements will use channel response (APR method)
//*							and per-symbol average CSI
//*							Assume we have CSI proportional to POWER NOISE TO SIGNAL RATIO
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

void CCSI::CalcWeights(const tNative *csi, 
					   const tNative *channel, 
					   tNative *weights, 
					   CDRMConfig* config)
{
	tNative kmin=config->kmin();
	tNative kmax=config->kmax();
	tNative dc_bin=config->dc_bin();
	tNative istart = dc_bin + kmin;
	tNative iend = dc_bin + kmax;
	tNative i;

	tNativeAcc weight; // * 32 bit *

	for (i=0; i<istart; i++) weights[i]=0;
	
	tNative scale = (1<<10);
	
	for (i = istart; i <= iend; i++)
	{
		tNative re = channel[i*2];
		tNative im = channel[i*2+1];
		
		tNative csi_in = csi[i];

		if(csi_in==0)
			csi_in=1;

		weight = (re*re + im*im) / csi_in; 

		// * clip weight *
//		if(weight>32767)
//			weight=32767;
		if(weight>16383)
			weight=16383;
		if(weight<0)
			weight=1;

		weights[i] = weight;
	}

	for (i=iend+1; i<FFT_SIZE_MALLOC; i++) 
		weights[i]=0;
}


//***************************************************************************
//*
//*			Function name : *** log10_fxp ***
//*
//*			Description : Function that calculates 10*log10(N) approximately
//*						  (dB conversion)
//*						  maximum input value 524200 ~= 57dB output
//*
//*			Returns :
//*
//*			Input : unsigned integer
//*
//*			Output : (the input in dB)*(1<<7) ; to get the real value divide by (1<<7) 
//*
//*			Tree : Class::function
//*							\->Class::function
//*										\->Class::function
//***************************************************************************

tNative CCSI::fxp_10_log10(tUNativeAcc in)
{
	if(in==0)
		return 0;

	tUNativeAcc mask=0x00040000;
//	unsigned int i;
	tNative i;

	for(i=18;((mask&in)==0) && i>=0;in<<=1,i--);

	tUNativeAcc mantissa=(0x3FFFF&in)>>11;
	
	return((3095*((i<<7)+mantissa))>>10); 
}
