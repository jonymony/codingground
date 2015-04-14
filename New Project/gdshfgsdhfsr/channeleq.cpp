// ChannelEq.cpp: implementation of the CChannelEq class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "ChannelEq.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "../pc_types.h"


// *** Macros for UnTwist function ***
#define UNTWIST_BITS 16 
#define UNTWIST_MULTIPLY_SHIFT UNTWIST_BITS-1 
#define UNTWIST_ONE ((1<<UNTWIST_MULTIPLY_SHIFT)-1)


//***************************************************************************************
//*	 *** Filter Coefficients For EstimateChannelTimeInterp ***     Fixed Point Version 
//***************************************************************************************

// New 8 tap filter. Design BW is 4.125Hz. 2 taps one side, 6 the other
#define NM(x) ((tNative)x) //base 9 fxp

// 3rd Generation 26/06/02 (Not changed from previous generation)
tNative CChannelEq::m_timefilter_a[TSPACING_A][MAX_TFILTER_LEN]=
{
    {NM(-30), NM(117), NM(453), NM(-26), NM(-22), NM(32), NM(-22), NM(10)}, 
    {NM(-40), NM(217), NM(400), NM(-88), NM(18), NM(10), NM(-13), NM(8)}, 
	{NM(-40), NM(323), NM(300), NM(-105), NM(38), NM(-4), NM(-7), NM(7)}, 
	{NM(-26), NM(423), NM(163), NM(-76), NM(33), NM(-8), NM(-3), NM(6)},
	{0, 512, 0, 0, 0, 0, 0, 0}
};

// 3rd Generation 26/06/02
tNative CChannelEq::m_timefilter_b[TSPACING_B][MAX_TFILTER_LEN]=
	{{NM(-34),NM(188),NM(398),NM(-30),NM(-42),NM(53),NM(-34),NM(13)}, //phase 5
	{NM(-34),NM(360),NM(240),NM(-66),NM(-3),NM(28),NM(-25),NM(12)},   //phase 4
	{0, 512, 0, 0, 0, 0, 0, 0}}; //phase 3

// 3rd Generation 26/06/02
tNative CChannelEq::m_timefilter_c[TSPACING_C][MAX_TFILTER_LEN]=

	{{NM(-35),NM(300),NM(258),NM(28),NM(-55),NM(9),NM(22),NM(-15)}, //phase 3
	{0, 512, 0, 0, 0, 0, 0, 0}}; //phase 2

// 3rd Generation 26/06/02 (Mode D was previously the same as mode B)
tNative CChannelEq::m_timefilter_d[TSPACING_D][MAX_TFILTER_LEN]=
	{{NM(-30),NM(165),NM(452),NM(-108),NM(42),NM(-11),NM(0),NM(2)}, //phase 5
	{NM(-36),NM(354),NM(267),NM(-113),NM(54),NM(-19),NM(3),NM(2)}, // phase 4
	{0, 512, 0, 0, 0, 0, 0, 0}}; //phase 3


//*****************************************************************************************
//*	 *** Filter Coefficients For EstimateChannelFreqInterp ***     Fixed Point Version 
//*****************************************************************************************

#define NM2(x) ((tNative)x)

//3rd Generation 26/06/02

tNative CChannelEq::m_freq_end_filter_a[END_CARRIERS_A][END_TAP_LENGTH]={
	{ 512, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //phase 0
	{NM2(333),NM2(290),NM2(-170),NM2(78),NM2(-3),NM2(-38),NM2(39),NM2(-23),NM2(7),NM2(-1)}, //phase 1
	{NM2(180),NM2(482),NM2(-236),NM2(132),NM2(-58),NM2(17),NM2(-9),NM2(10),NM2(-10),NM2(4)}, //phase 2
	{NM2(83),NM2(508),NM2(-93),NM2(1),NM2(38),NM2(-39),NM2(17),NM2(0),NM2(-6),NM2(3)}, // phase 3
	{ 0, 512, 0, 0, 0, 0, 0, 0, 0, 0 }, // phase 4
	{NM2(-25),NM2(395),NM2(192),NM2(-62),NM2(2),NM2(24),NM2(-18),NM2(5),NM2(4),NM2(-5)}, // phase 5
	{NM2(-44),NM2(292),NM2(304),NM2(-29),NM2(-38),NM2(42),NM2(-15),NM2(-7),NM2(13),NM2(-6)},// phase 6
	{NM2(-35),NM2(156),NM2(412),NM2(-9),NM2(-31),NM2(20),NM2(5),NM2(-17),NM2(15),NM2(-4)}, //phase 7
	{ 0, 0, 512, 0, 0, 0, 0, 0, 0, 0 }, // phase 8
	{NM2(-6),NM2(-16),NM2(383),NM2(205),NM2(-75),NM2(14),NM2(11),NM2(-11),NM2(3),NM2(4)}, // phase 9
	{NM2(4),NM2(-54),NM2(305),NM2(289),NM2(-21),NM2(-39),NM2(39),NM2(-16),NM2(-3),NM2(8)}, // phase 10
	{NM2(20),NM2(-80),NM2(225),NM2(332),NM2(53),NM2(-67),NM2(32),NM2(3),NM2(-16),NM2(10)}, // phase 11
	{ 0, 0, 0, 512, 0, 0, 0, 0, 0, 0 }, // phase 12
	{NM2(20),NM2(-44),NM2(33),NM2(342),NM2(217),NM2(-57),NM2(-21),NM2(44),NM2(-32),NM2(10)}, // phase 13
	{NM2(13),NM2(-16),NM2(-27),NM2(280),NM2(302),NM2(-22),NM2(-51),NM2(57),NM2(-33),NM2(9)}, // phase 14
	{NM2(5),NM2(14),NM2(-72),NM2(218),NM2(338),NM2(54),NM2(-79),NM2(52),NM2(-20),NM2(2)}, // phase 15
	{ 0, 0, 0, 0, 512, 0, 0, 0, 0, 0 }}; // phase 16


//3rd Generation 26/06/02
tNative CChannelEq::m_freq_end_filter_b[END_CARRIERS_B][END_TAP_LENGTH]={
	{ 512, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //phase 0
	{NM2(185),NM2(481),NM2(-249),NM2(156),NM2(-78),NM2(16),NM2(15),NM2(-22),NM2(13),NM2(-5)}, //phase 1
	{ 0, 512, 0, 0, 0, 0, 0, 0, 0, 0 }, //phase 2
	{NM2(-33),NM2(251),NM2(387),NM2(-144),NM2(75),NM2(-36),NM2(17),NM2(-7),NM2(5),NM2(-3)}, //phase 3
	{ 0, 0, 512, 0, 0, 0, 0, 0, 0, 0 }, //phase 4
	{NM2(17),NM2(-70),NM2(313),NM2(300),NM2(-47),NM2(-17),NM2(37),NM2(-33),NM2(18),NM2(-6)}, //phase 5
	{ 0, 0, 0, 512, 0, 0, 0, 0, 0, 0 }, // phase 6
	{NM2(2),NM2(12),NM2(-66),NM2(307),NM2(307),NM2(-58),NM2(-3),NM2(21),NM2(-18),NM2(8)}, // phase 7
	{ 0, 0, 0, 0, 512, 0, 0, 0, 0, 0 } // phase 8
};


//3rd Generation 26/06/02 (2nd generation was incorrectly the same as B)
tNative CChannelEq::m_freq_end_filter_c[END_CARRIERS_C][END_TAP_LENGTH]={
	{ 512, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //phase 0
	{NM2(193),NM2(483),NM2(-285),NM2(237),NM2(-202),NM2(156),NM2(-113),NM2(67),NM2(-36),NM2(12)}, //phase 1
	{ 0, 512, 0, 0, 0, 0, 0, 0, 0, 0 }, //phase 2
	{NM2(-46),NM2(262),NM2(388),NM2(-158),NM2(103),NM2(-65),NM2(41),NM2(-21),NM2(10),NM2(-2)}, //phase 3
	{ 0, 0, 512, 0, 0, 0, 0, 0, 0, 0 }, //phase 4
	{NM2(22),NM2(-66),NM2(287),NM2(353),NM2(-125),NM2(68),NM2(-40),NM2(20),NM2(-9),NM2(2)}, //phase 5
	{ 0, 0, 0, 512, 0, 0, 0, 0, 0, 0 }, // phase 6
	{NM2(-13),NM2(35),NM2(-84),NM2(310),NM2(327),NM2(-100),NM2(48),NM2(-20),NM2(9),NM2(0)}, // phase 7
	{ 0, 0, 0, 0, 512, 0, 0, 0, 0, 0 } // phase 8
};

//3rd Generation 26/06/02
tNative CChannelEq::m_freq_mid_filter_a[MID_PHASES_A][MID_TAP_LENGTH]=
{
	{NM2(-13),NM2(47),NM2(-80),NM2(65),NM2(330),NM2(216),NM2(-52),NM2(-14),NM2(29),NM2(-16)}, // phase 17
	{NM2(-16),NM2(41),NM2(-47),NM2(-8),NM2(286),NM2(286),NM2(-8),NM2(-47),NM2(41),NM2(-16)}, // phase 18
	{NM2(-16),NM2(29),NM2(-14),NM2(-52),NM2(216),NM2(330),NM2(65),NM2(-80),NM2(47),NM2(-13)} // phase 19
};


//3rd Generation 26/06/02
tNative CChannelEq::m_freq_mid_filter_b[MID_PHASES_B][MID_TAP_LENGTH]=
{{NM2(-4),NM2(3),NM2(15),NM2(-69),NM2(311),NM2(311),NM2(-69),NM2(15),NM2(3),NM2(-4)}}; //phase 9



//3rd Generation 26/06/02
tNative CChannelEq::m_freq_mid_filter_c[MID_PHASES_C][MID_TAP_LENGTH]=
{{NM2(5),NM2(-16),NM2(42),NM2(-93),NM2(320),NM2(320),NM2(-93),NM2(42),NM2(-16),NM2(5)}}; //phase 9


tNative CChannelEq::m_tref_table[NUM_MODES][MAX_TREFS][2] =
{
	/* Mode A */
	{
		{17,973},{19,717},{21,264},{28,357},{29,357},{32,952},{33,440},{39,856},{40,88},
		{41,88},{53,68},{55,836},{56,836},{60,1008},{61,1008},{63,752},{71,215},{73,727},
		{NO_MORE, NO_MORE}
	},
	/* Mode B */
	{
		{14,304},{18,108},{20,620},{24,192},{26,704},{32,44},{36,432},{42,588},{44,844},
		{49,651},{50,651},{54,460},{56,460},{62,944},{66,940},{68,428},
		{NO_MORE, NO_MORE}
	},
	/* Mode C */
	{
		{8,722},{10,466},{12,214},{14,479},{16,516},{18,260},{22,577},{24,662},{28,3},
		{30,771},{32,392},{36,37},{38,37},{42,474},{45,242},{46,754}
	},
	/* Mode D */
	{
		{5,636},{6,124},{8,788},{9,200},{11,688},{12,152},{14,920},{15,920},{17,644},
		{18,388},{20,652},{23,176},{24,176},{26,752},{27,496},{29,432},{30,964},{32,452}
	}

};

tNative CChannelEq::m_fref_table[NUM_MODES][NUM_FREFS+1][3] = /* {k, phase even symbs, phase odd symbs}*/
{
	{{18,205,205}, {54,836,836}, {72,215,215}, {NO_MORE,NO_MORE}}, /* Mode A */
	{{16,331,331}, {48,651,651}, {64,555,555}, {NO_MORE,NO_MORE}}, /* Mode B */
	{{11,214,214}, {33,392,392}, {44,242,242}, {NO_MORE,NO_MORE}}, /* Mode C */
	{{7,788,276}, {21,1014,502}, {28,332,332}, {NO_MORE,NO_MORE}}  /* Mode D */
};


/* Q, W and Z tables */
tNative CChannelEq::m_q1024_table[NUM_MODES] = {36, 12, 12, 14};

tNative CChannelEq::m_w1024_table[NUM_MODES][SYMBOLS_PER_FRAME_MALLOC] = 
{
	{228, 455, 683, 910, 114, 341, 569, 796, 0, 228, 455, 683, 910, 114, 341}, /* Mode A */
	{512, 0, 512, 0, 512, 0, 512, 0, 512, 0, 512, 0, 512, 0, 512}, /* Mode B */
	{465, 931, 372, 838, 279, 745, 186, 652, 93, 559, 0, 465, 
		931, 372, 838, 279, 745, 186, 652, 93}, /* Mode C */
	{366, 731, 73, 439, 805, 146, 512, 878, 219, 585, 951, 
	293, 658, 0, 366, 731, 73, 439, 805, 146, 512, 878, 219, 585} /* Mode D */
};

tNative CChannelEq::m_z256_table[NUM_MODES][SYMBOLS_PER_FRAME_MALLOC] =
{
	{0, 18, 122, 129, 33, 81, 106, 116, 129, 32, 248, 106, 31, 39, 111}, /* Mode A */
	{0, 168, 25, 57, 255, 232, 164, 161, 132, 64, 106, 233, 12, 118, 38}, /* Mode B */
	{0, 179, 76, 178, 29, 83, 76, 253, 9, 127, 190, 105, 161, 101, 248, 198, 33, 250, 108, 145}, /* Mode C */
	{0, 110, 165, 240, 7, 7, 17, 78, 252, 60, 82, 124, 220, 175, 253, 38, 150, 177, 151, 106, 197, 101, 25, 142} /* Mode D */
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CChannelEq::CChannelEq()
{
	tNative i;

	m_pilot_store = new tNative[PILOT_STORE_DEPTH*MAX_PILOTS_PER_SYMBOL*2];
	
	// *** Initialise pilot store ***
	tNative *p = m_pilot_store;
	for (i=0; i<PILOT_STORE_DEPTH; i++)
	{
		m_pilot_store_ptr[i] = p;
		p += MAX_PILOTS_PER_SYMBOL*2;
	}

	// Clear the pilot store to avoid the curry house effect (lots of NaNs)
	for (i=0; i<PILOT_STORE_DEPTH*MAX_PILOTS_PER_SYMBOL*2; i++)
		m_pilot_store[i] = 0; 

	m_cos_table=new tNative[COS_TABLE_SIZE];

	m_pilotphase=new tNative[2*FFT_SIZE_MALLOC]; 

	// *** Maybe use a static cos table ? ***
	float phase;
	for (i=0; i<COS_TABLE_SIZE; i++)
	{
		phase=(float) (2*i) * PI / COS_TABLE_SIZE;
		m_cos_table[i] = NATIVE_CAST( UNTWIST_ONE * cos(phase) );
	}

#define PHASE_MASK 1023

#define COS_LU(x) (m_cos_table[x & PHASE_MASK])
#define SIN_LU(x) (m_cos_table[(x-256) & PHASE_MASK])


}

CChannelEq::~CChannelEq()
{
	delete [] m_cos_table;
	delete [] m_pilotphase;
	delete [] m_pilot_store;
}


//***************************************************************************
//*
//*			Function name : *** EstimateChannelTimeInterp ***
//*
//*			Description : Temporal Interpolation
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

void CChannelEq::EstimateChannelTimeInterp(tNative		*data,    // * Input Vector *
										   tNative		*channel, // * Output Vector *
										   CDRMConfig	*config, 
										   tNative		symbolnum)
{
	tNative i;

	// *** Rotate pilot store: 0 is most recent and PILOT_STORE_DEPTH-1 is oldest ***
	tNative j;

	tNative*  newstore=m_pilot_store_ptr[PILOT_STORE_DEPTH-1];

	for (i=PILOT_STORE_DEPTH-1; i>0; i--)
	{
		m_pilot_store_ptr[i]=m_pilot_store_ptr[i-1];
	}
	m_pilot_store_ptr[0]=newstore;
	
	// *** Copy the pilots from the current symbol into the store ***
	tNative fspacing2 = 2*config->fspacing();
	tNative tspacing  = config->tspacing();
	tNative fstep     = config->fstep();
	tNative jmax      = 2*(config->dc_bin() + config->kmax() );

	j=2*(config->dc_bin()+config->firstpilot(symbolnum) );

	// *** Overboosted pilots = 2 at each edge ***
	tNative jminboost = 2*(config->dc_bin() + config->kmin() + fstep);
	tNative jmaxboost = jmax - 2*fstep;

	// *** Start from an offset position in the pilot store so given carrier always goes in the ***
	// *** same position, whatever the spectral occupancy										***

	i = (config->firstpilot(symbolnum)-config->min_kmin()) / config->fspacing() * 2;

	for(; j<=jmax; i+=2, j+=fspacing2)
	{		
		tNative re, im, pre, pim;
		// *** Undo the pilot modulation ***
		pre = 1;
		pim = 0;

		re=data[j];
		im=data[j+1];

		if (j<=jminboost || j>=jmaxboost)
		{
			re = (tNative)(( re * 362 )>>9) ;
			im = (tNative)(( im * 362 )>>9) ;
		}

		newstore[i]  = re * pre  + im * pim;
		newstore[i+1]= re *-pim  + im * pre;

	}

	// *** Clear the output array (for display purposes only) ***
	memset(channel, 0, sizeof(tNative)* TOTAL_SYMBOL_COMPLEX_MALLOC);

	
	// *** Time interpolation ***
	tNative (*timefilter)[MAX_TFILTER_LEN];

	switch (config->mode())
	{
	case ground:
		timefilter = m_timefilter_a; break;
	case sky:
		timefilter = m_timefilter_b; break;
	case robust1:
		timefilter = m_timefilter_c; break;
	case robust2:
		timefilter = m_timefilter_d; break; 

	default:
		timefilter = NULL; 
	}
	
	// * k = phase number *
	for (tNative k=0; k<tspacing; k++) 
	{
		tNative minisymbolnum=(symbolnum+tspacing-k) % tspacing;

		tNative *h   = timefilter[k]; // * Select the coefficients for this phase *
		tNative j    = 2*(config->dc_bin() + config->firstpilot(minisymbolnum) ); // * j=cmplx symbol array index *
		tNative jmax = 2*(config->dc_bin() + config->kmax());

		// * i=0 corresponds to pilot with most negative k over all spectral occupancies *
		i = (config->firstpilot(minisymbolnum)-config->min_kmin()) / config->fspacing() * 2;

		for (; j<=jmax; i+=2, j+=fspacing2) // * i = complex pilot store array index *	
		{	
			// *** could use 16 bit accumulator and 
			// *** shift down before accumulation instead but gives worse performance
			int acc_re=0; // * 32 bit *
			int acc_im=0; // * 32 bit *

			for (tNative l=0; l<MAX_TFILTER_LEN; l++) // l=tap number
			{
				acc_re += (int) ((h[l] * m_pilot_store_ptr[k+l * tspacing][i]  ));
				acc_im += (int) ((h[l] * m_pilot_store_ptr[k+l * tspacing][i+1]));
			}
			channel[j]   = (tNative)(acc_re>>9);
			channel[j+1] = (tNative)(acc_im>>9);
		}
	}

}

//***************************************************************************
//*
//*			Function name : *** EqualiseChannel ***
//*
//*			Description : 
//*
//*			short		*data_in,   * correctly delayed unequalised data (input)
//*			short		*channel,	* channel response (input)
//*			short		*data_out,  * equalised data (output)
//*
//***************************************************************************

void CChannelEq::EqualiseChannel(tNative		*data_in, 
								 tNative		*channel, 
								 tNative		*data_out, 
								 CDRMConfig *config)
{
	tNative i;
	tNative minbin = config->dc_bin() + config->kmin();
	tNative maxbin = config->dc_bin() + config->kmax();
	
	for(i=0; i<minbin*2; i++) 
		data_out[i]=0;	

	for(i=maxbin*2; i<TOTAL_SYMBOL_COMPLEX_MALLOC; i++) 
		data_out[i]=0;

	tNative dre1, dim1, cre1, cim1, re1, im1, cmag2_1;

	tNative max_re1=0,max_im1=0;
	tNative max_cmag2=0;

	for (i=minbin*2; i<=maxbin*2; i+=2)
	{
		
		dre1=data_in[i];
		dim1=data_in[i+1];

		cre1=channel[i];
		cim1=channel[i+1];

		re1     = NATIVE_CAST((dre1 * cre1  + dim1 * cim1)>>12); //intermediate value scaled down to fit within range
		im1     = NATIVE_CAST((dre1 * -cim1 + dim1 * cre1)>>12); //intermediate value scaled down to fit within range

		cmag2_1 = NATIVE_CAST((cre1 * cre1  + cim1 * cim1)>>12); //scaled down same amount to fit in range

		if(cmag2_1 == 0) // * check for divide by zero *
			cmag2_1 = 1; 

		tNative sqrt2sqrt42 = 9385; // *** (1<<10) *sqrt(2) * sqrt(42) *** //level output of the final eq constellation is set here

		data_out[i]   = NATIVE_CAST((re1*sqrt2sqrt42)/cmag2_1); 
													 
		data_out[i+1] = NATIVE_CAST((im1*sqrt2sqrt42)/cmag2_1);							 
	}
}






//***************************************************************************
//*
//*			Function name : *** CompDelayLength ***
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

int CChannelEq::CompDelayLength(CDRMConfig *config)
{
	return(config->tspacing()*2-1);
}



//***************************************************************************
//*
//*			Function name : *** EstimateChannelFreqInterp ***
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

void CChannelEq::EstimateChannelFreqInterp(tNative	  *channel, 
										   CDRMConfig *config)
{
	// *** Frequency interpolation ***

	tNative i,j,p;
	tNative fstep = config->fstep();

	if (fstep==1) 
	{
		return; // * No frequency interpolation required if fstep = 1 *
	}

	tNative minbin = config->dc_bin() + config->kmin();
	tNative maxbin = config->dc_bin() + config->kmax();

	tNative (*freq_end_filter)[END_TAP_LENGTH];
	tNative (*freq_mid_filter)[MID_TAP_LENGTH];

	switch (config->mode())
	{
	case ground:
		freq_end_filter = m_freq_end_filter_a;
		freq_mid_filter = m_freq_mid_filter_a; 
		break;
	case sky:
		freq_end_filter = m_freq_end_filter_b;
		freq_mid_filter = m_freq_mid_filter_b; 
		break;
	case robust1:
		freq_end_filter = m_freq_end_filter_c; 
		freq_mid_filter = m_freq_mid_filter_c; 
		break;
	default:
		freq_end_filter = freq_mid_filter = NULL; 
		break;
	}

	// * First the end carriers *

	tNative carrier;
	tNative end_carriers = (END_TAP_LENGTH/2 - 1) * fstep + 1;

	for (j=0, carrier=2*minbin; j<end_carriers; j++, carrier+=2)
	{
		// *** could use 16 bit accumulator and 
		// *** shift down before accumulation instead but gives worse performance
		int acc_re=0; // * 32 bit
		int acc_im=0; // * 32 bit

		// * Always use the same pilots, just change the taps *
		for (i=0, p=2*minbin; i<END_TAP_LENGTH; i++, p+=2*fstep)
		{
			tNative channel1=channel[p]; 
			tNative channel2=channel[p+1];
			
			acc_re += NATIVE_CAST_ACC((freq_end_filter[j][i] * channel1)); 
			acc_im += NATIVE_CAST_ACC((freq_end_filter[j][i] * channel2)); 
		}															   
		
		channel[carrier]  = NATIVE_CAST(acc_re>>9);
		channel[carrier+1]= NATIVE_CAST(acc_im>>9);
	}

	tNative lastbin=maxbin-fstep*(MID_TAP_LENGTH-1);

	// * Now the middle carriers *
	for (carrier=2*minbin;	carrier<=2*lastbin; carrier+=2*fstep)
	{
		for (j=0; j<fstep-1; j++) // * number of phases of filter *
		{
			// *** could use 16 bit accumulator and 
			// *** shift down before accumulation instead but gives worse performance
			tNativeAcc acc_re=0; // * 32 bit
			tNativeAcc acc_im=0; // * 32 bit
			
			tNative q=carrier;
			
			for (p=0; p<MID_TAP_LENGTH; p++)
			{
				tNative channel1=channel[q]; 
				tNative channel2=channel[q+1];
				
				acc_re += NATIVE_CAST_ACC((freq_mid_filter[j][p] * channel1)); 
				acc_im += NATIVE_CAST_ACC((freq_mid_filter[j][p] * channel2));

				q += 2*fstep;
			}

			channel[carrier+2*j+2*end_carriers]   = NATIVE_CAST(acc_re>>9);
			channel[carrier+2*j+2*end_carriers+1] = NATIVE_CAST(acc_im>>9);
		} 
	}

	// * Finally the RH end carriers *

	for (j=0, carrier=2*maxbin; j<end_carriers; j++, carrier-=2)
	{
		// *** could use 16 bit accumulator and 
		// *** shift down before accumulation instead but gives worse performance
		tNativeAcc acc_re=0; // * 32 bit
		tNativeAcc acc_im=0; // * 32 bit

		// * Always use the same pilots, just change the taps *
		for (i=0, p=2*maxbin; i<END_TAP_LENGTH; i++, p-=2*fstep)
		{
			tNative channel1=channel[p]; 
			tNative channel2=channel[p+1];
		
			acc_re += NATIVE_CAST_ACC((freq_end_filter[j][i] * channel1)); 
			acc_im += NATIVE_CAST_ACC((freq_end_filter[j][i] * channel2));
		}
		
		channel[carrier]  = NATIVE_CAST(acc_re>>9);
		channel[carrier+1]= NATIVE_CAST(acc_im>>9);
	}
}




//***************************************************************************
//*
//*			Function name : *** UnTwist ***
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

void CChannelEq::UnTwist(tNative		*data, 
						 CDRMConfig *config, 
						 tNative		symbolnum)
{	
	tNative i;
	tNative dc_bin = config->dc_bin();
	tNative minbin = dc_bin + config->kmin();
	tNative maxbin = dc_bin + config->kmax();
	tNative fstep, tspacing, k0, fspacing;
	tNative re,im;
	tNative z1024, w1024, q1024;
	tNative p, beta, two_p_beta, theta1024;
	TMode robustness = config->mode();

	// *** Get parameters for GREFs *** 
	fstep = config->fstep();
	tspacing = config->tspacing();
	fspacing = fstep * tspacing;
	k0 = config->pilot_index_symb0();
	tNative nextgref=config->firstpilot(symbolnum);

	z1024 = 4 * m_z256_table[robustness][symbolnum];
	w1024 = m_w1024_table[robustness][symbolnum];
	q1024 = m_q1024_table[robustness];

	// *** Initialise the running values of p^2(1+s)Q1024  ***
	p = (nextgref - (k0 + fstep * (symbolnum % tspacing))) / (fstep * tspacing);
	beta = (1+symbolnum) * q1024; // beta is coeff of p^2 
	two_p_beta = 2* p * beta;     // =2p(1+s)q1024 
	theta1024 = z1024 + p * w1024 + p*p*beta;

	// *** Initialise rotation array to 'no rotation' ***
	for (i=minbin*2; i<=maxbin*2; i+=2) 
	{
		m_pilotphase[i]=UNTWIST_ONE; //1.0 FXP
		m_pilotphase[i+1]=0;
	}

	// *** Now do the gain references ***
	for (i=nextgref+dc_bin; i<= maxbin; i += fspacing)
	{
		re = COS_LU(theta1024);
		im = SIN_LU(theta1024);
		m_pilotphase[2*i]   = re;
		m_pilotphase[2*i+1] = im;

		// *** Update the running totals ***
		theta1024  += w1024 + two_p_beta + beta; 
		two_p_beta += 2*beta;
	}

	// *** Now do the Freq references ***
	for (i=0; i<NUM_FREFS; i++)
	{
		tNative k = dc_bin + m_fref_table[robustness][i][0];
		tNative phase = m_fref_table[robustness][i][1 + (symbolnum & 1)];
		m_pilotphase[2*k]   = COS_LU(phase); 
		m_pilotphase[2*k+1] = SIN_LU(phase); 
	}

	// *** Finally the Time Refs (only in symbol 0) ***
	if (symbolnum==0)
		for (i=0; i<MAX_TREFS; i++)
		{
			tNative k = m_tref_table[robustness][i][0];
			if (k==NO_MORE) break;
			k += dc_bin;
			tNative phase = m_tref_table[robustness][i][1];
			// TODO: phase alternations for mode D
			m_pilotphase[2*k] = COS_LU(phase); 
			m_pilotphase[2*k+1] = SIN_LU(phase); 
		}

	// *** Now rotate all carriers. This is a bit wasteful - perhaps improve later. ***
	for (i=minbin*2; i<=maxbin*2; i+=2)
	{
		tNative dre, dim, wre, wim, re, im;

		dre=data[i];
		dim=data[i+1];

		wre=m_pilotphase[i];
		wim=m_pilotphase[i+1];

		re = (dre*wre + dim*wim )>>UNTWIST_MULTIPLY_SHIFT;
		im = (dre*-wim + dim*wre)>>UNTWIST_MULTIPLY_SHIFT;
		data[i]=re;
		data[i+1]=im;
	}
}

