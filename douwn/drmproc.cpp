// DrmProc.cpp: implementation of the CDrmProc class.
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include "DrmProc.h"
#include "../RxStatus.h"
#include "../S_Defs.h"
#include "../common/sample_rate.h"
#include "../pc_types.h"
#include "../message_types.h"
#include <stdlib.h>

#define TRIGGER_PULSE_WIDTH 100

#define SIGNAL_FAIL_COUNT_LONG 20
#define SIGNAL_FAIL_COUNT_SHORT 5

// *** Hand back (Pre/Post sync) threshold level ***
#define HAND_BACK_TIME_THRES 20
#define HAND_BACK_FREQ_THRES 20

#define INT_ON_COUNT 40

//#define PULL_DOWN -3300 // *** fxp base 23 ***  
//#define PULL_DOWN 0 // *** fxp base 23 ***  
//#define PULL_DOWN 699048 // for 50kHz sample rate
#define PULL_DOWN 2155406 //for 54.166 kHz sample rate


#pragma message(__FILE__": FIXED POINT")

//#pragma comment(linker, "-nodefaultlib:libmmt.lib -nodefaultlib:libcmt.lib ")
//#pragma message(__FILE__": telling linker to ignore libmmt.lib and libcmt.lib")

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDrmProc::CDrmProc()
{
	tNativeAcc i;
	m_notch_out = new tNative[TOTAL_SYMBOL_COMPLEX_MALLOC];	
	m_pFilteredSpectrum = new tNative[FFT_SIZE_MALLOC*2];
	memset(m_pFilteredSpectrum, 0, sizeof(tNative)*FFT_SIZE_MALLOC*2);
	m_fft_out = new tNative[TOTAL_SYMBOL_COMPLEX_MALLOC];
	m_fft_cw_out = new tNative[TOTAL_SYMBOL_COMPLEX_MALLOC];
	m_channel = new tNative[TOTAL_SYMBOL_COMPLEX_MALLOC];
	m_channel_state = new tNative[FFT_SIZE_MALLOC];
	m_weights = new tNative[FFT_SIZE_MALLOC];
	m_time_dat = new tNative[TOTAL_SYMBOL_COMPLEX_MALLOC];
	m_smoother_out=new tNative[TOTAL_SYMBOL_COMPLEX_MALLOC];
	m_sdc_data = new tFRAME_CELL[MAX_SDC_CELLS];
	m_correlations = new tNative[2*FREQ_OFFSET_RANGE+10];
	m_opdata = new tVitNative[MAX_MSC_PER_LFRAME * NUM_LEVELS * 2];
	m_sdcbits = new tVitNative[MAX_SDC_CELLS*4];
	m_mode_detect_monitoring = new tNative[TOTAL_SYMBOL_COMPLEX_MALLOC*2];
	m_imp_resp = new tNative[FFT_SIZE_MALLOC*4];
	m_search_output = new tNative[FFT_SIZE_MALLOC*6];
	m_fac_data = new tFRAME_CELL[MAX_FAC_CELLS];
	m_FACRawBits = new unsigned char[20]; 
	m_SDCRawBits = new unsigned char[300];

	m_mag_store= new tNative[TOTAL_SYMBOL_COMPLEX_MALLOC];
	memset(m_mag_store, 0, TOTAL_SYMBOL_COMPLEX_MALLOC * sizeof(tNative) );

	
//	m_local_signal_level=new short[SYMS_PER_FRAME_D];
	m_signal_level=new tNative[SYMS_PER_FRAME_D];
//	m_peak_level=new float[SYMS_PER_FRAME_D];
	m_MER=new tNative[SYMS_PER_FRAME_D];

	m_int_freq = new tNativeAcc[MAX_NO_INTERS];
	m_store_freq = new tNativeAcc[MAX_NO_INTERS];

	for(i=0; i<MAX_FAC_CELLS; i++)
	{
		m_fac_data[i].re=0;
		m_fac_data[i].im=0;
		m_fac_data[i].csi=0;
	}

	for(i=0; i<MAX_SDC_CELLS; i++)
	{
		m_sdc_data[i].re=0;
		m_sdc_data[i].im=0;
		m_sdc_data[i].csi=0;
	}

	m_pShortIn2=(tNative*) _cache_malloc(SINGLE_BUFFER_SIZE_UPSAMP, -1);
	m_pShortIn1=(tNative*) _cache_malloc(SINGLE_BUFFER_SIZE_UPSAMP, -1);


	m_raw_data_out_a = new tNative[TOTAL_SYMBOL_COMPLEX_MALLOC];
	m_raw_data_out_b = new tNative[TOTAL_SYMBOL_COMPLEX_MALLOC];

	memset(m_time_dat, 0, TOTAL_SYMBOL_COMPLEX_MALLOC*sizeof(tNative) ); 
	memset(m_smoother_out ,0, TOTAL_SYMBOL_COMPLEX_MALLOC*sizeof(tNative) ); 
	memset(m_fft_out, 0, TOTAL_SYMBOL_COMPLEX_MALLOC*sizeof(tNative) ); 
	memset(m_fft_cw_out, 0, TOTAL_SYMBOL_COMPLEX_MALLOC*sizeof(tNative) ); 
	memset(m_search_output, 0, FFT_SIZE_MALLOC*6*sizeof(tNative) );
	memset(m_imp_resp, 0, FFT_SIZE_MALLOC*4*sizeof(tNative) );

	memset(&m_misc_store, 0, sizeof(m_misc_store) );
	memset(&m_old_misc_store, 0, sizeof(m_old_misc_store) );


//	m_services_act[0] = 1;
//	m_services_act[1] = 0;

	m_FAC_Length=0;
	m_msc_deint_index=0;
	m_sdc_data_index=0;
	m_fac_check_sum_fails=0;
	m_sdc_check_sum_fails=0;
	m_frame_count=0;
	m_super_frame_count=0;
	m_mode_found=FALSE;

	m_old_service = 5;

	m_fft_symbol_count = 0;

	m_id_num =-1;

	m_tjump_flag=FALSE;
	m_fjump_flag=FALSE;
	m_sjump_flag=FALSE;
	m_prefft_flag=FALSE;
	m_postfft_flag=FALSE;
	m_mode_search_flag=FALSE;
	m_cw_flag=FALSE;
	m_bDrmOk=FALSE;
	m_new_mode=FALSE;

	m_min_FP=MAX_FREQ_PULL_FXP*2;
	m_max_FP=-MAX_FREQ_PULL_FXP*2;
	m_min_TCS=MAX_PULL_TIMING_LOOP*2;
	m_max_TCS=-MAX_PULL_TIMING_LOOP*2;
	
	m_csi_mean=0;
	m_csi_peak=0;
	m_csi_peak_mean=0;
	m_csi_peak_pos=0;

	m_cir_peak=0;
	m_cir_peak_pos=0;
	m_phase_err=0;
	m_cw_pos=0;

	m_SDC_Length=0;
	m_FAC_Length=0;


	m_buffer_1=TRUE;
	m_buff_1_pos=0;
	m_buff_2_pos=0;

	//set for sky mode
	if(RECEIVER_START_MODE == sky)
	{
		m_buff_1_size=TOTAL_SYMBOL_COMPLEX_B_UP;
		m_buff_2_size=TOTAL_SYMBOL_COMPLEX_B_UP;
	}
	else if(RECEIVER_START_MODE == ground)
	{
		m_buff_1_size=TOTAL_SYMBOL_COMPLEX_A_UP;
		m_buff_2_size=TOTAL_SYMBOL_COMPLEX_A_UP;
	}
	else if(RECEIVER_START_MODE == robust1)
	{
		m_buff_1_size=TOTAL_SYMBOL_COMPLEX_C_UP;
		m_buff_2_size=TOTAL_SYMBOL_COMPLEX_C_UP;
	}
	else
	{
		m_buff_1_size=TOTAL_SYMBOL_COMPLEX_D_UP;
		m_buff_2_size=TOTAL_SYMBOL_COMPLEX_D_UP;
	}


	m_hand_back_count_time=0;
	m_hand_back_count_freq=0;

//	m_raw_data_flag=0;

	m_signal_fail=0;

	for (i=0; i<TOTAL_SYMBOL_COMPLEX_UPSAMP_MALLOC; i++){
		m_pShortIn1[i] = 0;
		m_pShortIn2[i] = 0; 
	} 

	m_config.ReConfigureMode(RECEIVER_START_MODE);
	ConfigRec();

	m_bCoarseFreqError = TRUE;
	m_bPost_FFT_timing = FALSE;
	m_bSignalPresent = FALSE;
	m_bTrefOnFlag = FALSE;
	
	for(i=0;i<FFT_SIZE_MALLOC;i++) 
		m_channel_state[i]=0;

	m_doppler_shift=0;
	m_best_search_pos = 0; 
	m_clock_freq_err = 0; 
	m_jump=0;
	m_freq_pull=0; 
	m_afc_pull=0; 

	m_symbolnum=0;
	m_framenum=0;
	m_free_symbolnum=0;

	for(i=0; i< SYMS_PER_FRAME_D; i++) 
		m_MER[i]=0;


	m_fac_struct.Services[0].InUse=FALSE;
	m_fac_struct.Services[1].InUse=FALSE;
	m_fac_struct.Services[2].InUse=FALSE;
	m_fac_struct.Services[3].InUse=FALSE;
	m_fac_struct.FACReadyFlag=FALSE;
	m_sdc_struct.SDCReadyFlag=FALSE;
	m_sdc_struct.Type8.UTC=685; //10:45
	m_sdc_struct.Type8.JulianDate=51808; //21,September 2000


	for(i=0; i< MAX_NO_STREAMS;i++)
	{
		m_sdc_struct.Type0.DataLen[i].datalenA=0;
		m_sdc_struct.Type0.DataLen[i].datalenB=0;
	}
	m_sdc_struct.Type0.streams=0;

	for(i=0;i<MAX_NO_SERVICES;i++)	m_sdc_struct.Type6[i].multiplex=0;
	for(i=0;i<MAX_NO_SERVICES;i++)	m_sdc_struct.Type6[i+MAX_NO_SERVICES].multiplex=1;

	m_sdc_decoder.Reset(m_sdc_struct);

	m_interferer_freq = -(1<<23);

	m_bInterFlag = TRUE;

	ResetLang();

	m_test1=NULL;

	for(i=0; i< SYMS_PER_FRAME_D ;i++)
	{
		m_signal_level[i]=0;
	}

	DP(("created DrmProc\n"));

}

CDrmProc::~CDrmProc()
{
	delete [] m_mode_detect_monitoring;
	delete [] m_pFilteredSpectrum;
	delete [] m_notch_out;
	delete [] m_fft_out;
	delete [] m_fft_cw_out;
	delete [] m_mag_store;
	delete [] m_channel;
	delete [] m_channel_state;
	delete [] m_weights;
	delete [] m_time_dat;
	delete [] m_smoother_out;
	delete [] m_sdc_data;
	delete [] m_correlations;
	delete [] m_opdata;
	delete [] m_sdcbits;
	delete [] m_imp_resp;
	delete [] m_search_output;
	delete [] m_fac_data;
	delete [] m_FACRawBits; 
	delete [] m_SDCRawBits;
	delete [] m_raw_data_out_a;
	delete [] m_raw_data_out_b;
	delete [] m_signal_level;
	delete [] m_MER;

	delete [] m_int_freq;
	delete [] m_store_freq;

	_cache_free (m_pShortIn1);
	_cache_free (m_pShortIn2);

	DP(("Deleted DRM Proc %i \n", m_id_num));
}


/**************************************************************************

		Function name : *** main_process ***

		Description : This is THE Main Process
					  This function takes an IQ signal with 
					  reference frequency at -5khz

short			*data_in_fxp,     * Input DRM data IQ at -5kHz reference * 
									(TOTAL_SYMBOL_COMPLEX_UPSAMP_MALLOC)

int				&msc_data_index,  * MSC data index *
tFRAME_CELL		*msc_data_cells,  * MSC data streams *
TM_BOOL&		monit_ready,	  * Monitor array ready *
MonitorType		*trigger_dat,     * Monitor array with trigger pulses *					
int				&posteq_framenum, * frame number *
int				&posteq_symbolnum,* symbol number *
TM_BOOL			band_scan,		  * Band scan ? *
TM_BOOL			&drm_found,		  * DRM found ? *
UInt8			*change_flags)    * flags change (rest from CAB but could be useful) *

**************************************************************************/

TM_BOOL CDrmProc::main_process(tNative			*data_in_fxp, 
							   tNativeAcc				&msc_data_index, 
							   tFRAME_CELL		*msc_data_cells,
							   TM_BOOL&			monit_ready, 
							//   MonitorType		*trigger_dat, 
							   tNative			&posteq_framenum, 
							   tNative			&posteq_symbolnum,
							   TM_BOOL			band_scan, 
							   TM_BOOL			&drm_found, 
							   UInt8			*change_flags,
							   tNative			*DRMLevel,
							   tUNativeAcc		sample_rate_dev)
							   
{
		WritePC(1000);

//		short		i;
		TMode		local_mode				= unknown;
		TLoop		time_loop_control		= m_config.timing_control();
		TLoop		freq_loop_control		= m_config.frequency_control();
		TFilt		filt_comm				= m_config.filtering();
		TMLC		mode_detect				= m_config.mode_detect_stat();
		TM_BOOL		valid_block;
		tNative		fft_size				= m_config.fftsize();
		tNative		guard_interval			= m_config.guard_interval();
		tNative		total_symbol			= fft_size + guard_interval;
		tNative		total_symbol_complex	= 2 * total_symbol;
		tNative		kmin					= m_config.kmin();
		tNative		kmax					= m_config.kmax();

		// *** trigger array pointer ***
//		MonitorType* restrict trigger	= &trigger_dat[m_free_symbolnum*total_symbol_complex*2];
	
		if(m_jump != 0)
		{
			m_tjump_flag=TRUE;
		}
		
		// * Timing adjust variable * 
		tNative timing_shift = 0; //* 32bit * fxp 

		// *** Don't add the timing pull in search mode ***
		if (m_mode_found || mode_detect!=on)
			timing_shift += m_freq_pull;
		else 
		{
			m_jump = 0;
			timing_shift = 0;
		}


		// *** Resampling ***
		if(m_upsamp.resamp(sample_rate_dev+timing_shift, m_jump,data_in_fxp, total_symbol))
		//if(m_upsamp.resamp(PULL_DOWN+timing_shift, m_jump,data_in_fxp, total_symbol, &m_config))
		{

//			DP(("timing %i \n", timing_shift));
			
			WritePC(1010);
			
			// Monitoring *Initialise*
//			m_monitor.ClearTrigger();
//			m_monitor.GenerateTrigger();
//			m_monitor.ClearMonit();
		
			WritePC(1020);	

//			DP(("timing %i \n", timing_shift));

			// Monitoring *Raw Time Data* 
			m_monitor.RawTimeData(data_in_fxp);	

			// *** Find the Signal Level ***
		//	SignalLevels(data_in_fxp, m_free_symbolnum, total_symbol_complex);

			WritePC(1030);

			// *** Pre FFT process
			Pre_FFT_process(data_in_fxp,
							DRMLevel,
							band_scan, 
							drm_found);

			WritePC(2000);

			// * define a new starting point for the array indexing *
			// * move half a guard interval in						*
			
			// *** FFT start for the fixed point array ***
			tNative* fft_start_fxp=data_in_fxp+2*guard_interval;
			
			m_shift=0;

			// * remove the guard interval by copying the first half of the *
			// * guard interval to the end of the active symbol				*
			Guard_remove(data_in_fxp, m_shift);
			
			WritePC(2110);

			// *** Do the FFT (This is the main line in the whole DRM module) ***
			FFT_process(fft_start_fxp); 
			

			WritePC(2200);

			// Monitoring *FFTData* 
			//m_monitor.FFTData(fft_start_fxp);

			// *** Find CW interferer ***
/*			m_interferer_freq = m_cw_finder.FindCW(fft_start_fxp, 
												   m_pFilteredSpectrum, 
												   &m_config,
												   m_cw_pos);
*/
			
			
			// Monitoring *FFT filtered mag sq* 
			m_monitor.FilteredFFTMagSq(m_pFilteredSpectrum);

			

			WritePC(2210);

			// *** n Symbol Delay ***
			tNative *olddata = m_symbol_delay.getsymbolptr(m_config.tspacing());

			

			// *** Post FFT process *** 
			Post_FFT_process(fft_start_fxp, 
							 msc_data_index);
			

			WritePC(2300);

			tNativeAcc msc_data_length=0;
	
			
			
			// *** Channel Process ***
			Channel_process(fft_start_fxp, 
							olddata,
							posteq_symbolnum,
							posteq_framenum, 
							msc_data_length, 
							msc_data_index,
							msc_data_cells);


			

			
			WritePC(2400);

			TMode mode;
			
			// *** MLC process ***
			MLC_process(posteq_symbolnum, 
						posteq_framenum, 
						monit_ready, 
						mode, 
						change_flags);

			WritePC(2500);
			
			m_free_symbolnum++;
			
			if(m_free_symbolnum >= m_config.syms_per_frame())
			{
				monit_ready = TRUE;
				m_free_symbolnum=0;
			}
			
			m_symbolnum++;

			if(m_symbolnum >= m_config.syms_per_frame())
			{
				m_symbolnum=0;
				m_framenum++;
				if (m_framenum >= FRAMES_PER_SFRAME)
				{
					m_framenum=0;
					
					// * write a super frame trigger pulse *
				//	for(i=0; i<TRIGGER_PULSE_WIDTH; i+=2)
					{
				//		trigger[i]+=10000;
					}
				}
				
				//write a frame trigger pulse
				//for(i=0; i<TRIGGER_PULSE_WIDTH; i+=2)
				{
				//	trigger[i]+=10000;
				}
			}

			valid_block = TRUE;

		}//if(m_upsamp.resamp(PULL_DOWN+timing_shift, m_jump, data_in_fxp, total_symbol, &m_config))
		else
		{
			m_jump=0;
			valid_block = FALSE;
		}

		WritePC(2600);
		

		return(valid_block);
}



//***************************************************************************
//*
//*			Function name : *** Guard_remove ***
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

void CDrmProc::Guard_remove(tNative	*data_in, 
							tNativeAcc		start_pos)
{
	tNativeAcc i;
	tNativeAcc fftsize=m_config.fftsize();
	tNativeAcc guard_interval=m_config.guard_interval();

	for(i=guard_interval+2*start_pos;i<2*guard_interval;i++){
		data_in[i+2*fftsize]=data_in[i];
	}
}


void CDrmProc::Config_Change_Command(TConfig_Command *config_command)
{

	m_config.command(config_command);

	// *** if mode changes will need to reconfigure DRMProc for the new mode ***
	if(config_command->operation==set_mode){
		m_config.SetAutoDetect(off);
		
		ConfigRec();
	}
	if(config_command->operation==reset_CRC_count) ResetCRCCount();

	if(config_command->operation==set_mode_detect){
			if(config_command->value == (int) on ) ReacquireMode(RECEIVER_START_MODE);
	}

}



//************************************************************************************
//*
//*		Function name	: *** find_COM ***
//*
//*		Description		: Finds The Centre of Mass
//*
//*		Returns:	void 
//*
//*		Input:		data_in					Input vector
//*					post_fft_flag			Flag post FFT
//*					m_best_search_pos		Hold the best position for search
//*					m_config				Holds current configuration
//*
//*		Output:		pos_of_com				Centre of Mass position
//*					m_hand_back_count_time  Counter for the handover/handback
//*					
//*
//*		Tree : CDRMProc::main_process
//*						\->CDRMProc::Pre_FFT_Process
//*									\->CDRMProc::find_COM
//************************************************************************************

void CDrmProc::find_COM(tNative	*data_in,
						tNative   &pos_of_com,
						TM_BOOL	&post_fft_flag)
{	
	tNative i;	
	tNative guard_interval       = NATIVE_CAST(m_config.guard_interval());
	tNative fft_size             = NATIVE_CAST(m_config.fftsize());
	tNative fstep                = NATIVE_CAST(m_config.fstep());
	tNative total_symbol         = NATIVE_CAST(fft_size + guard_interval);
	tNative total_symbol_complex = NATIVE_CAST(2 * total_symbol);

	tNativeAcc	moment_of_mass1	= 0; // * 32 bit *
	tNativeAcc	total_mass1		= 0; // * 32 bit *

	tNativeAcc	moment_of_mass2	= 0; // * 32 bit *
	tNativeAcc	total_mass2		= 0; // * 32 bit *

	tNativeAcc pos_of_com1     = 0; // * 32 bit *
	tNativeAcc pos_of_com2     = 0; // * 32 bit *
	
	// *** need to do this in two halves to take account of edge effects ***
	
	// *** First half (0 to total_symbol) ***
	for( i=0 ; i < total_symbol ; i+=2)
	{
		total_mass1 += NATIVE_CAST_ACC data_in[i];
		moment_of_mass1 += NATIVE_CAST_ACC (data_in[i] * i);
	}

	// *** Second half (total_symbol to total_symbol_complex) ***
	for( i=total_symbol ; i < total_symbol_complex ; i+=2)
	{
		total_mass2 += NATIVE_CAST_ACC data_in[i];
		moment_of_mass2 += NATIVE_CAST_ACC(data_in[i] * i);
	}

	moment_of_mass1 >>= 1; 
	if(total_mass1 == 0) total_mass1 =1;
	pos_of_com1 = moment_of_mass1/total_mass1;

	moment_of_mass2 >>= 1;
    if(total_mass2 == 0) total_mass2 =1;
	pos_of_com2 = moment_of_mass2/total_mass2;


	// *** check to see if the "peak" wraps round the edge ***
	if(total_mass1 > total_mass2)
	{
		pos_of_com = pos_of_com1;
	}
	else
	{
		pos_of_com = pos_of_com2;
	}

	if(pos_of_com < 0) 
		pos_of_com = 0;

	tNative start_pos = 2 * ( NATIVE_CAST(pos_of_com) - (fft_size>>1) );
	tNative end_pos   = 2 * ( NATIVE_CAST(pos_of_com) + (fft_size>>1) );

	total_mass1=0;
		moment_of_mass1=0;
	
	// *** If you are before the symbol start ***
	if(start_pos < 0)
	{
		for( i=start_pos+total_symbol_complex ; i < total_symbol_complex ; i+=2)
		{
			total_mass1 += NATIVE_CAST_ACC data_in[i];
			moment_of_mass1 += NATIVE_CAST_ACC(data_in[i] * (i-total_symbol_complex));
		}
		for( i=0 ;i < end_pos ; i+=2 )
		{
			total_mass1 += NATIVE_CAST_ACC data_in[i];
			moment_of_mass1 += NATIVE_CAST_ACC(data_in[i] * i);
		}
	}
	
	// *** if you are after the symbol ***
	else if( end_pos > total_symbol_complex )
	{
		for( i=start_pos ; i < total_symbol_complex ; i+=2 )
		{
			total_mass1 += NATIVE_CAST_ACC data_in[i];
			moment_of_mass1 += NATIVE_CAST_ACC(data_in[i]*i);
		}
		for(i=0;i < (end_pos-total_symbol_complex); i+=2)
		{
			total_mass1 += NATIVE_CAST_ACC data_in[i];
			moment_of_mass1 += NATIVE_CAST_ACC (data_in[i]*(i+total_symbol_complex));
		}
	}

	// *** if you are within the symbol *** 
	else
	{
		for( i = start_pos ; i < end_pos ; i+=2 )
		{
			total_mass1 += NATIVE_CAST_ACC data_in[i];
			moment_of_mass1 += NATIVE_CAST_ACC (data_in[i]*i);
		}
	}

	moment_of_mass1 >>= 1;
	if(total_mass1 == 0) total_mass1 =1;
	pos_of_com1 = moment_of_mass1/total_mass1;

	pos_of_com = pos_of_com1;
	
	if(pos_of_com > total_symbol) 
		pos_of_com -= total_symbol;

	tNative avg_spread = 5;

	tNativeAcc sum_im=0; // * 32 bit *
	tNativeAcc sum_re=0; // * 32 bit *


	for( i = NATIVE_CAST_ACC pos_of_com*2 - 2*avg_spread ; i <= NATIVE_CAST_ACC pos_of_com*2 + 2*avg_spread ; i+=2)
	{
		if( i > 0 && i < total_symbol_complex )
		{ 
			sum_im += NATIVE_CAST_ACC data_in[i+1];
			sum_re += NATIVE_CAST_ACC data_in[i];
		}
	}

	if(pos_of_com > total_symbol/2)
			pos_of_com -= total_symbol;


	// *** This handback mechanism can lock out in an alias position for long echoes ***
	//if(pos_of_com < -fft_size/fstep/2 || pos_of_com > fft_size/fstep/2)
	//	m_hand_back_count_time++;
	//else m_hand_back_count_time=0;

	// *** Alternative handback based on difference between pre and post ***
	tNative pre_post_diff = NATIVE_CAST(pos_of_com - (m_best_search_pos>>9));
		
	// * Could use half design TW = gi+20% *
	tNative pre_post_diff_thresh = guard_interval>>1; 

	if(pre_post_diff < -pre_post_diff_thresh || pre_post_diff > pre_post_diff_thresh)
		m_hand_back_count_time++;
	else 
		m_hand_back_count_time=0;
	
	if( m_hand_back_count_time >= HAND_BACK_TIME_THRES) 
		post_fft_flag=FALSE;

	// *** final check ***
	if(pos_of_com > total_symbol/2 || pos_of_com < -total_symbol/2) 
		pos_of_com=0;

}




//***************************************************************************
//*
//*			Function name : *** ConfigRec ***
//*
//*			Description :	place here any initialisations required 
//*							when starting the receiver
//*
//*			Returns :
//*
//*			Input :
//*
//*			Output :
//*
//*			Tree : 
//*
//***************************************************************************

void CDrmProc::ConfigRec()
{
	m_bPost_FFT_timing	=	FALSE;
	m_bSignalPresent	=	FALSE;

	m_loopfilter.FilterReset();
	m_freqloopfilter.Reset();

	m_fac_check_sum_fails	=	0;
	m_sdc_check_sum_fails	=	0;
	m_frame_count			=	0;
	m_super_frame_count		=	0;
	m_symbolnum				=	0;

	m_tref_finder.Reset();

	m_mode_detect.Reset();
	
	if(m_config.mode() == sky)
	{
        m_FFTFuncPtr=&(CFourier::fft512); 
		m_channel_filter.SetSpecOcc(0);
		m_time_proc.Reset(FFT_SIZE_B*2);
	}
	else if(m_config.mode() == ground)
	{
        m_FFTFuncPtr=&(CFourier::fft576);
		m_channel_filter.SetSpecOcc(0);
		m_time_proc.Reset(FFT_SIZE_A*2);
	}
	else if(m_config.mode() == robust1)
	{
        m_FFTFuncPtr=&(CFourier::fft352);
		m_channel_filter.SetSpecOcc(3);
		m_time_proc.Reset(FFT_SIZE_C*2);
	}
	else
	{
        m_FFTFuncPtr=&(CFourier::fft224);
		m_channel_filter.SetSpecOcc(3);
		m_time_proc.Reset(FFT_SIZE_D*2);
	}

	m_cir.Config(&m_config);

	m_signal_fail = 0;

	m_free_symbolnum = 0;

	m_services = 0;
	m_old_services = 0;

	ResetLang();

	ResetSDC();
	
	ResetFAC();

	// *** clear the csi output ***
	memset(m_channel_state, 0, FFT_SIZE_MALLOC*sizeof(tNative) );

	memset(&m_service_label_store, 0, sizeof(m_service_label_store));


	for(tNativeAcc i=0; i< MAX_NO_SERVICES; i++)
	{
		m_old_programme_store[i].Prog =0;
		m_old_service_ident[i].ID=0;
		m_old_service_ident[i].ID2=0;
		m_old_service_ident[i].ID3=0;
		m_audio_store[i].audio=0;
		m_old_audio_store[i].audio=0;
		m_old_programme_store[i].service =i;
		m_old_service_ident[i].service=i;
		m_service_label_store[i].service=i;
		m_audio_store[i].service=i; 
		m_old_audio_store[i].service=i;

	}

	// *** clear the input buffers ***
	memset(m_pShortIn1, 0, TOTAL_SYMBOL_COMPLEX_UPSAMP_MALLOC*sizeof(tNative) );
	memset(m_pShortIn2, 0, TOTAL_SYMBOL_COMPLEX_UPSAMP_MALLOC*sizeof(tNative) );
}

TM_BOOL CDrmProc::ReacquireMode(TMode mode)
{
	// *** always swicth off the MLC when reacquiring a new mode ***
	m_config.SetMLCStat(off);

	if(m_config.mode_detect_stat() == on){
		
		m_mode_found=FALSE;
		m_config.ReConfigureMode(RECEIVER_START_MODE);
		m_config.SetAutoDetect(RECEIVER_START_MODE_DETECT);
		m_signal_fail =0;
		m_bDrmOk=FALSE;
		m_mode_detect.Reacquire();
		m_mode_detect.Reset();
		m_sdc_decoder.Reset(m_sdc_struct);
		ChangeMode(RECEIVER_START_MODE);   
		ConfigRec();

		return TRUE;
	}
	else ConfigRec();

	return FALSE;


}

tNative CDrmProc::ReadTCS()
{
	return (m_freq_pull);
}

UInt16 CDrmProc::ReadStatus()
{
	UInt16 status=0;

	if(m_bPost_FFT_timing)	status |=POST_FFT_TIME_SYNC;

	if(m_bSignalPresent) status |=SIGNAL_PRESENT;

	if(m_bCoarseFreqError) status |= COARSE_FREQ_ERR;

	if(m_config.mlc_stat() == on) status |= MSC_MLC_ON;

	return(status);
}


void CDrmProc::ResetCRCCount()
{
	m_frame_count=0;
	m_super_frame_count=0;
	m_fac_check_sum_fails=0;
	m_sdc_check_sum_fails=0;
}


/**************************************************************************

			Function name : *** Pre_FFT_process ***

			Description : 

			Tree : CDrmProc::main_process
										\->CDrmProc::Pre_FFT_process


**************************************************************************/

void CDrmProc::Pre_FFT_process(tNative		*data_in_fxp,
							   tNative*		DRMLevel,
							   TM_BOOL		band_scan, 
							   TM_BOOL		&drm_found)
{
	WritePC(4000);

	TMode		local_mode			= unknown;
	TMLC		mode_detect			= m_config.mode_detect_stat();
	TLoop		time_loop_control	= m_config.timing_control();
	TLoop		freq_loop_control	= m_config.frequency_control();
	TFilt		filt_comm			= m_config.filtering();

	
	TSpecInvert spec_invert = no_invert;
	if(m_config.invert() == on) spec_invert = pre_invert;

	tNative fft_size=m_config.fftsize(); 
	tNative guard_interval=m_config.guard_interval();
	tNative total_symbol=fft_size+guard_interval; 
	tNative total_symbol_complex=2*total_symbol;

	WritePC(4010);
    	
	// *** find the frequency shift to make signal symmetric ***
	// *** fixed point base 23 ***
	tNativeAcc shift = m_config.fshift_to_sym_fxp(); // * 32 bits * 

//	spec_invert =on;

	if(spec_invert == pre_invert)
		shift = -m_config.fshift_to_sym_fxp_invert();

//	DP(("AFC %i\n", m_afc_pull));

	// *** Don't add the AFC pull in search mode ***
	if (m_mode_found || mode_detect!=on)
		shift += m_afc_pull;

//	printf("pull %f \n", 24000.0f * m_afc_pull / (8388608) );

	
	// *** Shift Centre of Spectrum to DC ***
	m_freqcorrector_pre_filt.Correct(data_in_fxp, shift, total_symbol, spec_invert);
	WritePC(4015);
	
	// *** Channel Filter *** 
	if(m_config.channel_filter() == on)  
		m_channel_filter.Filter(data_in_fxp, 
								total_symbol);
	WritePC(4020);

	// *** find the frequency shift to move k0 to dc ***
	shift = m_config.fshift_to_dc_fxp();

	// *** shift back here so that k0 is always at dc ***
	m_freqcorrector_post_filt.Correct(data_in_fxp, -shift, total_symbol, no_invert);

	WritePC(4025);

	// *** Apply DC notch filter in Simulcast Mode ***  
	if (m_config.channel_filter() == on && (m_config.spec_occ() == 0 || m_config.spec_occ() == 1))
	{
		m_dc_filter.filter(data_in_fxp, total_symbol);
	}

	WritePC(4030);

	// *** Find the Signal Level ***
	SignalLevels(data_in_fxp, m_free_symbolnum, total_symbol_complex);
	
	WritePC(4035);

#ifdef CWDETECT_ON

	tNative* restrict fft_cw_out=m_fft_cw_out;

	tNative* restrict fft_start=data_in_fxp; 
	// *** FFT start for the floating point array ***
	fft_start=data_in_fxp+2*guard_interval;
	
	m_fft_symbol_count++;

	m_int_freq[0] = -(1<<23);
	m_int_freq[1] = -(1<<23);
	//do a fourier transform to detect interferers on a side chain
	//perhaps run this every tenth symbol say, to save cycles
	if( (m_fft_symbol_count % 10) == 2)
	{
		(m_symbol_fourier_cw.*m_FFTFuncPtr)((fftw_complex*)fft_start, (fftw_complex*)fft_cw_out);//FLP

		//do a sideband swap to make identification easier??
		//remove later to save cycles
		m_symbol_fourier_cw.SideBandSwap(fft_cw_out, fft_size);

	}

		tNative bin1 = 0, bin2 = 0;		
		//now look for an interferer
		m_CWInter.Detect(fft_cw_out, m_mag_store, fft_size, m_int_freq, &bin1, &bin2);

		DP(("bin %i %i\n", bin1, bin2));

		//correct for frequency shift now done in detect
		//freq -= 0.25f;


	//DP(("freq %f \n", freq));

	//freq *= 2;

	//freq = 0.0f;

	//now get rid of the interferer on the main signal path
	//if the interferer is on the edge this could turn on and off
	//this is bad so apply some hysteresis to stop this
	//need to store the frequency that we were using in m_store_freq
	static int int_on1 = 0;
	static int int_on2 = 0;

	if(m_int_freq[0] > -(1<<23))
	{
		m_store_freq[0] = m_int_freq[0];

		int_on1 ++;
		if(int_on1 > INT_ON_COUNT)
			int_on1 = INT_ON_COUNT;
	}
	else
	{
		int_on1 --;
		if(int_on1 < 0)
			int_on1 = 0;
	}

	if(m_bInterFlag && int_on1 > 1)
	{
		
		m_notch_filter1.Filter(data_in_fxp, 
		//m_notch_filter2.Filter(m_mag_store, 
							  m_int_freq[0], 
							  total_symbol,
							  &m_config);

		DP(("notching %i %i\n", m_int_freq[0], int_on1));
	}
	else
	{
		DP(("not notching %i %i\n", m_int_freq[1], int_on1));
	}

	if(m_int_freq[1] > -(1<<23))
	{
		m_store_freq[1] = m_int_freq[1];

		int_on2 ++;
		if(int_on2 > INT_ON_COUNT)
			int_on2 = INT_ON_COUNT;
	}
	else
	{
		int_on2 --;
		if(int_on2 < 0)
			int_on2 = 0;
	}

	if(m_bInterFlag && int_on2 > 1)
	{
		
		m_notch_filter1.Filter(data_in_fxp, 
		//m_notch_filter2.Filter(m_mag_store, 
							  m_int_freq[1], 
							  total_symbol,
							  &m_config);

		DP(("notching %i %i\n", m_int_freq[1], int_on2));
	}
	else
	{
		DP(("not notching %i %i\n", m_int_freq[1], int_on2));
	}


	//monitor the detector
	m_monitor.FFTData(m_mag_store);
#endif


	// *** m_notch out goes to mode detection and pre FFT time sync. data_in_fxp goes to FFT ***
	memcpy(m_notch_out, data_in_fxp, total_symbol_complex * sizeof(tNative));
	
	// *** Apply notch filter if an interferer has been detected ***
	/*if(m_interferer_freq > -(1<<23))
	{
		m_notch_filter.Filter(m_notch_out, 
							  m_interferer_freq, 
							  total_symbol,
							  &m_config);

		m_cw_flag=TRUE;
	}*/

	WritePC(4036);

	tNative best_level =0;


	// *** Detect Mode (A,B,C or D) ***
	if(!m_mode_found && mode_detect==on)
	{
		// ** MODE_SEARCH_FILTER Decides which filter bandwith to use during mode detection **
		m_channel_filter.SetSpecOcc(MODE_SEARCH_FILTER);    

		// ** Function DetectMode returns false until same mode has been detected 20 times in a row **
		// ** and writes the detected mode to local_mode **
		
		// Monitoring * Look here *
		tNative monit_buf_len=AUDIO_POINTS_STEREO/m_config.syms_per_frame();
		tNative monit_points=monit_buf_len/NUMBER_OF_CHANNELS;

		
		TM_BOOL recon=m_mode_detect.DetectMode(m_notch_out, 
											   m_mode_detect_monitoring, 
											   monit_points, 
											   local_mode, 
											   &m_config,
											   &best_level);
	

/*****
 * ~ *
 *****/
		// Monitoring *Smoothed Correlation* (During Mode Detection)
		m_monitor.SmoothedCorrelationModeDetect(m_mode_detect_monitoring);
	
		WritePC(4037);

		drm_found = recon;
		
		WritePC(4040);
		
		m_mode_search_flag=TRUE;
		
		// ** If the mode is found with confidence of 20 symbols **
		if(recon && band_scan == false) //lock out the reconfig when band scanning
		//if(recon) 
		{
			DP(("\n                           %i: Found mode\n", m_id_num));
			// * Set flag for mode is found *
			m_mode_found=TRUE; 
			// * reconfigure to the new mode *
			m_config.ReConfigureMode(local_mode);
			// * Change buffer sizes to the new mode *
			ChangeMode(local_mode);
			// * Re-Initialize the receiver for the new mode *
			ConfigRec();  
		}
	//	else
	//		DP(("%i COULD NOT GET MODE\n", m_id_num));

		
		WritePC(4050);
	}

	WritePC(4060);

	/******************************************************************************
	      Pre FFT time syncronisation 
	******************************************************************************/

	// *** Correlate Data ***
	m_time_proc.time_correlate(m_notch_out, 
							   m_time_dat, 
							   fft_size, 
							   guard_interval,
							   &m_config);

/*****
 * ~ *
 *****/

	// Monitoring *Correlated Time Data*
	m_monitor.CorrelatedTimeData(m_time_dat);

	WritePC(4070);
	
	// *** Filter Data ***
	m_time_proc.filter(m_time_dat, 
					   &m_config, 
					   fft_size, 
					   guard_interval);

	
		
	WritePC(4080);
	
/*****
 * ~ *
 *****/

	// Monitoring *Filtered Correlation*
	m_monitor.FilteredCorrelation(m_time_dat);

	WritePC(4090);

	
	
	// *** Smooth Data ***
	m_time_proc.smoother(m_time_dat, 
						 m_smoother_out, 
						 filt_comm, 
						 fft_size, 
						 guard_interval,
						 &m_config);

	//need to take into account filter BW
	float correct = 20000 / ReadFilterBW();

	tNative peak_pos;

	//float drm_level = 0;
	*DRMLevel = (m_time_proc.peak_find(m_smoother_out, fft_size, guard_interval,
							&m_config, peak_pos) ) * correct;


	if(!m_mode_found && mode_detect==on)
		*DRMLevel = best_level * correct;
	
	
	WritePC(4100);

/*****
 * ~ *
 *****/

	// Monitoring *Smoothed Correlation*
	if(!(!m_mode_found && mode_detect==on))
		m_monitor.SmoothedCorrelation(m_smoother_out);

	WritePC(4110);

	// *** Restore Data ***
	m_time_proc.restore(m_time_dat, 
						m_smoother_out, 
						fft_size, 
						guard_interval,
						&m_config);	
	
	WritePC(4120);

	tNative COM_pos=0; 
	
	// *** Find Centre Of Mass ***
	find_COM(	m_time_dat,			// * input array *
				COM_pos,			// * Centre of mass position *
				m_bPost_FFT_timing);

//	DP(("COM %i \n", COM_pos));

	WritePC(4130);

	
	tNative peak_pos_ang=0; // *  PRE FFT fine AFC error * ( base 13 fxp )

	// *** Angle (AFC error) ***
	find_ang(	m_smoother_out, // * input array *
				COM_pos,   // * Centre of mass position *
				peak_pos_ang);	// * PRE FFT fine AFC error * 


//	printf("ang %f \n", 180*peak_pos_ang / (8192.0f * PI) ); 

	float temp_ang = 180.0f*peak_pos_ang / (8192.0f * PI);

	m_monitor.AFC(peak_pos_ang , m_afc_pull);
//	m_monitor.AFC(peak_pos_ang , 0);

 
	WritePC(4140);

/*****
 * ~ *
 *****/

	// Monitoring *Restored Correlation*
	m_monitor.RestoredCorrelation(m_time_dat,COM_pos,m_best_search_pos);

	// *** Use difference between pre and post ***
	TM_BOOL  afc_out_of_range = false;
		
	// *** Calculate difference btwn pre and post FFT AFC error ****

	// *** Set AFC range ***
	tNative afc_range; // * fixed point base 13 * 
	// +/-5% fu design BW (2*PI*fft_size/total_symbol * 0.05 * 2^13)
	switch(fft_size)
	{
		case 576: afc_range=2316; break;
		case 512: afc_range=2058; break;
		case 352: afc_range=1887; break;
		case 224: afc_range=1441; break;
		default:  afc_range=2058;
	}

	// * m_phase_error from base 23 to base 13 *
	tNative afc_pre_post_diff = peak_pos_ang - (short)(m_phase_err>>10); 

	// *** if the difference is out of range ***
	if(afc_pre_post_diff > afc_range || afc_pre_post_diff < -afc_range)
	{
		m_hand_back_count_freq++; 
		afc_out_of_range = true;
	}
	else 
		m_hand_back_count_freq=0;

	if(m_hand_back_count_freq >= HAND_BACK_FREQ_THRES)
		m_bPost_FFT_timing = FALSE;

//	DP(("pull %f diff %i count %i ang %i\n", m_afc_pull/8192, afc_pre_post_diff, m_hand_back_count_freq, peak_pos_ang));

	WritePC(4150);

	// *** turn off this bit after achieving crash lock and switch to fine control ***
	if(time_loop_control==loop_disable || m_bPost_FFT_timing==FALSE)
	{	
	// *** Timing loop filter *** Pre FFT TIMING LOOP FILTER

		m_freq_pull=m_loopfilter.FilterCoarse(COM_pos, 
											  afc_out_of_range, 
											  m_jump, 
											  m_bPost_FFT_timing,
											  &m_config, 
											  m_bTrefOnFlag);
	
		if(m_freq_pull < m_min_TCS) 
			m_min_TCS=m_freq_pull;

		if(m_freq_pull > m_max_TCS) 
			m_max_TCS=m_freq_pull;
	}

	WritePC(4160);

	if(m_bPost_FFT_timing)
		m_postfft_flag=TRUE;
	else
		m_prefft_flag=TRUE;

	// *** PRE FFT AFC ***
	if(freq_loop_control==loop_disable || m_bPost_FFT_timing == FALSE)
	{
		// *** Pre FFT AFC loop filter ***
		m_afc_pull=m_freqloopfilter.Filter(	peak_pos_ang,	// * Pre FFT fine AFC error *
											filt_comm);		

		// *** record the min frequency pull value ***
		if( m_afc_pull < m_min_FP ) 
			m_min_FP=m_afc_pull;   

		// *** record the max frequency pull value ***
		if( m_afc_pull > m_max_FP ) 
			m_max_FP=m_afc_pull;

//		sprintf(string, "pre freq %f", (float) m_afc_pull);
		
	}

	WritePC(4170);

}





//**************************************************************************
//*
//*			Function name : *** Post_FFT_process ***
//*
//*			Description : 
//*
//*			Tree : CDRMProcess::main_process
//*							\->CDRMProcess::Post_FFT_process
//*										
//**************************************************************************

void CDrmProc::Post_FFT_process(tNative* data_in,  
								tNativeAcc& msc_data_index)
{
	tNative* restrict fft_start=data_in;

	tNative fftsize=m_config.fftsize();
	tNative guard_interval=m_config.guard_interval();
	tNative total_symbol_complex=(guard_interval+fftsize)*2;

	WritePC(6000);

	TLoop time_loop_control=m_config.timing_control();
	TLoop freq_loop_control=m_config.frequency_control();
	TFilt filt_comm=m_config.filtering();

	WritePC(6010);

	
	TM_BOOL is_symbol0=FALSE;

	tNativeAcc coarse_freq_err=0;

	// *** Find the timing references ***
	m_tref_finder.find_refs(fft_start, 
							m_correlations, 
							coarse_freq_err, 
							m_bPost_FFT_timing, 
							is_symbol0, 
							m_interferer_freq, 
							&m_config);

//	DP(("coarse error %i\n", coarse_freq_err));

	if(coarse_freq_err == 0) 
		m_bCoarseFreqError = TRUE; 

	if (is_symbol0)
	{
		// * If the symbol number has jumped then reset the data indices to *
		// * prevent writing off the end of the array in a 'long' frame     *
		if (m_symbolnum != 0)	
								
		{
			msc_data_index = 0;
			m_sdc_data_index = 0;
			m_sjump_flag=TRUE;
		}

		m_symbolnum = 0;
	}

	WritePC(6020);
	
	// *** Coarse Frequency Jump ***
	if(coarse_freq_err !=0)
	{ 
		m_fjump_flag=TRUE;

		m_freqloopfilter.CoarseJump(-coarse_freq_err, &m_config);
	}


	WritePC(6030);

/*****
 * ~ *
 *****/

	// Monitoring *Time Reference Finder*
	m_monitor.TimeReferenceFinder(m_correlations); 

	WritePC(6040);


	// *** UnTwist (Undo the phase rotations) ***
	m_channeleq.UnTwist(fft_start, 
						&m_config, 
						m_symbolnum);



	WritePC(6050);

	WritePC(6060);

	WritePC(6070);		
}





//**************************************************************************
//*
//*			Function name : *** MLC_process ***
//*
//*			Description : FAC and SDC decoding
//*
//*			Tree : CDRMProcess::main_process
//*							\->CDRMProcess::MLC_process
//*	
//**************************************************************************

void CDrmProc::MLC_process(tNative		posteq_symbolnum, 
						   tNative		posteq_framenum, 
						   TM_BOOL		&monit_ready, 
						   TMode		&mode, 
						   UInt8		*change_flags)
{
	tNative index=0;
	tNative i;
	tNative fftsize=m_config.fftsize();
	tNative guard_interval=m_config.guard_interval();
	tNative total_symbol_complex=2*(fftsize+guard_interval);
	tNative service=m_config.service_selected();


	WritePC(8000);
	
	// *** MLC decoding for FAC ***
	if(posteq_symbolnum==m_config.syms_per_frame()-1)
	{
		WritePC(8001);

		tNativeAcc numnetbits, numnetbitsVSPP;
		
		tMLCDefinition MLCDef = m_config.fac_mlc_definition();
		
		m_mlc_decoder.SetCodeRate(MLCDef, m_config.fac_cell_per_frame());

		WritePC(8002);
		
		// *** MLC decoding ***
		m_mlc_decoder.Decode(m_fac_data, m_opdata, numnetbits, numnetbitsVSPP);

		WritePC(8004);
		
		// *** Energy dispersal ***
		m_energy_dispersal.Dispersal(m_opdata, numnetbits);

		WritePC(8010);
		
		// *** FAC decode ***
		if(m_fac_decoder.FACDataDecode(m_opdata, m_fac_struct))
		{
			WritePC(8020);

			m_framenum=m_fac_struct.Frame_num+1;
			
			if(m_framenum >= FRAMES_PER_SFRAME) 
				m_framenum=0;
			
			m_free_symbolnum=m_symbolnum;
			
			DP(("%i FAC check sum passed\n", m_id_num));
			
			m_frame_count++;
			
			m_fac_struct.FACReadyFlag=TRUE;
			
			m_bTrefOnFlag=FALSE;
			
			m_config.ReConfigureFromFAC(&m_fac_struct);
			
			// * we don't want to change the filter if we are still looking for the mode
			// * as it causes spurious output from the filter
			if(m_mode_found) 
				m_channel_filter.SetSpecOcc(m_config.spec_occ());

			m_FAC_Length=PackBits((unsigned char*) m_opdata, m_FACRawBits, numnetbits, 0);

			CheckServiceID(change_flags);
	
			if(change_flags[1] & NEW_SERVICE_ID)
	
			{
					//clear the fac and sdc contents
			//		ResetFACService(service);
			//		ResetSDC();
			//		for(i=0; i< MAX_NO_SERVICES; i++)
			//			m_old_service_ident[i].ID=0;
			}

			CheckProgramme(change_flags);

			m_signal_fail=0;
			m_bDrmOk=TRUE;


			WritePC(8025);
			
		}

		else
		{
            DP(("%i FAC check sum FAILED\n", m_id_num));
			WritePC(8030);
			m_fac_struct.FACReadyFlag=FALSE;
			// * swicth off the services *
			for(i=0;i<MAX_NO_SERVICES;i++)	
				m_fac_struct.Services[i].InUse=FALSE;

			m_fac_check_sum_fails++;
			m_frame_count++;
			m_signal_fail++;

		}

		WritePC(8050);
		if(!m_frame_count) m_fac_check_sum_fails=0;

	}

	WritePC(8052);
	
	if(m_bDrmOk && m_signal_fail >= SIGNAL_FAIL_COUNT_SHORT)
	{
		ReacquireMode(m_config.mode() );

		// *** set the flag to indicate a change ***
		change_flags[0] |= NEW_AUDIO_SERVICES_MASK;
		change_flags[1] |= NEW_LABEL_MASK;
		change_flags[1] |= NEW_LANGUAGE_MASK;
		change_flags[1] |= NEW_SERVICE_ID;
		change_flags[1] |= NEW_PROGRAMME_MASK;
		change_flags[1] |= NEW_AUDIO_MASK;

		for(i=0; i< MAX_NO_SERVICES; i++)
		{
			
			strcpy((char*) m_service_label_store[i].label, "no sig");

			// *** set the flag to indicate a change ***
			change_flags[i+2] |= NEW_LABEL_MASK;
			change_flags[i+2] |= NEW_LANGUAGE_MASK;
			change_flags[i+2] |= NEW_SERVICE_ID;
			change_flags[i+2] |= NEW_PROGRAMME_MASK;
			change_flags[i+2] |= NEW_AUDIO_MASK;
		}
	
	}

	if(!m_bDrmOk && m_signal_fail >= SIGNAL_FAIL_COUNT_LONG)
	{
		ReacquireMode(m_config.mode() ) ;

		// *** set the flag to indicate a change ***
		change_flags[0] |= NEW_AUDIO_SERVICES_MASK;
		change_flags[1] |= NEW_LABEL_MASK;
		change_flags[1] |= NEW_LANGUAGE_MASK;
		change_flags[1] |= NEW_SERVICE_ID;
		change_flags[1] |= NEW_PROGRAMME_MASK;
		change_flags[1] |= NEW_AUDIO_MASK;

		for(i=0; i< MAX_NO_SERVICES; i++)
		{
			
			strcpy((char*) m_service_label_store[i].label, "no sig");

			//*** set the flag to indicate a change ***
			change_flags[i+2] |= NEW_LABEL_MASK;
			change_flags[i+2] |= NEW_LANGUAGE_MASK;
			change_flags[i+2] |= NEW_SERVICE_ID;
			change_flags[i+2] |= NEW_PROGRAMME_MASK;
			change_flags[i+2] |= NEW_AUDIO_MASK;
		}
		
	}

	WritePC(8054);
	
	
	// *** MLC Decoding for SDC ***
	if (posteq_framenum==0 && posteq_symbolnum==2)
	{

		WritePC(8055);
		
		tMLCDefinition MLCDef = m_config.sdc_mlc_definition();
		
		m_mlc_decoder.SetCodeRate(MLCDef, m_config.sdc_cell_per_frame());
		
		tNativeAcc numnetbits, numnetbitsVSPP;

		WritePC(8056);

		

		// *** MLC decoding ***
		m_mlc_decoder.Decode(m_sdc_data, m_opdata, numnetbits, numnetbitsVSPP);
		
		WritePC(8057);

//		memset(m_sdcbits, 0, MAX_SDC_CELLS);
		
		// *** UnDeal ***
		m_mlc_decoder.UnDeal(m_opdata, m_sdcbits, m_config.sdc_cell_per_frame());
		
		WritePC(8058);
		
		// *** Energy Dispersal ***
		m_energy_dispersal.Dispersal(m_sdcbits, numnetbits);

		WritePC(8060);
		
		tNativeAcc checksum;
		
		// *** SDC data decode ***
		if(m_sdc_decoder.SDCDataDecode(m_sdcbits, m_sdc_struct, numnetbits, checksum, modulationDRM) )
		{
			WritePC(8065);
			DP(("%i SDC check sum passed\n", m_id_num));
			m_sdc_struct.SDCReadyFlag=TRUE;
			m_config.SetMLCStat(on);
			m_super_frame_count++;
			m_config.ReConfigureFromSDC(&m_sdc_struct);

			CheckLabels(change_flags);

			SetLang();

			CheckLang(change_flags);
			
			SetAudioStore();

			CheckAudio(change_flags);

			CheckMisc(change_flags);

			// * moved from the FAC decode *
			if(SetServices() )
				// * set the flag to indicate a change *
				change_flags[0] |= NEW_AUDIO_SERVICES_MASK;

			m_SDC_Length=PackBits((unsigned char*) m_sdcbits, m_SDCRawBits, numnetbits, 4);
			WritePC(8070);
						
		}
		
		else
		{
            DP(("%i SDC check sum FAILED\n", m_id_num));
			m_sdc_struct.SDCReadyFlag=FALSE;
			m_sdc_check_sum_fails++;
			m_super_frame_count++;

			WritePC(8080);
		}

		if(!m_super_frame_count) 
			m_sdc_check_sum_fails=0;

		WritePC(8100);
	}
	
	
	WritePC(8200);

}




//**************************************************************************
//*
//*			Function name : *** Channel Process ***
//*
//*			Description :  
//*
//*			Tree : CDRMProcess::main_process
//*							\->CDRMProcess::Channel_process
//*
//**************************************************************************

void CDrmProc::Channel_process(tNative		*data_in, 
							   tNative		*old_data, 
							   tNative		&posteq_symbolnum, 
							   tNative		&posteq_framenum, 
							   tNativeAcc			&msc_data_length,
							   tNativeAcc			&msc_data_index, 
							   tFRAME_CELL	*msc_cells)
{
	tNative* restrict fft_start=data_in;
	tNative* restrict olddata=old_data;
	
	tFRAME_CELL* restrict msc_data_cells=msc_cells;
	TLoop freq_loop_control=m_config.frequency_control();
	TMLC DUBJA_stat=m_config.dubja_stat();
	tNative fft_size=m_config.fftsize(), guard_interval=m_config.guard_interval();
	tNative total_symbol=fft_size+guard_interval, total_symbol_complex=2*total_symbol;
	tNative display_offset=fft_size/4;
	
	TLoop time_loop_control=m_config.timing_control();
	TFilt filt_comm=m_config.filtering();

	WritePC(10000);

	
	// *** Estimate the channel response ***

	// *** Temporal Interpolation ***
	m_channeleq.EstimateChannelTimeInterp(fft_start, 
										  m_channel, 
										  &m_config, 
										  m_symbolnum);


	
	WritePC(10010);
	
	
	// *** Frequency Interpolation ***
	m_channeleq.EstimateChannelFreqInterp(m_channel, 
										  &m_config);

	WritePC(10020);
	
/*****
 * ~ *
 *****/

	// Monitoring *Channel Estimator*
	m_monitor.ChannelEstimator(m_channel); 
	

/*****
 * ~ *
 *****/

	// Monitoring *AFC* 
//	m_monitor.AFC(m_freq_pull, m_afc_pull);
	


	WritePC(10030);
	
	// *** Determine which error to use  Depends on m_config.AFCErrSource *** //TODO get rid of AFCErrSource from drmconfig
	// *** m_phase_err will hold the error ***
	

	// *** Find freq error (post interp) ***
	tNative clock_err=0;
	m_post_interp_afc.FindFreqErr(	m_channel,			// * Input array from frequency interpolator *
									&m_config,			// * Configuration *
									clock_err,			// * Clock frequency error *
									m_interferer_freq);

	
	if (m_config.TimFreqSource()==postInterpTimFreq)
	{
		// *** clock_err variable m_clock_freq error is set here *** 
		m_clock_freq_err = clock_err;
	}

	// * m_channel max range +/-10 *
	// *** Find the Impulse Response for the channel ***
	m_cir.FindImpResp(	m_channel,
						m_imp_resp, 
						m_phase_err, // * Post FFT fine AFC error *
						fft_size, 
						&m_config,
						m_doppler_shift);

	float doppler_value = ((OFDM_SAMPLE_RATE/total_symbol) * sqrt((4.0f/(m_doppler_shift)))/PI);
//	DP(("Doppler < %f Hz >\n",doppler_value));
	doppler_value *= 256;
//	DP(("Doppler < %f Hz >\n",doppler_value));
	m_doppler_ssi[0] =  (UInt8) doppler_value;
	m_doppler_ssi[1] =  (UInt8) (doppler_value /256);
//	DP(("\nDoppler < %f Hz >\n",doppler_value));

	WritePC(10040);

/*****
 * ~ *
 *****/

	// Monitoring *CIR Complex*
	m_monitor.CIRComplex(m_imp_resp); 

	WritePC(10050);

	// *** Find Magnitude of impulse response ***
	m_cir.FindImpRespMag(m_imp_resp, 
						 &m_config, 
						 m_cir_peak, 
						 m_cir_peak_pos);


	m_cir_ssi[1] = (Int8) m_cir_peak_pos;
	m_cir_ssi[0] = (UInt8) ((m_cir_peak_pos - m_cir_ssi[1]) *256);

	WritePC(10070);

/*****
 * ~ *
 *****/

	// Monitoring *CIR* 
	m_monitor.CIR(m_imp_resp); //TODO

	WritePC(10080);
	
	// * m_imp_resp max range +/- 200000
	// * m_search_output max range +/- 1.0

	// * for display purpose (marks that show the best search position) *
	tNative start; 
	tNative end;
	tNative middle;

	// *** Do a systematic search for the best position ***
	// *** Search (apply weighting fn)									
	m_cir.Search(m_imp_resp, 
				 m_search_output,	//for display only
				 m_best_search_pos, 
				 &m_config,
				 start,				//for display only
				 end,				//for display only
				 middle);			//for display only


	tNative search_offset = guard_interval/2 * 2 * m_config.fstep();   //for display only
		
/*****
 * ~ *
 *****/
	
	// Monitoring *Search Output*
	m_monitor.SearchOutput(m_search_output,search_offset,start,end,middle); 
		
	// *** Turn this bit on after achieving a satisfactory crash lock ***
	if(m_bPost_FFT_timing==TRUE && time_loop_control==loop_enable)
	{
	
		m_freq_pull=m_loopfilter.FilterFine(m_best_search_pos,   // * Best search position *
											m_clock_freq_err,    // * Clock frequency error given by CPostInterpAFC::FindFreqErr *
											m_bPost_FFT_timing); // * Pre or post FFT sync *
												
		// TODO this is a mix of flp fxp (for display purpose)
		// * Min freq pull *
		if(m_freq_pull < m_min_TCS) 
			m_min_TCS=m_freq_pull;

		// * Max freq pull *
		if(m_freq_pull > m_max_TCS) 
			m_max_TCS=m_freq_pull;
	}

	if(freq_loop_control==loop_enable && m_bPost_FFT_timing==TRUE)
	{
		// *** Frequency Loop filter POST FFT ***
		m_afc_pull=m_freqloopfilter.Filter2(m_phase_err) ; 

		if(m_afc_pull < m_min_FP) 
			m_min_FP=m_afc_pull;
		if(m_afc_pull > m_max_FP) 
			m_max_FP=m_afc_pull;
	}


	WritePC(10100);

		
	// *** Clock the data delay line ***
	m_symbol_delay.delay(fft_start);

	WritePC(10120);

	

	// *** delay line ***
	olddata=m_symbol_delay.getsymbolptr(m_channeleq.CompDelayLength(&m_config)+1); //+1 because delay already clocked


	// ***Equalise the data ***
	m_channeleq.EqualiseChannel(olddata, 
								m_channel, 
								fft_start, 
								&m_config);


	WritePC(10130);

	
/*****
 * ~ *
 *****/

	// Monitoring *Equalised Channel*
	m_monitor.EqualisedChannel(fft_start);

	WritePC(10140);

		
	posteq_framenum=m_framenum;
	posteq_symbolnum=m_symbolnum-m_channeleq.CompDelayLength(&m_config);
	if (posteq_symbolnum<0)
	{
		posteq_symbolnum+=m_config.syms_per_frame();
		posteq_framenum--;
		
		if (posteq_framenum<0) 
			posteq_framenum+=FRAMES_PER_SFRAME;
	}

	if(posteq_symbolnum==0) 
	{
		m_sdc_data_index=0;
		if (posteq_framenum==0) 
			msc_data_index=0;
	}

	WritePC(10150);
		
	// *** Update the channel state indicator ***
	
	// *** Update CSI filter state ***
	m_csi.update_state(fft_start,		// * TOTAL_SYMBOL_COMPLEX_MALLOC
					   m_channel,		// * TOTAL_SYMBOL_COMPLEX_MALLOC
					   m_channel_state, // * FFT_SIZE_MALLOC
					   m_MER,			// * SYMS_PER_FRAME_D
					   &m_config, 
					   posteq_symbolnum, 
					   posteq_framenum, 
					   filt_comm, 
					   m_bPost_FFT_timing,
					   m_csi_mean, 
					   m_csi_peak, 
					   m_csi_peak_pos, 
					   m_fac_struct.FACReadyFlag);

	m_cw_ssi[1]= (UInt8) m_csi_peak_pos;
	//m_cw_ssi[0]= (UInt8)((m_csi_peak_pos - m_cw_ssi[1]) *256);
	m_cw_ssi[0]= (UInt8)((m_csi_peak_pos) >> 8);

	

	// * check that m_csi_mean and m_csi_peak are not 0 *
	if( (m_csi_mean == 0) || (m_csi_peak == 0) )
	{
		m_csi_mean = 1;
		m_csi_peak = 1;
	}

	
	m_csi_peak_mean = fxp_10_log10(m_csi_peak/m_csi_mean);
	//m_csi_peak_mean=10.0f*FLOAT (log10(m_csi_peak/m_csi_mean) );
//	DP(("csi peak %i %i  %i\n", m_csi_peak, m_csi_mean, m_csi_peak_mean ));

	m_cw_ssi[3]= (UInt8) m_csi_peak_mean;
	m_cw_ssi[2]= (UInt8)((m_csi_peak_mean >> 8));

	WritePC(10160);

/*****
 * ~ *
 *****/

	// Monitoring *Channel State Info*
	m_monitor.ChannelStateInfo(m_channel_state);
	

	// *** Calculate CSI weightings ***
	m_csi.CalcWeights(m_channel_state, 
					  m_channel, 
					  m_weights, 
					  &m_config);


	WritePC(10170);

/*****
 * ~ *
 *****/

	// Monitoring *Weightings*
	m_monitor.Weightings(m_weights); 
	
	// *** Extract the data ***
	tNativeAcc sdc_data_length;
	tNativeAcc fac_data_length;
	TM_BOOL temp_isframe0; 

	tNativeAcc fac_data_index=m_config.fac_index(posteq_symbolnum);

	// *** Cell Demapping ***
	m_demapper.demap(fft_start, 
					 msc_data_cells + msc_data_index, 
					 msc_data_length, 
					 m_fac_data + fac_data_index, 
					 fac_data_length, 
					 m_sdc_data + m_sdc_data_index, 
					 sdc_data_length, 
					 &m_config, 
					 m_weights, 
					 posteq_symbolnum, 
					 posteq_framenum, 
					 temp_isframe0);

	
		
	m_sdc_data_index += sdc_data_length;
	
	// * put a check on this, it could go wrong check this JEE *
	if(m_sdc_data_index > MAX_SDC_CELLS)
	{
		DP(("m_sdc_data_index %i\n" ,m_sdc_data_index));
		m_sdc_data_index =0;
	}    
	
	msc_data_index += msc_data_length;

		WritePC(10200);
		
}


//**************************************************************************
//*
//*			Function name : *** find_ang ***
//*
//*			Description : 
//*
//*			Tree : CDRMProcess::main_process
//*							\->CDRMProcess::Pre_FFT_process
//*										\->CDRMProcess::find_ang
//**************************************************************************

void CDrmProc::find_ang(tNative	*data_in,
						tNative	pos, 
						tNative	&angle)
{
	if( pos<0 ) 
		pos += ( m_config.fftsize() + m_config.guard_interval() );

	if( data_in[2*pos+1] !=0 && data_in[2*pos] !=0 )
		angle = arctan2_fxp( data_in[2*pos+1] , data_in[2*pos]);
}


//***************************************************************************
//*
//*			Function name : *** arctan2_fxp ***
//*
//*			Description : fast approximate arctan function
//*
//*			Returns : Returns arctan(im/re) in base 13 fixed-point (rad * 2^13)
//*
//*			Input : 
//*
//*			Output :
//*
//*			Tree : Class::function
//*							\->Class::function
//*										\->Class::function
//***************************************************************************

tNative CDrmProc::arctan2_fxp(tNative y, tNative x)
{

#define AT2_MULTP(A,B) (NATIVE_CAST((A*B)>>13))
	
	tNative	c1				= 1608;  // (short)((0.1963)*(1<<13));
	tNative	c2				= 8042;  // (short)((0.9817)*(1<<13));
	tNative	pi_over_4		= 6434;  // (short)((0.785398163)*(1<<13));
	tNative	three_pi_over_4	= 19302; // (short)((2.35619449)*(1<<13));

	tNative abs_y = NATIVE_CAST abs(y) + 1;      // * kludge to prevent 0/0 condition * 

	tNative r;
	tNative angle;

	if (x>=0)
	{
		r = NATIVE_CAST(((x - abs_y)<<13) / (x + abs_y));
		angle = NATIVE_CAST( AT2_MULTP( AT2_MULTP(c1 , r) , AT2_MULTP(r , r)) - AT2_MULTP( c2 , r ) + pi_over_4);
	}
	else
	{
		r = NATIVE_CAST(((x + abs_y)<<13) / (abs_y - x));
		angle = NATIVE_CAST( AT2_MULTP( AT2_MULTP(c1 , r) , AT2_MULTP(r , r)) - AT2_MULTP( c2 , r ) + three_pi_over_4);
	}

	if (y < 0)
		return(-angle);     // * negate if in quad III or IV *
	else
		return(angle);	

}



TMode CDrmProc::ReadMode()
{
	if(m_mode_found)
		return(m_config.mode() );
	else
		return(unknown);
}

TM_BOOL CDrmProc::CheckSDC()
{
	return(m_sdc_struct.SDCReadyFlag);
}



TM_BOOL CDrmProc::CheckFAC()
{
	return(m_fac_struct.FACReadyFlag);
}



unsigned char* CDrmProc::ReadFACRaw(tNativeAcc& length)
{
	length = m_FAC_Length;
	return m_FACRawBits;
}

unsigned char* CDrmProc::ReadSDCRaw(tNativeAcc& length)
{
	length = m_SDC_Length;
	return m_SDCRawBits;
}


tNativeAcc CDrmProc::PackBits(const unsigned char *in, unsigned char *out, tNativeAcc len, tNativeAcc pad)
{
	tNativeAcc i;
	tNativeAcc pos=0, index=0;
	tNativeAcc shift=7-pad;

	unsigned char byte=0;
	out[pos]=0;

	for(i=0; i< len; i++){

		out[pos] |= in[i] << shift;

		if(--shift < 0 ){
			shift =7;
			pos++;
			out[pos]=0;
		} 
	}
	return (pos+1);		
}


UInt16 CDrmProc::ReadNumberSymbols(void)
{
	return m_config.syms_per_frame();
}


/*
tNative CDrmProc::ReadServiceSelected()
{
	return m_config.service_selected();
}*/

/*unsigned char* CDrmProc::ReadRawData(int &length)
{
	length = m_config.fftsize() *2 *sizeof(short);

	if(m_raw_data_flag)
		return (unsigned char*) m_raw_data_out_b;
	else
		return (unsigned char*) m_raw_data_out_a;
}

int CDrmProc::CheckRaw()
{
	return m_raw_data_flag;
}
*/

/**************************************************************************

			Function name : *** ProcessBlock ***

  Description: Interface to the CDrmProc Class 
			   and buffer size conversion
			   Takes input blocks of a fixed size and 
			   outputs blocks the size of 1 symbol 
			   for the current DRM mode. 


// *** INPUT ***
short			*data_in,		  * Input DRM data IQ at -5kHz reference * 
									(CAPTURE_BUFFER_POINTS_U)
// *** MONITORING ***
TM_BOOL&		monit_ready,	  * Monitor array ready *
MonitorType		* monit1,		  * Monitor array 1
MonitorType		* monit2,		  * Monitor array 2
MonitorType		*trigger_dat,     * Monitor array with trigger pulses *

// *** OUTPUT ***
tFACStruct		*FACStruct,		  * FAC data *
tSDCStruct		*SDCStruct,		  * SDC data *
tFRAME_CELL		*msc_data,		  * MSC data streams *
int				&msc_data_index,  * MSC data index *
int				&posteq_framenum, * frame number *
int				&posteq_symbolnum,* symbol number *

// *** MISC ***
TM_BOOL			band_scan,		  * Band scan ? *
TM_BOOL			&drm_found,		  * DRM found ? *
UInt8			*change_flags)    * flags change (rest from CAB but could be useful) *


**************************************************************************/

TM_BOOL CDrmProc::ProcessBlock(	tNative		 * data_in, 
								TM_BOOL		 & monit_ready,	
							//	MonitorType  * trigger,	
								MonitorType  * monit1,	
								MonitorType  * monit2,	
								tFACStruct	 * FACStruct, 
								tSDCStruct	 * SDCStruct, 
								tFRAME_CELL	 * msc_data, 
								tNativeAcc			 & msc_data_index,   
								tNative		& posteq_framenum,  
								tNative		& posteq_symbolnum, 
								TM_BOOL		 band_scan,     
								TM_BOOL		 & drm_found, 
								UInt8		 * change_flags,
								tNative		* DRMLevel,
								tUNativeAcc	sample_rate_dev)
								
{

	// Monitoring *Set pointers for Monitoring Member-Class Object*
	m_monitor.SetPointers(&m_config,
						  monit1,
						  monit2,
				//		  trigger,
						  &m_free_symbolnum); 

	tNative* pTempShort = data_in;
	tNative* pShortIn1 = m_pShortIn1;
	tNative* pShortIn2 = m_pShortIn2;

	TM_BOOL valid_drm_block=FALSE;

	tNativeAcc buff_1_size=m_buff_1_size;
	tNativeAcc buff_2_size=m_buff_2_size;
	tNativeAcc buff_1_pos=m_buff_1_pos;
	tNativeAcc buff_2_pos=m_buff_2_pos;  
	
//	printf("here\n");

	// *** reset the program counter ***
	WritePC(0); 

	if(m_buffer_1)
	{
		if( (buff_1_pos + CAPTURE_BUFFER_POINTS_U) >= DRM_BUFFER_POINTS_U)
		{
			memcpy( &pShortIn1[buff_1_pos] , pTempShort , (DRM_BUFFER_POINTS_U-buff_1_pos)*sizeof(tNative) );  

			m_buffer_1 = FALSE;

			// *** Main Process ***
			
			valid_drm_block = main_process(pShortIn1,
										   msc_data_index, 
										   msc_data, 
										   monit_ready, 
									//	   trigger, 
										   posteq_framenum, 
										   posteq_symbolnum, 
										   band_scan, 
										   drm_found,
										   change_flags,
										   DRMLevel,
										   sample_rate_dev);
										   
			

			tNativeAcc points = CAPTURE_BUFFER_POINTS_U-(DRM_BUFFER_POINTS_U-buff_1_pos);
			
			memcpy(pShortIn2, &pTempShort[DRM_BUFFER_POINTS_U-buff_1_pos], points*sizeof(tNative) ); 

			buff_2_pos += points;

			buff_1_pos = 0;
		}
		else
		{
			memcpy( &pShortIn1[buff_1_pos] , pTempShort, CAPTURE_BUFFER_POINTS_U *sizeof(tNative) );


			buff_1_pos += CAPTURE_BUFFER_POINTS_U;
		}
	}
	else
	{
		if( (buff_2_pos + CAPTURE_BUFFER_POINTS_U) >= DRM_BUFFER_POINTS_U)
		{
			memcpy( &pShortIn2[buff_2_pos] , pTempShort, (DRM_BUFFER_POINTS_U-buff_2_pos)*sizeof(tNative) );

			m_buffer_1 = TRUE;
			
			// *** Main Process ***
			
			valid_drm_block = main_process(pShortIn2,
										   msc_data_index, 
										   msc_data,
										   monit_ready, 
									//	   trigger, 										 
										   posteq_framenum, 
										   posteq_symbolnum, 
										   band_scan, 
										   drm_found,
										   change_flags,
										   DRMLevel,
										   sample_rate_dev); 
			
				
			tNativeAcc points = CAPTURE_BUFFER_POINTS_U-(DRM_BUFFER_POINTS_U-buff_2_pos);

			memcpy(pShortIn1, &pTempShort[DRM_BUFFER_POINTS_U-buff_2_pos], points*sizeof(tNative) );
		
			buff_1_pos+=points;

			buff_2_pos=0;
		}
		else
		{
			memcpy( &pShortIn2[buff_2_pos] , pTempShort, CAPTURE_BUFFER_POINTS_U*sizeof(tNative) ) ;

			buff_2_pos += CAPTURE_BUFFER_POINTS_U;
		}
	}
	WritePC(30000);

	memcpy(FACStruct, &m_fac_struct, sizeof(m_fac_struct) );
	memcpy(SDCStruct, &m_sdc_struct, sizeof(m_sdc_struct) );

	if(m_new_mode)
	{
		m_buff_1_pos=0;
		m_buff_2_pos=0;
		m_buffer_1=TRUE;
		m_new_mode=FALSE;
	}
	else
	{
		m_buff_1_pos=buff_1_pos;
		m_buff_2_pos=buff_2_pos;
	}

	WritePC(30050);

	return(valid_drm_block);
}

void CDrmProc::ChangeMode(TMode mode)
{
	if(mode == ground)
		m_buff_1_size=m_buff_2_size=TOTAL_SYMBOL_COMPLEX_A_UP;	
	
	else if(mode == robust2)
		m_buff_1_size=m_buff_2_size=TOTAL_SYMBOL_COMPLEX_D_UP;
	
	else if(mode == robust1)
		m_buff_1_size=m_buff_2_size=TOTAL_SYMBOL_COMPLEX_C_UP;
	
	else 
		m_buff_1_size=m_buff_2_size=TOTAL_SYMBOL_COMPLEX_B_UP;
	

	m_new_mode=TRUE;

}


//***************************************************************************
//*
//*			Function name : *** FFT_process ***
//*
//*			Description : 
//*
//*			Returns :
//*
//*			Input :
//*
//*			Output :
//*
//*			Tree : CDrmProc::main_process
//*							\->CDrmProc::FFT_process
//***************************************************************************

void CDrmProc::FFT_process(tNative* data_in) 
{   
	tNative* restrict fft_out=m_fft_out;
	tNative* restrict fft_start=data_in; 

	tNative i;
	tNative fft_size=m_config.fftsize(), guard_interval=m_config.guard_interval();
	tNative total_symbol=fft_size+guard_interval, total_symbol_complex=2*total_symbol;
	tNative index=0;

	WritePC(121);

	m_shift=0; 

	// *** Fourier transform the data (not) in place ***
	(m_symbol_fourier.*m_FFTFuncPtr)((fftw_complex*)fft_start, (fftw_complex*)fft_out);		

	WritePC(130);

	// *** Scale FFT output ***
	tNative scale;
	switch(fft_size)
	{
		case 224: scale=9362; break;
		case 352: scale=11915; break;
		case 512: scale=16384; break;
		case 576: scale=14563; break;
		default: scale=16384;
	}
	
	WritePC(140);
	
	for(i=0; i< 2*fft_size; i++)
		fft_start[i]=(NATIVE_CAST((fft_out[i]*scale)>>15));

	WritePC(150);

	// *** swap the side bands so that dc is in the middle of the data array ***
	m_symbol_fourier.SideBandSwap(fft_start,fft_size);

	WritePC(160);
}

tNative* CDrmProc::ReadMER(void)
{	
	return(m_MER);
}

TM_BOOL CDrmProc::ReadSyncStatus()
{
	return m_bPost_FFT_timing;
}

TM_BOOL CDrmProc::ReadFACStatus()
{
	return m_fac_struct.FACReadyFlag;
}

TM_BOOL CDrmProc::ReadSDCStatus()
{
	return m_sdc_struct.SDCReadyFlag;
}

TM_BOOL CDrmProc::ReadTjump()
{
	TM_BOOL temp=m_tjump_flag;

	m_tjump_flag=FALSE;

	return(temp);
}

TM_BOOL CDrmProc::ReadFjump()
{
	TM_BOOL temp=m_fjump_flag;

	m_fjump_flag=FALSE;

	return(temp);
}

TM_BOOL CDrmProc::ReadSjump()
{
	TM_BOOL temp=m_sjump_flag;

	m_sjump_flag=FALSE;

	return(temp);
}

TM_BOOL CDrmProc::ReadPreFFT()
{
	TM_BOOL temp=m_prefft_flag;

	m_prefft_flag=FALSE;

	return(temp);
}

TM_BOOL CDrmProc::ReadPostFFT()
{
	TM_BOOL temp=m_postfft_flag;

	m_postfft_flag=FALSE;

	return(temp);
}

TM_BOOL CDrmProc::ReadModeSearch()
{
	TM_BOOL temp=m_mode_search_flag;

	m_mode_search_flag=FALSE;

	return(temp);
}

TM_BOOL CDrmProc::ReadCWFlag()
{
	TM_BOOL temp=m_cw_flag;

	m_cw_flag=FALSE;

	return(temp);
}

tNativeAcc CDrmProc::ReadMinFP()
{
	tNativeAcc temp=0;

	// * To calculate the floating point value -> ((float)m_min_FP/(1<<23))*OFDM_SAMPLE_RATE;
	temp = m_min_FP;

	m_min_FP=MAX_FREQ_PULL_FXP*2;

	return(temp);
}

tNativeAcc CDrmProc::ReadMaxFP()
{
	tNativeAcc temp=0;
	
	// * To calculate the floating point value -> ((float)m_max_FP/(1<<23))*OFDM_SAMPLE_RATE;
	temp=m_max_FP;

	m_max_FP=-MAX_FREQ_PULL_FXP*2;

	return(temp);
}

tNative CDrmProc::ReadMinTCS()
{
	// * To calculate the floating point value -> ((float)m_min_TCS/(1<<23))*OFDM_SAMPLE_RATE;
	tNative temp=m_min_TCS;
	
	m_min_TCS=MAX_PULL_TIMING_LOOP*2;

	return(temp);
}

tNative CDrmProc::ReadMaxTCS()
{
	// * To calculate the floating point value -> ((float)m_max_TCS/(1<<23))*OFDM_SAMPLE_RATE;
	tNative temp=m_max_TCS;

	m_max_TCS=-MAX_PULL_TIMING_LOOP*2;

	return(temp);
}



tNative CDrmProc::ReadMaxSearch0()
{
	return(0);
}



tNative CDrmProc::ReadMaxSearch()
{
	return(0);
}


tNative CDrmProc::ReadCSIMean()
{
	tNative temp=m_csi_mean; 

	m_csi_mean=0;

	return(temp);
}

tNative CDrmProc::ReadCSIPeak()
{
	tNative temp=m_csi_peak; 

	m_csi_peak=0;

	return(temp);
}

tNative CDrmProc::ReadCSIPeakMean()
{
	tNative temp=m_csi_peak_mean; 

	m_csi_peak_mean=0;

	return(temp);
}

tNative CDrmProc::ReadCSIPeakPos()
{
	// *** csi pek pos given in Hz *** 
	tNative temp=m_csi_peak_pos;

	m_csi_peak_pos=0;

	return(temp);
}

tNative CDrmProc::ReadCIRPeak()
{
	tNative temp=m_cir_peak;

	m_cir_peak=0;

	return(temp);
}

tNative CDrmProc::ReadCIRPeakPos()
{
	tNative temp=m_cir_peak_pos;

	m_cir_peak_pos=0;

	return(temp);
}


tNative CDrmProc::ReadTotalMass()
{
	return(0);
}

tNative CDrmProc::ReadMoM1()
{
	return(0);
}

tNative CDrmProc::ReadMoM2()
{
	return(0);
}

Int16 CDrmProc::ReadCWPos()
{
	Int16 temp=m_cw_pos;

	m_cw_pos=0;

	return(temp);
}


tNative* CDrmProc::ReadSignalLevel()
{
	return (m_signal_level);
}

tNative CDrmProc::ReadPeakLevel()
{
	return (0);
}


void CDrmProc::SignalLevels(tNative* pShort, tNative symbol, tNative points)
{
	tNative i;

	tNative real, im;
	tNativeAcc temp_mag = 0; // * 32 bit
	tNativeAcc mag_tot = 0;  // * 32 bit
//	float temp_mag_f=0, mag_tot_f=0;

	// *** Take the sum of sq magnitudes
	for(i=0;i< points;i+=2)
	{
		real= pShort[i];
		im  = pShort[i+1];

		temp_mag=(int)((real*real+im*im)>>11);
	//	temp_mag_f=(float)((real*real+im*im) );
		
		mag_tot+=temp_mag;
	//	mag_tot_f += temp_mag_f;
	}

	// * take the mean *
	mag_tot /= points;
	mag_tot *=2;

//	mag_tot_f /= points;
//	mag_tot_f *=2;

//	float flog = 10 * log10(mag_tot_f);

//	flog -= 90.3f;

	tNative slog = fxp_10_log10(mag_tot);
	
	tNative temp = (33+slog/128);

	temp = 90 - temp;

	m_signal_level_ssi = (UInt8) temp;


	m_signal_level[symbol]=mag_tot;
	// *** m_signal_level[] stores the (average sqared magnitude)/(1<<11) 
	// *** for all symbols in 1 frame 



	// *** Use this bit of code to calculate and display the average level over 1 frame ***
	/*
	int syms=m_config.syms_per_frame();
	int m_signal_level_av;

	// *** To get the average of the (average sqared magnitude)/(1<<11) over 1 frame ***
	for(i=0; i< syms; i++)
		m_signal_level_av+=m_signal_level[i];

	m_signal_level_av /= syms;

	DP(("********Average signal level <%d> ***\n",m_signal_level_av));
	*/
}


CDRMConfig* CDrmProc::GetConfig()
{
	return(&m_config);
}

tNative CDrmProc::ReadFilterBW()
{
	tNative freq=m_channel_filter.GetFilterBW();

	switch(freq)
	{
	case 0:
		return 4500;
	case 1:
		return 5000;
	case 2:
		return 9000;
	case 3:
		return 10000;
	case 4:
		return 18000;
	case 5:
		return 20000;
	default:
		return 0;
	}

}

void CDrmProc::WritePC(UInt16 value)
{
	tNativeAcc stop=sizeof(UInt16);                    
	tNativeAcc i;

	for(i=0; i< stop; i++)
		xioWrite(DPRAM_DRM_PC_BASE+i, (char) (value >> (i*8) ) );
}


void CDrmProc::GetLabel(tNativeAcc type, UInt8 *address, tNativeAcc length,
							   tNativeAcc *retType, tNativeAcc *retLen, UInt8 **retAdd, UInt8 service)
{
	tNativeAcc service_now=m_config.service_selected();

	if(service == 4) service = service_now;

	*retType=type+0x80;
	*retLen = MAX_LABEL_LEN_OLD; 
	*retAdd = (UInt8*) &m_service_label_store[service];
}

void CDrmProc::GetLabelNewSpec(int type, UInt8 *address, tNativeAcc length,
							   tNativeAcc *retType, tNativeAcc *retLen, UInt8 **retAdd, UInt8 service)
{
	tNativeAcc service_now=m_config.service_selected();

	if(service == 4) 
		service = service_now;

	*retType=type+0x80;
	*retLen = MAX_LABEL_LEN; 
	*retAdd = (UInt8*) &m_service_label_store[service];
}


void CDrmProc::GetServiceID(tNativeAcc type, UInt8 *address, tNativeAcc length,
							   tNativeAcc *retType, tNativeAcc *retLen, UInt8 **retAdd, UInt8 service)
{
	if(service == 4) service = (UInt8) m_config.service_selected();


	*retType=type+0x80;
	*retLen = 4;
//	DP(("sizeof %i \n", sizeof(tServiceIDStore) ));
	*retAdd = (UInt8*) &m_old_service_ident[service];

}

void CDrmProc::GetProg(tNativeAcc type, UInt8 *address, tNativeAcc length,
							   tNativeAcc *retType, tNativeAcc *retLen, UInt8 **retAdd, UInt8 service)
{
	if(service == 4) 
		service = (UInt8) m_config.service_selected();

	*retType=type+0x80;
	*retLen = 2;
	*retAdd = (UInt8*) &m_old_programme_store[service];
}

void CDrmProc::GetLang(tNativeAcc type, UInt8 *address, tNativeAcc length,
							   tNativeAcc *retType, tNativeAcc *retLen, UInt8 **retAdd, UInt8 service)
{
	if(service == 4) 
		service = (UInt8) m_config.service_selected();

	*retType=type+0x80;
	*retLen = 7;
	*retAdd = (UInt8*) &m_language_store[service];

}

void CDrmProc::GetServices(tNativeAcc type, UInt8 *address, tNativeAcc length,
							   tNativeAcc *retType, tNativeAcc *retLen, UInt8 **retAdd)
{

	*retType=type+0x80;
	*retLen = 1;
	*retAdd = (UInt8*) &m_services;

}

void CDrmProc::GetMER(tNativeAcc type, UInt8 *address, tNativeAcc length,
							   tNativeAcc *retType, tNativeAcc *retLen, UInt8 **retAdd)
{

	*retType=type+0x80;
	*retLen = 2;
//	*retAdd = (UInt8*) &m_mer_ssi;
	//*retAdd = (UInt8*) &m_MER[2]; //this is the unweighted mer

	int value1=address[0];

	 
	//mer[0] = m_wgt_mer_fac;
	//mer[1] = m_wgt_mer_msc;
	//mer[2] = m_unwgt_mer_msc;

	if(value1 == 0)
		*retAdd = (UInt8*) &m_MER[0];
	else if(value1 == 1)
		*retAdd = (UInt8*) &m_MER[1];
	else
		*retAdd = (UInt8*) &m_MER[2];

}

void CDrmProc::GetFACRaw(tNativeAcc type, UInt8 *address, tNativeAcc length,
							   tNativeAcc *retType, tNativeAcc *retLen, UInt8 **retAdd)
{

	*retType=type+0x80;
	*retLen = 9;
	*retAdd = (UInt8*) &m_FACRawBits;

}

void CDrmProc::GetSDCRaw(tNativeAcc type, UInt8 *address, tNativeAcc length,
							   tNativeAcc *retType, tNativeAcc *retLen, UInt8 **retAdd)
{

	*retType=type+0x80;
	*retLen = m_SDC_Length;
	*retAdd = (UInt8*) &m_SDCRawBits;

}



void CDrmProc::GetCIR(tNativeAcc type, UInt8 *address, tNativeAcc length,
							   tNativeAcc *retType, tNativeAcc *retLen, UInt8 **retAdd)
{

	*retType=type+0x80;
	*retLen = 2;
	*retAdd = (UInt8*) &m_cir_ssi;

}

void CDrmProc::GetMisc(tNativeAcc type, UInt8 *address, tNativeAcc length,
							   tNativeAcc *retType, tNativeAcc *retLen, UInt8 **retAdd)
{



	*retType=type+0x80;
	*retLen = sizeof(m_misc_store);
	*retAdd = (UInt8*) &m_old_misc_store;



}

UInt8 CDrmProc::GetRF(void)
{

	return m_signal_level_ssi;
}

UInt8 CDrmProc::GetDRMLevel(void)
{

	return m_drm_signal_level_ssi;
}

void CDrmProc::GetServiceAct(tNativeAcc type, UInt8 *address, tNativeAcc length,
							   tNativeAcc *retType, tNativeAcc *retLen, UInt8 **retAdd)
{

	m_service_act[0] = m_config.service_selected();
	*retType=type+0x80;
	*retLen = 2;
	*retAdd = (UInt8*) &(m_service_act);
	

}

void CDrmProc::GetCW(tNativeAcc type, UInt8 *address, tNativeAcc length,
							   tNativeAcc *retType, tNativeAcc *retLen, UInt8 **retAdd)
{

	*retType=type+0x80;
	*retLen = 4;
	*retAdd = (UInt8*) &m_cw_ssi;

}


void CDrmProc::GetDop(tNativeAcc type, UInt8 *address, tNativeAcc length,
							   tNativeAcc *retType, tNativeAcc *retLen, UInt8 **retAdd)
{

	*retType=type+0x80;
	*retLen = 2;
	*retAdd = (UInt8*) &m_doppler_ssi;

}

void CDrmProc::GetDel(tNativeAcc type, UInt8 *address, tNativeAcc length,
							   tNativeAcc *retType, tNativeAcc *retLen, UInt8 **retAdd)
{

	*retType=type+0x80;
	*retLen = 2;
	*retAdd = (UInt8*) &m_delay_ssi;

}


void CDrmProc::SetChannelFilter(tNativeAcc type, UInt8 *address, tNativeAcc length,
							   tNativeAcc *retType, tNativeAcc *retLen, UInt8 **retAdd)
{

	UInt8 val = address[0];

	*retType=type+0x80;
	*retLen = 0;
	*retAdd = NULL;

	TConfig_Command command;

	command.operation=set_chan_filt; 
	command.value=val;

	m_config.command(&command);

}

TM_BOOL CDrmProc::SetServices()
{
	m_services = 0;

	if( m_fac_struct.Services[0].InUse ) m_services |= S0;
	if( m_fac_struct.Services[1].InUse ) m_services |= S1;
	if( m_fac_struct.Services[2].InUse ) m_services |= S2;
	if( m_fac_struct.Services[3].InUse ) m_services |= S3;

	if( m_fac_struct.Services[0].AFlag ) m_services |= S0_AUDIO;
	if( m_fac_struct.Services[1].AFlag ) m_services |= S1_AUDIO;
	if( m_fac_struct.Services[2].AFlag ) m_services |= S2_AUDIO;
	if( m_fac_struct.Services[3].AFlag ) m_services |= S3_AUDIO;

	// * check to see if something has changed *
	if(m_old_services != m_services)
	{
		m_old_services = m_services;
		return TRUE;
	}
	else
		return FALSE;
	

}

void CDrmProc::ResetLang()
{
	memset(&m_language_store, 0, sizeof(m_language_store));
	memset(&m_old_language_store, 0, sizeof(m_old_language_store) );

	for(tNativeAcc i=0; i< MAX_NO_SERVICES; i++)
	{
		m_old_language_store[i].service=i;
		m_language_store[i].service=i;
	}

}

void CDrmProc::ResetFAC()
{
	memset(&m_fac_struct,0,sizeof(tFACStruct));
}

void CDrmProc::ResetFACService(tNativeAcc service)
{
	tNativeAcc i;

	for(i=0; i< MAX_NO_SERVICES; i++)
	{
		if(service != i)
			memset(&m_fac_struct.Services[i], 0, sizeof(tServiceStruct) );
	}
}

void CDrmProc::ResetSDC()
{
	// * clear the sdc contents *
	memset(&m_sdc_struct,0,sizeof(tSDCStruct));

	// * write in no label to the type 1's *
	for(tNativeAcc i=0; i<MAX_NO_SERVICES; i++)
		strcpy(m_sdc_struct.Type1[i].label, "No Label");

}

TM_BOOL CDrmProc::SetLang()
{
	tNativeAcc service = m_config.service_selected();

	for(tNativeAcc i=0; i< MAX_NO_SERVICES; i++)
	{
		m_language_store[i].language[0] = m_fac_struct.Services[i].language;

		m_language_store[i].language[1] = (unsigned char) (m_sdc_struct.Type12[i].Lang_code) >> 16;
		m_language_store[i].language[2] = (unsigned char) (m_sdc_struct.Type12[i].Lang_code) >> 8;
		m_language_store[i].language[3] = (unsigned char) (m_sdc_struct.Type12[i].Lang_code);
		m_language_store[i].language[4] = (unsigned char) (m_sdc_struct.Type12[i].Country_code) >> 8;
		m_language_store[i].language[5] = (unsigned char) (m_sdc_struct.Type12[i].Country_code);
	}
	return TRUE;
}


void CDrmProc::CheckLabels(UInt8* change_flags)
{       
		// * check to see if the label has changed * 
		if(strcmp((char*) m_sdc_struct.Type1[m_config.service_selected() ].label, (char*) m_service_label_store[m_config.service_selected()].label) ) //changel) )
			// * set the flag to indicate a change *
			change_flags[1] |= NEW_LABEL_MASK;

		if(m_old_service != m_config.service_selected() )
		{                                                         
			change_flags[1] |= NEW_LABEL_MASK;
			change_flags[1] |= NEW_LANGUAGE_MASK;
			change_flags[1] |= NEW_SERVICE_ID;
			change_flags[1] |= NEW_PROGRAMME_MASK;
			change_flags[1] |= NEW_AUDIO_MASK;
			m_old_service = m_config.service_selected();
		}
			

		// * go thorugh all the services *
		for(tNativeAcc i=0; i< 4; i++)
		{
			// * check to see if the label has changed *
			if(strcmp((char*)m_sdc_struct.Type1[i].label, (char*) m_service_label_store[i].label) ) //change here
			{
				
				// * now copy the new label into the store *
				strcpy((char*) m_service_label_store[i].label, (char*) m_sdc_struct.Type1[i].label);
				
				// * set the flag to indicate a change *
				change_flags[i+2] |= NEW_LABEL_MASK;

				char string[2*MAX_LABEL_LEN + 10];
				sprintf(string, "%s %s %i", m_sdc_struct.Type1[i].label, m_service_label_store[i].label, i+2);
				DP(("%s %s %i", m_sdc_struct.Type1[i].label, m_service_label_store[i].label, i+2));

				
			}		
		}
}

void CDrmProc::CheckMisc(UInt8* change_flags)
{
	tNativeAcc i;

	tNative temp_lengths[MAX_NO_STREAMS];

	tNativeAcc service = m_config.service_selected();


	for(i=0; i< MAX_NO_STREAMS; i++)
	{
		temp_lengths[i] = m_sdc_struct.Type0.DataLen[i].datalenA + m_sdc_struct.Type0.DataLen[i].datalenB;
	//	m_misc_store.lengths[i].length = m_sdc_struct.Type0.DataLen[i].datalenA + m_sdc_struct.Type0.DataLen[i].datalenB;	
	}

	for(i=0; i< MAX_NO_SERVICES; i++)
	{
		if(m_fac_struct.Services[i].InUse)
		{
			if(m_fac_struct.Services[i].DFlag)
			{
				tNativeAcc stream = m_sdc_struct.Type5[i].streamid;
				m_misc_store.lengths[i+1].length = temp_lengths[stream];
			}
			else
			{
				tNativeAcc stream = m_sdc_struct.Type9[i].streamid;
				m_misc_store.lengths[i+1].length = temp_lengths[stream];

				if(m_sdc_struct.Type9[i].textflag)
					m_misc_store.lengths[i+1].length -= 4;
			}

		}
	}

	m_misc_store.lengths[0].length = m_misc_store.lengths[service+1].length;

/*	if(m_fac_struct.Services[service].InUse)
	{
		int stream=0;
		if(m_fac_struct.Services[service].DFlag)
		{
			stream = m_sdc_struct.Type5[service].streamid;
		//	m_misc_store.lengths[0].length = temp_lengths[stream];
		}
		else
		{
			stream = m_sdc_struct.Type9[service].streamid;
		//	m_misc_store.lengths[0].length = temp_lengths[stream];
			
		//	if(m_sdc_struct.Type9[service].textflag)
		//		m_misc_store.lengths[i+1].length -= 4;
		}
		m_misc_store.lengths[0].length = m_misc_store.lengths[service].length;
		
	}*/

	

	m_misc_store.MSC = m_fac_struct.MSCMode;
	m_misc_store.SDC = m_fac_struct.SDCMode;

	
	if( memcmp(&m_misc_store, &m_old_misc_store, sizeof(m_misc_store) ) )
	{
		//set the flag to indicate a change
		change_flags[0] |= NEW_MISC_INFO;
		memcpy(&m_old_misc_store, &m_misc_store, sizeof(m_misc_store) );
	}


}

void CDrmProc::CheckLang(UInt8* change_flags)
{
	tNativeAcc i, j;
	tNativeAcc service = m_config.service_selected();
	
	for(j=0; j< LANG_STORE; j++)
	{
		if(m_old_language_store[service].language[j] != m_language_store[service].language[j] )
		// * set the flag to indicate a change *
		change_flags[1] |= NEW_LANGUAGE_MASK;
	}

	for(i=0; i< MAX_NO_SERVICES; i++)
	{
		for(j=0; j< LANG_STORE; j++)
		{
			if(m_old_language_store[i].language[j] != m_language_store[i].language[j] )
				// * set the flag to indicate a change *
				change_flags[i+2] |= NEW_LANGUAGE_MASK;
		}

		
		for(j=0; j< LANG_STORE; j++)
		{
			m_old_language_store[i].language[j] = m_language_store[i].language[j];

		}
	}

}

/*(void CDrmProc::CheckActive(void)
{
	m_services_act[0] = m_config.service_selected();
//	m_services_act[1] = m_config.service_selected_usb();

	if( (m_services_act[0] != m_old_services_act[0]) || (m_services_act[1] != m_old_services_act[1]) )
	{
//		change_flags_ssi[0] |= ACTIVE_SERVICE_CHANGE;
//		change_flags_usb[0] |= ACTIVE_SERVICE_CHANGE;

		m_old_services_act[0] = m_services_act[0];
//		m_old_services_act[1] = m_services_act[1];
	}
		

}*/

void CDrmProc::CheckServiceID(UInt8* change_flags)
{
	tNativeAcc service = m_config.service_selected();

	tUNativeAcc ID = (m_old_service_ident[service].ID <<16) + (m_old_service_ident[service].ID2 << 8) + m_old_service_ident[service].ID3;
	if(ID != m_fac_struct.Services[service].service_ident.i) //todo change here
		change_flags[1] |= NEW_SERVICE_ID;

	for(tNativeAcc i=0; i< MAX_NO_SERVICES; i++)
	{
		tUNativeAcc ID = (m_old_service_ident[i].ID <<16) + (m_old_service_ident[i].ID2 << 8) + m_old_service_ident[i].ID3;
		if(ID != m_fac_struct.Services[i].service_ident.i) //todo change here
			change_flags[i+2] |= NEW_SERVICE_ID;

		m_old_service_ident[i].ID = (m_fac_struct.Services[i].service_ident.i) >> 16;
		m_old_service_ident[i].ID2 = (m_fac_struct.Services[i].service_ident.i >> 8);
		m_old_service_ident[i].ID3 = m_fac_struct.Services[i].service_ident.i;
	}

	
}

void CDrmProc::CheckProgramme(UInt8* change_flags)
{
	tNativeAcc service = m_config.service_selected();

	if(m_fac_struct.Services[service].prog_type != m_old_programme_store[service].Prog)
		// * set the flag to indicate a change *
		change_flags[1] |= NEW_PROGRAMME_MASK;

	for(tNativeAcc i=0; i< MAX_NO_SERVICES; i++)
	{
		if(m_fac_struct.Services[i].prog_type != m_old_programme_store[i].Prog)
			// * set the flag to indicate a change *
			change_flags[i+2] |= NEW_PROGRAMME_MASK;

		m_old_programme_store[i].Prog = m_fac_struct.Services[i].prog_type;
	}

	
}

void CDrmProc::CheckAudio(UInt8* change_flags)
{

	tNativeAcc service = m_config.service_selected();

	if(m_old_audio_store[service].audio != m_audio_store[service].audio)
		// * set the flag to indicate a change *
		change_flags[1] |= NEW_AUDIO_MASK;

	for(tNativeAcc i=0; i< MAX_NO_SERVICES; i++)
	{
		if(m_old_audio_store[i].audio != m_audio_store[i].audio)
			// * set the flag to indicate a change *
			change_flags[i+2] |= NEW_AUDIO_MASK;

		m_old_audio_store[i].audio = m_audio_store[i].audio;
	}

}

TM_BOOL CDrmProc::SetAudioStore()
{

	tNativeAcc i;

	for(i=0; i<MAX_NO_SERVICES; i++)
	{
		m_audio_store[i].audio =0;

		tNativeAcc audio_mode=m_sdc_struct.Type9[i].audiomode1*2+m_sdc_struct.Type9[i].audiomode2;

		if( m_sdc_struct.Type9[i].audiocoding != 0) audio_mode =0;

		m_audio_store[i].audio |= audio_mode;
		m_audio_store[i].audio |= (m_sdc_struct.Type9[i].audiocoding << 3);
		m_audio_store[i].audio |= (m_sdc_struct.Type9[i].SBRFlag << 2);

	}

	return TRUE;

}

void CDrmProc::GetAudioType(tNativeAcc type, UInt8 *address, tNativeAcc length,
							   tNativeAcc *retType, tNativeAcc *retLen, UInt8 **retAdd,
							   UInt8 service)
{
	if(service == 4) service = (UInt8) m_config.service_selected();

	*retType=type+0x80;
	*retLen = 2;
	*retAdd = (UInt8*) &m_audio_store[service];

}

tNative CDrmProc::GetService()
{
	return m_config.service_selected();
}

tNative CDrmProc::fxp_10_log10(tUNativeAcc in)
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

tNative CDrmProc::fixed_sqrt(tNative x) 
{ 

if (x <= 0) return 0; 
tNative x2 = x; 
x = (x + (x2<<14) / x) >> 1; // Iteration 1 
x = (x + (x2<<14) / x) >> 1; // Iteration 2 
x = (x + (x2<<14) / x) >> 1; // Iteration 3 
x = (x + (x2<<14) / x) >> 1; // Iteration 4 
//x = (x + (x2<<14) / x) >> 1; // Iteration 5... more iterations -> higher precision 

return x;
 
} 

void CDrmProc::SetID(tNative id)
{
	m_id_num = id;
}

void CDrmProc::SetInter(int value)
{

	if(value == 1)
		m_bInterFlag = TRUE;
	else
		m_bInterFlag = FALSE;

}

