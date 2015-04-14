// DRMConfig.h: interface for the CDRMConfig class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DRMCONFIG_H__A87F4C53_03C1_11D4_818D_00C04F7DD1B3__INCLUDED_)
#define AFX_DRMCONFIG_H__A87F4C53_03C1_11D4_818D_00C04F7DD1B3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../structurescommon.h"	// Added by ClassView
#include "../configcommands.h"	// Added by ClassView
//#include "../hardwaredefs.h"
#include "../S_Defs.h"
#include "../bbc_types.h"

#define MON_POINTS 18

struct tBASIC_PARAM{
	tNative fft_size;
	tNative guard_interval;
	tNative fspacing;
	tNative fstep;
	tNative pilot_index_symb0;
	tNative syms_per_frame;
};

struct tEXTENDED_PARAM{
	tNative kmin;
	tNative kmax;
	tNative data_cell_per_frame;
	tNative sdc_cell_per_frame;
};


struct tMLCDefinition
{
	tNative CodeRateIndices[3][2][2]; // * level axis part
	tNative CodeRateLCMA;			  // * Part A LCM
	tNative NetBitsPerLCMA;			  // * Total net bits in (CodeRateLCMA) cells
	TConstellation Constellation;
	THierarchy Hierarchy;
	tNative TailBitMode;
	tNative NumBitsA;				  // * Minimum number of bits in part A
};

class CDRMConfig  
{
public:
	TDiversityMode DiversityMode(void);
	TAFCErrSource AFCErrSource(void);
	TTimFreqSource TimFreqSource(void);
	tNative fac_index(tNative symbol);
	tNative min_kmin(void);
	void ResetSpecOcc(TMode mode);
	void SetService(tNative service);
	tMLCDefinition msc_mlc_definition(void);
	tMLCDefinition sdc_mlc_definition(void);
	tMLCDefinition fac_mlc_definition(void);
	TInterleaverDepth interleaver_depth(void);
	TConstellation msc_constellation(void);
	TConstellation sdc_constellation(void);
	TMLC mode_detect_stat(void) {return (m_mode_detect);}
	TMLC mlc_stat(void) {return (m_mlc_stat);}
	TMLC dubja_stat(void) {return (m_dubja_stat);}
	TMLC invert(void) {return( m_spectrum_invert);}
	TMLC channel_filter(void) {return (m_channel_filter);}
	
	TFilt filtering(void) {return (m_filt_command);}
	TLoop timing_control(void) {return (m_time_loop_command);}
	TLoop frequency_control(void) {return (m_freq_loop_command);}
	TMode mode(void);

	// * for Monitor class *
	TMonitor monitor1(void) {return (m_monitor1);}
	TMonitor monitor2(void) {return (m_monitor2);}
	
	tNative gp0(void) {return (m_gp0);}
	tNative gp1(void) {return (m_gp1);}
	tNative gp2(void) {return (m_gp2);}
	tNative gp3(void) {return (m_gp3);}

	// * fxp *
	int fshift_to_sym_fxp(void);
	int fshift_to_sym_fxp_invert(void);

	// * fxp *
	int fshift_to_dc_fxp(void);

	void SetMLCStat(TMLC value);
	void SetAutoDetect(TMLC value);
	void ReConfigureFromSDC(tSDCStruct *sdc);
	void ReConfigureFromFAC(tFACStruct *fac_struct);
	void ReConfigureMode(TMode mode);
	void command(TConfig_Command* config_command);

	tNative fac_cell_per_frame(void);
	tNative sdc_cell_per_frame(void);
	tNative data_cell_per_frame(void);
	tNative pilot_index_symb0(void);
	tNative kmax(void);
	tUNative syms_per_frame(void);
	tNative fftsize(void);
	tNative guard_interval(void);
	tNative kmin(void);
	tNative fstep(void);
	tNative tspacing(void);
	tNative fspacing(void);
	tNative dc_bin(void);
	tNative firstpilot(tNative symbolnum);
	tNative service_selected(void);
	tNative spec_occ(void);
	CDRMConfig();
	virtual ~CDRMConfig();
	
private:

	TLoop m_time_loop_command;
	TLoop m_freq_loop_command;

	// * General Purpose Controls *
	tNative m_gp0;
	tNative m_gp1;
	tNative m_gp2;
	tNative m_gp3;

	TDiversityMode m_DiversityMode;
	TTimFreqSource m_TimFreqSource;
	TAFCErrSource m_AFCErrSource;
	tNative m_protection_level_VSPP;
	THierarchy m_hierarchy;
	tMLCDefinition m_msc_mlc_definition;
	tMLCDefinition m_sdc_mlc_definition;
	tMLCDefinition m_fac_mlc_definition;
	TInterleaverDepth m_interleaver_depth;
	TConstellation m_msc_constellation;
	TConstellation m_sdc_constellation;
	TMLC m_dubja_stat;
	TMode m_receiver_mode;
	TMonitor m_monitor1;
	TMonitor m_monitor2;
	TFilt m_filt_command;
	TMLC m_mlc_stat;
	TMLC m_spectrum_invert;
	TMLC m_mode_detect;
	TMLC m_channel_filter;
	void CalculateParams(void);
	tNative m_guard_interval;
	tNative m_protection_level[2];
	tNative m_spec_occ;
	tNative m_NumBytesA;
	tNative m_fac_cell_per_frame;
	tNative m_sdc_cell_per_frame;
	tNative m_data_cell_per_frame;
	tNative m_pilot_index_symb0;
	tNative m_fspacing;
	tNative m_tspacing;
	tNative m_fftsize;
	tUNative m_syms_per_frame;
	tNative m_fstep;
	tNative m_kmax;
	tNative m_kmin;
	tNative m_dc_bin;
	tNative m_service_selected;
	static const tEXTENDED_PARAM m_mode_params_extended[NUM_MODES][(MAX_SPEC_OCC+1)];
	static const tBASIC_PARAM m_mode_params_basic[NUM_MODES];
	static const tNative m_CodeRateIndexTable[2][4][3];
	static const tNative m_CodeRateIndexTableHM[4][3][2];
	static const tNative m_CodeRateLCMTable[2][4];
	static const tNative m_CodeRateLCMTableHMSym[4];
	static const tNative m_CodeRateLCMTableHMMix[4];
	static const tNative m_CodeRateNetBitsPerLCMTable[2][4];
	static const tNative m_CodeRateNetBitsPerLCMTableHMSym[4];
	static const tNative m_CodeRateNetBitsPerLCMTableHMMix[4];
	static const tNative m_CodeRateIndexTableSDC16QAM[3];
	static const tNative m_CodeRateIndexTableSDC4QAM[3];
	static const tNative m_CodeRateIndexTableFAC[3];
	static const tNative m_mon_scale[MON_POINTS];
	
	// * fxp *
	static const tNativeAcc m_shift_table_sym_fxp[(MAX_SPEC_OCC+1)];
	static const tNativeAcc m_shift_table_sym_fxp_invert[(MAX_SPEC_OCC+1)];
	
	// * fxp *
	static const tNativeAcc m_shift_table_dc_fxp[NUM_MODES][(MAX_SPEC_OCC+1)];

	static const tNative m_spec_BW[(MAX_SPEC_OCC+1)];
	static const tNative m_fac_index_table[NUM_MODES][SYMBOLS_PER_FRAME_MALLOC];



};

#endif // !defined(AFX_DRMCONFIG_H__A87F4C53_03C1_11D4_818D_00C04F7DD1B3__INCLUDED_)
