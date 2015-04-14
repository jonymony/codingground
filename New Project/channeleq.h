// ChannelEq.h: interface for the CChannelEq class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHANNELEQ_H__A87F4C52_03C1_11D4_818D_00C04F7DD1B3__INCLUDED_)
#define AFX_CHANNELEQ_H__A87F4C52_03C1_11D4_818D_00C04F7DD1B3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../bbc_types.h"
#include "../S_Defs.h"
#include "DRMConfig.h"


#define TSPACING_A 5
#define TSPACING_B 3
#define TSPACING_C 2
#define TSPACING_D 3

#define MAX_TSPACING 5
#define MAX_TFILTER_LEN 8 // New design
#define PILOT_STORE_DEPTH (MAX_TSPACING*MAX_TFILTER_LEN)
#define MAX_PILOTS_PER_SYMBOL 75

// *** number of end carriers is (filterlength/2-1)*fstep +1 ***
// 3rd generation
#define END_CARRIERS_A 17 // (10/2-1)*4+1
#define END_CARRIERS_B 9  // (10/2-1)*2+1
#define END_CARRIERS_C 9  // (10/2-1)*2+1

#define END_TAP_LENGTH 10 // 3rd generation
#define MID_TAP_LENGTH 10 // 3rd generation

#define MID_PHASES_A 3
#define MID_PHASES_B 1
#define MID_PHASES_C 1

#define COS_TABLE_SIZE 1024
#define NO_MORE 32767

//typedef int tNative;

class CChannelEq  
{
public:

	void UnTwist(tNative		*data, 
				 CDRMConfig *config, 
				 tNative		symbolnum);


	void EstimateChannelTimeInterp(tNative      *data, 
								   tNative      *channel, 
								   CDRMConfig *config, 
								   tNative      symbolnum);

	void EstimateChannelFreqInterp(tNative	    *channel, 
								   CDRMConfig   *config);
	
	int CompDelayLength(CDRMConfig *config);



	void EqualiseChannel(tNative      *data_in, 
						 tNative      *channel, 
						 tNative      *data_out, 
						 CDRMConfig *config);



	CChannelEq();
	virtual ~CChannelEq();

private:
	
	tNative* m_pilotphase;

	// * Maybe have a static cos table fxp *
	tNative* m_cos_table; 

	tNative* m_pilot_store_ptr[PILOT_STORE_DEPTH];
	tNative* m_pilot_store;

	static tNative m_timefilter_a[TSPACING_A][MAX_TFILTER_LEN];
	static tNative m_timefilter_b[TSPACING_B][MAX_TFILTER_LEN];
	static tNative m_timefilter_c[TSPACING_C][MAX_TFILTER_LEN];
	static tNative m_timefilter_d[TSPACING_D][MAX_TFILTER_LEN];

	static tNative m_freq_end_filter_a[END_CARRIERS_A][END_TAP_LENGTH];
	static tNative m_freq_end_filter_b[END_CARRIERS_B][END_TAP_LENGTH];
	static tNative m_freq_end_filter_c[END_CARRIERS_C][END_TAP_LENGTH];

	static tNative m_freq_mid_filter_a[MID_PHASES_A][MID_TAP_LENGTH];
	static tNative m_freq_mid_filter_b[MID_PHASES_B][MID_TAP_LENGTH];
	static tNative m_freq_mid_filter_c[MID_PHASES_C][MID_TAP_LENGTH];

	static tNative m_tref_table[NUM_MODES][MAX_TREFS][2];
	static tNative m_fref_table[NUM_MODES][NUM_FREFS+1][3];
	static tNative m_q1024_table[NUM_MODES];

	static tNative m_w1024_table[NUM_MODES][SYMBOLS_PER_FRAME_MALLOC];
	static tNative m_z256_table[NUM_MODES][SYMBOLS_PER_FRAME_MALLOC];

};

#endif // !defined(AFX_CHANNELEQ_H__A87F4C52_03C1_11D4_818D_00C04F7DD1B3__INCLUDED_)
