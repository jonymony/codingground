// DrmProc.h: interface for the CDrmProc class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DRMPROC_H__44162991_EF8E_11D3_8B00_00C04FA11AF6__INCLUDED_)
#define AFX_DRMPROC_H__44162991_EF8E_11D3_8B00_00C04FA11AF6__INCLUDED_



#include "PALCellDeinterleaver.h"
#include "../common_fxp/MLCDecoder.h"
#include "../common/PRBSCheck.h"
#include "Fourier.h"
#include "../common/DeMap.h"
#include "TimLoopFilter.h"
#include "Resample.h"
#include "FreqCorrector.h"
#include "FreqLoopFilter.h"
#include "ChannelEq.h"
#include "SymbolDelay.h"
#include "DRMConfig.h"
#include "CSI.h"
#include "TRefFinder.h"
#include "ImpulseResp.h"
#include "../common/FACDecode.h"
#include "../structurescommon.h"
#include "../common/EnergyDispersal.h"
#include "../common/SDCDecode.h"
#include "../configcommands.h"
#include "TimeProc.h"
#include "ModeDetect.h"
//#include "CWFinder.h"
#include "NotchFilter.h"
#include "ChannelFilter.h"
#include "DCFilter.h"	
#include "PostInterpAFC.h"
#include "DigAGC.h"
#include "Monitor.h"
#include "cwinter.h"
#include "../bbc_types.h"


class CDrmProc  
{

public:

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
	TM_BOOL ProcessBlock(tNative				*data_in, 
						 TM_BOOL			&monit_ready, 
				//		 MonitorType		*trigger, 
						 MonitorType		*monit1, 
						 MonitorType		*monit2,
						 tFACStruct			*FACStruct, 
						 tSDCStruct			*SDCStruct,
						 tFRAME_CELL		*msc_data_cells, 
						 tNativeAcc				&msc_data_index,
						 tNative				&posteq_framenum, 
						 tNative				&posteq_symbolnum,
						 TM_BOOL			band_scan, 
						 TM_BOOL			&drm_found, 
						 UInt8				*m_change_flags,
						 tNative			*DRMLevel,
						 tUNativeAcc	sample_rate_dev);
						 


	// *** Access Methods ***
	tNative GetService();
	
	UInt8 GetRF(void);
	
	UInt8 GetDRMLevel(void);

	void GetSDCRaw(	tNativeAcc		type, 
					UInt8	*address, 
					tNativeAcc		length,
					tNativeAcc		*retType, 
					tNativeAcc		*retLen, 
					UInt8	**retAdd);

	void GetFACRaw(	tNativeAcc		type, 
					UInt8	*address, 
					tNativeAcc		length,
					tNativeAcc		*retType, 
					tNativeAcc		*retLen, 
					UInt8	**retAdd);

	void GetMER(	tNativeAcc		type, 
					UInt8	*address, 
					tNativeAcc		length,
					tNativeAcc		*retType, 
					tNativeAcc		*retLen, 
					UInt8 **retAdd);

	void GetDop(	tNativeAcc		type, 
					UInt8	*address, 
					tNativeAcc		length,
					tNativeAcc		*retType, 
					tNativeAcc		*retLen, 
					UInt8	**retAdd);


	void GetDel(	tNativeAcc		type, 
					UInt8	*address, 
					tNativeAcc		length,
					tNativeAcc		*retType, 
					tNativeAcc		*retLen, 
					UInt8	**retAdd);
					

	void GetCIR(	tNativeAcc		type, 
					UInt8	*address, 
					tNativeAcc		length,
					tNativeAcc		*retType, 
					tNativeAcc		*retLen, 
					UInt8	**retAdd);

	void GetCW(		tNativeAcc		type, 
					UInt8	*address, 
					tNativeAcc		length,
					tNativeAcc		*retType, 
					tNativeAcc		*retLen, 
					UInt8	**retAdd);

	void GetServiceAct(tNativeAcc type,
		UInt8 *address,
		tNativeAcc length,
		tNativeAcc *retType,
		tNativeAcc *retLen,
		UInt8 **retAdd);


	void GetMisc(tNativeAcc type, 
		UInt8 *address, 
		tNativeAcc length,
		tNativeAcc *retType,
		tNativeAcc *retLen,
		UInt8 **retAdd);

	void GetServiceID(	tNativeAcc		type, 
						UInt8	*address, 
						tNativeAcc		length,
						tNativeAcc		*retType, 
						tNativeAcc	*retLen, 
						UInt8	**retAdd,
						UInt8	service);

	void GetServices(	tNativeAcc		type, 
						UInt8	*address, 
						tNativeAcc		length,
						tNativeAcc	*retType, 
						tNativeAcc		*retLen, 
						UInt8	**retAdd);

	void GetLabel(		tNativeAcc		type,
						UInt8	*address, 
						tNativeAcc		length,
						tNativeAcc		*retType, 
						tNativeAcc	*retLen, 
						UInt8	**retAdd,
						UInt8 service);

	void GetLabelNewSpec(tNativeAcc		type,
						UInt8	*address, 
						tNativeAcc		length,
						tNativeAcc		*retType, 
						tNativeAcc		*retLen, 
						UInt8	**retAdd,
						UInt8 service);

	void GetProg(		tNativeAcc		type, 
						UInt8	*address, 
						tNativeAcc		length,
						tNativeAcc		*retType, 
						tNativeAcc		*retLen, 
						UInt8	**retAdd,
						UInt8	service);

	void GetLang(		tNativeAcc		type, 
						UInt8	*address, 
						tNativeAcc		length,
						tNativeAcc		*retType, 
						tNativeAcc		*retLen, 
						UInt8	**retAdd,
						UInt8 service);

	void SetChannelFilter(	tNativeAcc		type, 
							UInt8	*address, 
							tNativeAcc	length,
							tNativeAcc		*retType, 
							tNativeAcc		*retLen, 
							UInt8	**retAdd);

	void GetAudioType(	tNativeAcc		type, 
						UInt8	*address, 
						tNativeAcc		length,
						tNativeAcc		*retType, 
						tNativeAcc		*retLen,                    
						UInt8	**retAdd,
						UInt8 service);

	void SetID(tNative id);
	void SetInter(int value);
	
	tNative ReadFilterBW();
	CDRMConfig* GetConfig();
	TM_BOOL ReadSyncStatus();
	TM_BOOL ReadSDCStatus();
	TM_BOOL ReadFACStatus();
	TM_BOOL ReadTjump();
	TM_BOOL ReadFjump();
	TM_BOOL ReadSjump();
	TM_BOOL ReadPreFFT();
	TM_BOOL ReadPostFFT();
	TM_BOOL ReadModeSearch();
	TM_BOOL ReadCWFlag();
	
	tNativeAcc ReadMinFP();	// * return_value/(1<<23)*OFDM_SAMLPE_RATE 
						// * -> gives min AFC frequency pull in Hz

	tNativeAcc ReadMaxFP();    // * return_value/(1<<23)*OFDM_SAMLPE_RATE 
						// * -> gives max AFC frequency pull in Hz

	tNative ReadMinTCS(); // * return_value/(1<<23)*OFDM_SAMLPE_RATE 
						// * -> gives min Timing loop frequency pull in Hz

	tNative ReadMaxTCS(); // * return_value/(1<<23)*OFDM_SAMLPE_RATE 
						// * -> gives max Timing loop frequency pull in Hz
	
	tNative ReadMaxSearch0();//not working
	tNative ReadMaxSearch(); //not working
	
	tNative ReadCSIMean();	// * returns CSI mean value
	
	tNative ReadCSIPeak();	// * returns CSI peak value

	tNative ReadCSIPeakMean();// * returns CSI peak to mean value
								
	tNative ReadCSIPeakPos(); // * returns CSI peak position in whole nr of Hz
	
	tNative ReadCIRPeak();	// * returns Impulse response peak value

	tNative ReadCIRPeakPos(); // * return_value/(m_upsample_factor*2*(OFDM_SAMPLE_RATE*0.001))
							// * -> gives impulse response peak position in ms (milli seconds)
	
	tNative ReadTotalMass();  // not working
	tNative ReadMoM1(); // not working
	tNative ReadMoM2(); // not working

	Int16 ReadCWPos(); // * returns the bin in which an interferer has been found
	
	tNative* ReadSignalLevel();	
	// *** Returns a pointer to an array that stores the (average sqared magnitude)/(1<<11) 
	// *** at the point where the function SignalLevels is placed, for all symbols in 1 frame. 
	// *** To use the value to calculate RF levels: take into account calibration 
	// *** factors and gains in the front-end and all the way to the point where the function
	// *** SignalLevels is placed.

	tNative ReadPeakLevel();		// not working
	
	tNative ReadTCS();  // * return_value/(1<<23) * OFDM_SAMPLE_RATE
					  // * -> gives frequency pull in Hz


	tNative ReadDoppler(void){return(m_doppler_shift);}
	// * ((OFDM_SAMPLE_RATE/total_symbol) * sqrt((4.0f/(return_value)))/PI)
	// * -> gives the Doppler value in Hz

	tNative* ReadMER(void); // * returns a pointer to an array of MER values in fxp base 7
						  // * return_value[x]/(1<<7) 
						  // * -> gives each MER value in dB 

	float ReadDelayIntervals(tNativeAcc i){	return(0);}  // not working
	
	tNative CheckRaw(); // keep 
//	tNative ReadServiceSelected();  // keep 



	void WritePC(UInt16 value); // keep
	void ResetCRCCount(void);   // keep
	void ConfigRec();
	void Config_Change_Command(TConfig_Command* config_command);
	void Guard_remove(tNative* data_in, tNativeAcc start_pos);

	UInt16 ReadSuperFrameCount(void){	return m_super_frame_count;}
	UInt16 ReadFrameCount(void)		{	return m_frame_count;}
	UInt16 ReadFACCheckSums(void)	{	return m_fac_check_sum_fails;}
	UInt16 ReadSDCCheckSums(void)	{	return m_sdc_check_sum_fails;}
	UInt16 ReadStatus();
	UInt16 ReadNumberSymbols(void);
	unsigned char* ReadFACRaw(tNativeAcc& length);
	unsigned char* ReadSDCRaw(tNativeAcc& length);
	unsigned char* ReadRawData(tNativeAcc& length);
	TM_BOOL ReacquireMode( TMode mode);
	TM_BOOL CheckFAC();
	TM_BOOL CheckSDC();
	TMode ReadMode();
	tSDCStruct ReadSDC(void){	return(m_sdc_struct);}
	tFACStruct ReadFAC(void){	return(m_fac_struct);}
	int ReadFP(void){	return(m_afc_pull);} 
	
	
	CDrmProc();
	virtual ~CDrmProc();

private:

	TM_BOOL         m_bTrefOnFlag;
	tAudioStore     m_old_audio_store[MAX_NO_SERVICES];
	tAudioStore     m_audio_store[MAX_NO_SERVICES];
	tLangStore      m_old_language_store[MAX_NO_SERVICES];
	tLangStore      m_language_store[MAX_NO_SERVICES];
	tProgStore      m_old_programme_store[MAX_NO_SERVICES];
	tServiceIDStore m_old_service_ident[MAX_NO_SERVICES];
	tMiscStore		m_misc_store;
	tMiscStore		m_old_misc_store;
	char            m_services;
	char            m_old_services;
	tLabelStore     m_service_label_store[MAX_NO_SERVICES];
	CPostInterpAFC  m_post_interp_afc;

	// *** Member variables ***

	tNative* m_signal_level;
	tNative m_doppler_shift;

	CDCFilter m_dc_filter;
	tNative * m_mode_detect_monitoring;

	tNativeAcc m_interferer_freq; /*	fxp base 23 
								set by: CCWFinder::FindCW 
								and used by :
								CNotchFilter::filter
								CPostInterpAFC::FindFreqErr
								CTRefFinder::find_refs		*/	

	tNative * m_notch_out;
	tNative * m_pFilteredSpectrum;
	tNativeAcc* m_int_freq;
	tNativeAcc* m_store_freq;
	
	TM_BOOL m_tjump_flag;
	TM_BOOL m_fjump_flag;
	TM_BOOL m_sjump_flag;
	TM_BOOL m_prefft_flag;
	TM_BOOL m_postfft_flag;
	TM_BOOL m_mode_search_flag;
	TM_BOOL m_cw_flag;
	TM_BOOL m_bDrmOk;
	TM_BOOL m_new_mode;

	TM_BOOL m_bInterFlag;
	
	tNativeAcc m_min_FP; // * 32bit
	tNativeAcc m_max_FP; // * 32bit
	tNative m_min_TCS;
	tNative m_max_TCS;

	tNative m_csi_mean;
	tNative m_csi_peak;
	tNative m_csi_peak_mean;
	tNative m_csi_peak_pos;
	
	tNative m_cir_peak;
	tNative m_cir_peak_pos;
	Int16 m_cw_pos;

	TM_BOOL m_buffer_1;
	UInt32 m_buff_2_size;
	UInt32 m_buff_1_size;
	UInt32 m_buff_1_pos;
	UInt32 m_buff_2_pos;
//	int m_raw_data_flag;
	tNative m_hand_back_count_time;
	tNative m_hand_back_count_freq;
	tNative m_old_service;
	UInt32 m_signal_fail;
	tSDCStruct m_sdc_struct;
	tFACStruct m_fac_struct;
	tNative* m_pShortIn1;
	tNative* m_pShortIn2;
	
	tNative* m_channel;
	tNative* m_channel_state;
	tNative* m_weights;
	tNative* m_fft_out;
	tNative* m_fft_cw_out;
	tNative* m_mag_store;
	tNative* m_imp_resp;
	tNative* m_correlations;
	tNative* m_time_dat;
	tNative* m_search_output;
	tNative* m_smoother_out;
	tNative* m_raw_data_out_a;
	tNative* m_raw_data_out_b;

	tUNativeAcc m_free_symbolnum;
	UInt16 m_frame_count;
	UInt16 m_super_frame_count;
	UInt16 m_sdc_check_sum_fails;
	UInt16 m_fac_check_sum_fails;
	tNativeAcc m_jump;
	tNativeAcc m_shift;
	tNativeAcc m_FAC_Length;
	tNativeAcc m_SDC_Length;
	tVitNative* m_sdcbits;
	tVitNative* m_opdata;
	unsigned char* m_FACRawBits;
	unsigned char* m_SDCRawBits;

	UInt8 m_signal_level_ssi;
	UInt8 m_drm_signal_level_ssi;

	UInt8 m_mer_ssi[2];
	UInt8 m_doppler_ssi[2];
	UInt8 m_delay_ssi[2];
	UInt8 m_cir_ssi[2];
	UInt8 m_cw_ssi[4];
	UInt8 m_service_act[1];

	tUNative m_fft_symbol_count;
	tNativeAcc m_sdc_data_index;
	tNativeAcc m_symbolnum;
	tNativeAcc m_framenum;
	tNativeAcc m_msc_deint_index;
	TM_BOOL m_mode_found;
	TM_BOOL m_bSignalPresent;
	TM_BOOL m_bCoarseFreqError;
	TM_BOOL m_bPost_FFT_timing;
	tNative* m_MER;    // * MER in fxp base 7
	
	tNative m_id_num;

	tNativeAcc m_afc_pull;	 // * fxp. base 23 * 32bit *
	tNativeAcc m_phase_err; // * fxp. base 23 * 32bit *

	tNative m_clock_freq_err; 
	tNative m_freq_pull;	   // * fxp base 23 *
	tNativeAcc m_best_search_pos; // * fxp base 9  * 32bit

	
	tFRAME_CELL * m_sdc_data;
	tFRAME_CELL * m_fac_data;

	UInt32* m_test1;


	// *** Member classes ***
	CCWInter			m_CWInter;
	CChannelFilter		m_channel_filter;
	CNotchFilter		m_notch_filter1;
	CNotchFilter		m_notch_filter2;
//	CCWFinder			m_cw_finder;
	CFourier			m_symbol_fourier;
	CFourier			m_symbol_fourier_cw;
	CImpulseResp		m_cir;
	CFACDecode			m_fac_decoder;
	CCSI				m_csi;
	CChannelEq			m_channeleq;
	CDeMap				m_demapper;
	CResample			m_upsamp;
	CTimLoopFilter		m_loopfilter;
	CSDCDecode			m_sdc_decoder;
	CEnergyDispersal	m_energy_dispersal;
	CMLCDecoder			m_mlc_decoder;
	CPRBSCheck			m_prbs_check;
	CTRefFinder			m_tref_finder;
	CDRMConfig			m_config;
	CSymbolDelay		m_symbol_delay;
	CFreqLoopFilter		m_freqloopfilter;
	CFreqCorrector		m_freqcorrector_pre_filt;
	CFreqCorrector		m_freqcorrector_post_filt;
	CTimeProc			m_time_proc;
	CModeDetect			m_mode_detect;
//	CDigAGC				m_DigAGC;
	CMonitor			m_monitor; // *** For monitoring ***


	// *** Memeber functions ***
	void ResetSDC();
	void ResetFAC();
	void ResetFACService(int service);
	void ResetLang();
	TM_BOOL SetLang();
	TM_BOOL SetAudioStore();
	TM_BOOL SetServices();

	// *** To get the signal levels Floating point, gain is the AGC gain to correct for ***
	//void SignalLevels(short* pShort, short symbol , short points, float gain);
	void SignalLevels(tNative* pShort, tNative symbol , tNative points);
	
	void ChangeMode(TMode mode);

	tNativeAcc PackBits(			const unsigned char *in, 
							unsigned char		*out, 
							tNativeAcc					len, 
							tNativeAcc					pad);

	void Channel_process(	tNative		*data_in, 
							tNative		*olddata, 
							tNative		&posteq_symbolnum, 
							tNative		&posteq_framenum, 
							tNativeAcc			&msc_data_length,
							tNativeAcc			&msc_data_index, 
							tFRAME_CELL	*msc_data_cells);

	void MLC_process(		tNative		posteq_symbolnum, 
							tNative		posteq_framenum, 
							TM_BOOL		&monit_ready, 
							TMode		&mode, 
							UInt8		*change_flags);

	void Post_FFT_process(	tNative		*data_in, 
							tNativeAcc			&msc_data_index);

	void Pre_FFT_process(	tNative		*data_in_fxp, 
							tNative		*DRMLevel,
							TM_BOOL		band_scan, 
							TM_BOOL		&drm_found);

	void FFT_process(		tNative		*data_in);

	void find_ang(tNative* data_in, 
				  tNative pos, 
				  tNative& angle);
	
	void find_COM(tNative			*data_in, 
				  tNative			&pos_of_com,
				  TM_BOOL		&post_fft_flag);

	void CheckLabels(UInt8* change_flags);
	void CheckLang(UInt8* change_flags);
	void CheckMisc(UInt8* change_flags);
	void CheckServiceID(UInt8* change_flags);
	void CheckProgramme(UInt8* change_flags);
	void CheckAudio(UInt8* change_flags);
//	void CheckActive(void);
	tNative fxp_10_log10(tUNativeAcc in);
	tNative fixed_sqrt(tNative x) ;

	void (CFourier::* m_FFTFuncPtr) (const fftw_complex*, fftw_complex*);


	// *** Main Process DRM ***
	TM_BOOL main_process(tNative			*data_in,		// * Input DRM data IQ at -5kHz reference *
						 tNativeAcc			&msc_data_index,// * MSC data index *
						 tFRAME_CELL	*msc_data_cells,// * MSC data streams *
						 TM_BOOL		&monit_ready,	// * Monitor array ready *
				//		 MonitorType	*trigger,		// * Monitor array with trigger pulses *
						 tNative			&posteq_framenum, // * frame number *
						 tNative			&posteq_symbolnum,// * symbol number *
						 TM_BOOL		band_scan,		  // * Band scan ? *
						 TM_BOOL		&drm_found,		  // * DRM found ? *
						 UInt8			*m_change_flags,
						 tNative	*DRMLevel,
						 tUNativeAcc	sample_rate_dev); // * flags change (rest from CAB but could be useful) *
						 
	
	tNative arctan2_fxp(tNative y, tNative x); 

protected:
	
};

#endif // !defined(AFX_DRMPROC_H__44162991_EF8E_11D3_8B00_00C04FA11AF6__INCLUDED_)
