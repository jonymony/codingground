// MLCDecoder.cpp: implementation of the CMLCDecoder class.
//
//////////////////////////////////////////////////////////////////////
//#include "stdafx.h"
#include "MLCDecoder.h"
#include "../ConfigCommands.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "../pc_types.h"

/*
#ifdef _FXP
#define SCALE(x,y) (tNative)((x * y)>> 10)
#endif

#ifdef _FLP
#define SCALE(x,y) (x * y) 
#endif
*/
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CMLCDecoder::CMLCDecoder()
{
	// * Decoders for SM/HMSYM and real axis of HMMix *
	m_viterbi[0][0] = new CViterbiDecoder; 
	m_viterbi[1][0] = new CViterbiDecoder; 
	m_viterbi[2][0] = new CViterbiDecoder;
	// * Imaginary axis decoders for HMMix *
	m_viterbi[0][1] = new CViterbiDecoder; 
	m_viterbi[1][1] = new CViterbiDecoder; 
	m_viterbi[2][1] = new CViterbiDecoder;
	
	m_xy=(tNative*) _cache_malloc(2*MAX_MSC_PER_LFRAME*sizeof(tNative), -1 );       // Real and imaginary
	m_metrics1=(tNative*) _cache_malloc(2*MAX_MSC_PER_LFRAME*sizeof(tNative), -1 ); // Real and imaginary
	m_metrics2=(tNative*) _cache_malloc(2*MAX_MSC_PER_LFRAME*sizeof(tNative), -1 ); // Real and imaginary
	m_csi=(tNative*) _cache_malloc(MAX_MSC_PER_LFRAME*sizeof(tNative), -1 );
	

	m_recodedbits=(tVitNative*) _cache_malloc(2*MAX_MSC_PER_LFRAME*sizeof(tVitNative), -1 );
	m_ReintBitsLSB=(tVitNative*) _cache_malloc(2*MAX_MSC_PER_LFRAME*sizeof(tVitNative), -1 );
	m_ReintBitsCSB=(tVitNative*) _cache_malloc(2*MAX_MSC_PER_LFRAME*sizeof(tVitNative), -1 );
	m_ReintBitsMSB=(tVitNative*) _cache_malloc(2*MAX_MSC_PER_LFRAME*sizeof(tVitNative), -1 );

	m_InterleaverATable=new int[NUM_LEVELS][NUM_AXES];

	DP(("Created CMLCDecoder\n"));
}

CMLCDecoder::~CMLCDecoder()
{
	delete m_viterbi[0][0];	delete m_viterbi[0][1];
	delete m_viterbi[1][0];	delete m_viterbi[1][1];
	delete m_viterbi[2][0];	delete m_viterbi[2][1];

	_cache_free(m_xy);
	_cache_free(m_metrics1);
	_cache_free(m_metrics2);
	_cache_free(m_csi);
	_cache_free(m_recodedbits);
	_cache_free(m_ReintBitsLSB);
	_cache_free(m_ReintBitsCSB);
	_cache_free(m_ReintBitsMSB);

	delete [] m_InterleaverATable;
}

void CMLCDecoder::Decode(tFRAME_CELL *ipcells, 
						 tVitNative		 *opdata, 
						 int		 &numnetbits, 
						 int		 &numnetbitsVSPP)
{
	tVitNative *levelopdata, *axisopdata;
	int num_netbits[NUM_LEVELS][NUM_AXES];

	short startlevel;
	short axis;

	switch(m_constellation)
	{
	case qpsk	: startlevel = 2			; break;
	case qam16	: startlevel = 1			; break;
	case qam64	: default: startlevel = 0	; break;
	}

	numnetbits = 0;

	for (tNative level=startlevel; level<NUM_LEVELS; level++)
	{
		for (tNative a=0; a<(m_Hierarchy==hmmix ? 2: 1); a++)
		{
			num_netbits[level][a] = m_viterbi[level][a]->NumNetBits();
			numnetbits += num_netbits[level][a];
		}
	}

	if (m_Hierarchy==sm)
		numnetbitsVSPP=0;
	else
		numnetbitsVSPP=num_netbits[2][0];

	axisopdata = opdata;

	for ( axis = 0 ; axis < (m_Hierarchy==hmmix ? 2: 1) ; axis++)
	{
		levelopdata = axisopdata;
		CellsToXYCsi(ipcells, m_xy, m_csi, m_numcells, (m_Hierarchy==hmmix?axis:-1));
		
		// *** No HM for 4qam ***
		if (m_constellation == qpsk) 
		{
			m_viterbi[2][0]->QamMetricsMSB(m_xy, m_metrics2);

			m_viterbi[2][0]->WeightMetrics(m_metrics2, m_csi, m_metrics1);

			LevelDeinterleave(m_metrics1, m_metrics2, m_InterleaverATable[2][axis],2,axis);
			
			m_viterbi[2][0]->Decode(m_metrics2, levelopdata);

			numnetbits = num_netbits[2][0];

			return;
		}

		if (m_constellation == qam64)
		{
			// *** LSB ***
			m_viterbi[0][axis]->QamMetricsLSB(m_xy, m_metrics1);

			m_viterbi[0][axis]->WeightMetrics(m_metrics1, m_csi, m_metrics2);

			LevelDeinterleave(m_metrics2, m_metrics1, m_InterleaverATable[0][axis],0,axis);			

			m_viterbi[0][axis]->Decode(m_metrics1, levelopdata);
			// *** recode ***
			m_viterbi[0][axis]->ConvCodePunct(levelopdata, m_recodedbits); 

			LevelInterleave(m_recodedbits, m_ReintBitsLSB, m_InterleaverATable[0][axis],0,axis);

			levelopdata += num_netbits[0][axis];

			// *** CSB ***
			m_viterbi[1][axis]->SubtractBits(m_xy, m_ReintBitsLSB, m_metrics1, (1<<10)/*1.0f*/ /*AEGN*/);
			m_viterbi[1][axis]->QamMetricsCSB(m_metrics1, m_metrics2);
		}
		else
			m_viterbi[1][axis]->QamMetricsCSB(m_xy, m_metrics2);

		m_viterbi[1][axis]->WeightMetrics(m_metrics2, m_csi, m_metrics1);

		LevelDeinterleave(m_metrics1, m_metrics2, m_InterleaverATable[1][axis],1,axis); 

		m_viterbi[1][axis]->Decode(m_metrics2, levelopdata);
		
		// *** recode ***
		m_viterbi[1][axis]->ConvCodePunct(levelopdata, m_recodedbits); 

		LevelInterleave(m_recodedbits, m_ReintBitsCSB, m_InterleaverATable[1][axis],1,axis);
		levelopdata += num_netbits[1][axis];

		// *** MSB ***
		m_viterbi[2][axis]->SubtractBits(m_xy, m_ReintBitsCSB, m_metrics1, (1<<11) /*2.0f AEGN*/);

		m_viterbi[2][axis]->QamMetricsMSB(m_metrics1, m_metrics2);

		m_viterbi[2][axis]->WeightMetrics(m_metrics2, m_csi, m_metrics1);

		LevelDeinterleave(m_metrics1, m_metrics2, m_InterleaverATable[2][axis],2,axis); 
	
		m_viterbi[2][axis]->Decode(m_metrics2, levelopdata);
	
		m_viterbi[2][axis]->ConvCodePunct(levelopdata, m_recodedbits);

		LevelInterleave(m_recodedbits, m_ReintBitsMSB, m_InterleaverATable[2][axis],2,axis);
		
		// *** back to LSB for this axis ***
		levelopdata = axisopdata; 

		if (m_constellation == qam64)
		{
			// *** LSB again ***
			m_viterbi[0][axis]->SubtractBits(m_xy, m_ReintBitsCSB, m_metrics1, (1<<11)/*2.0f AEGN*/);

			m_viterbi[0][axis]->SubtractBits(m_metrics1, m_ReintBitsMSB, m_metrics2, (1<<12)/*4.0f AEGN*/);

			m_viterbi[0][axis]->QamMetricsLSB2(m_metrics2, m_metrics1);

			m_viterbi[0][axis]->WeightMetrics(m_metrics1, m_csi, m_metrics2);

			LevelDeinterleave(m_metrics2, m_metrics1, m_InterleaverATable[0][axis],0,axis);			

			m_viterbi[0][axis]->Decode(m_metrics1, levelopdata);

			// *** recode ***
			m_viterbi[0][axis]->ConvCodePunct(levelopdata, m_recodedbits);
			
			LevelInterleave(m_recodedbits, m_ReintBitsLSB, m_InterleaverATable[0][axis],0,axis);

			levelopdata += num_netbits[0][axis];

			// *** CSB again ***
			m_viterbi[1][axis]->SubtractBits(m_xy, m_ReintBitsLSB, m_metrics1, (1<<10)/*1.0f AEGN*/);

			m_viterbi[1][axis]->QamMetricsCSB(m_metrics1, m_metrics2);
		}
		else
		{
			m_viterbi[1][axis]->SubtractBits(m_xy, m_ReintBitsMSB, m_metrics1, (1<<12)/*4.0f AEGN*/);

			m_viterbi[1][axis]->QamMetricsMSB(m_metrics1, m_metrics2);
		}

		m_viterbi[1][axis]->WeightMetrics(m_metrics2, m_csi, m_metrics1);

		LevelDeinterleave(m_metrics1, m_metrics2, m_InterleaverATable[1][axis],1,axis); 

		m_viterbi[1][axis]->Decode(m_metrics2, levelopdata);

		// *** axisopdata should point to start of imag axis ***
		for (int l=startlevel; l<NUM_LEVELS; l++) 
			axisopdata+=num_netbits[l][axis];
	}


}

void CMLCDecoder::BER(tVitNative *data, 
					  tMLCErrorStruct errstruct[2], 
					  int numbytes, 
					  int numbytesVSPP, 
					  TM_BOOL reset)
{
	// *** New compliance PRBS ***
	tNative startlevel=0;
	tNative level;
	tNative axis;
	int part;
	tVitNative *leveldata = data;

	switch(m_constellation)
	{
	case qpsk: startlevel=2; break;
	case qam16: startlevel=1; break;
	case qam64: default: startlevel=0; break;
	}

	if (reset) m_prbs_check.ResetState();

	for (level=0; level<NUM_LEVELS; level++)
	{
		errstruct[0].bits[level].i = errstruct[1].bits[level].i = 0;
		errstruct[0].errors[level].i = errstruct[1].errors[level].i = 0;
	}

	// *** Count the VSPP errors. ***

	if (m_Hierarchy!=sm)
	{
		int numnetbits = m_viterbi[2][0]->NumNetBits();
		int errors = m_prbs_check.CountErrors(leveldata, 8*numbytesVSPP);
		leveldata += numnetbits;
		errstruct[1].bits[2].i=numnetbits;
		errstruct[1].errors[2].i=errors;
	}

	// *** for SPP ***
	int NetBitsLeft = numbytes*8; 

	// *** Now loop through all the levels for part A then part B ***
	for (part=0; part<2; part++)                        
	{              
		for (level=startlevel; level<3; level++)
		{

			int levelerrors=0;
			int numnetbits = 0;

			for (axis=(m_Hierarchy==hmmix ? 1: 0); axis>=0; axis--) //Do imag first in hmmix
			{
				numnetbits = m_viterbi[level][axis]->NumNetBitsPart(part);
				if (m_Hierarchy != sm && level==2 && axis==0) numnetbits=0; // VSPP already done

				if (numnetbits>NetBitsLeft) numnetbits = NetBitsLeft;
				levelerrors = m_prbs_check.CountErrors(leveldata, numnetbits);
				leveldata += numnetbits;
				NetBitsLeft -= numnetbits;
				errstruct[part].bits[level].i += numnetbits;
				errstruct[part].errors[level].i += levelerrors;
			}			
		}
	}
}

void CMLCDecoder::LevelPartDeinterleave(tNative *ipmetrics, 
										tNative *opmetrics, 
										int numbits, 
										int a)
{
	int Pi = 0;
	int p=1;

	// * Calculate p = smallest power of 2 >= numbits *
	int sr = numbits;
	while (sr)
	{
		sr >>= 1;
		p <<= 1;
	}
	if (p==numbits<<1) p>>=1; // * special case where n==2^i 

	int q = p/4-1;
	int mask = p-1;
	for (int i=0; i<numbits; i++)
	{
		// * Not really defined which direction is interleaving *
		opmetrics[Pi] = ipmetrics[i]; 

		do
		{
			Pi = ((a * Pi) + q) & mask;
		}
		while (Pi >= numbits);
	}

		
}

void CMLCDecoder::LevelPartInterleave(tVitNative *ipbits, 
									  tVitNative *opbits, 
									  int numbits, 
									  int a)
{
	int Pi = 0;
	int p=1;

	// * Calculate p = smallest power of 2 >= numbits *
	int sr = numbits;
	while (sr)
	{
		sr >>= 1;
		p <<= 1;
	}
	if (p==numbits<<1) p>>=1; /* special case where n==2^i */

	
	int q = p/4-1;
	int mask = p-1;
	for (int i=0; i<numbits; i++)
	{
		opbits[i] = ipbits[Pi]; // * Not really defined which direction is interleaving *

		do
		{
			Pi = ((a * Pi) + q) & mask;
		}
		while (Pi >= numbits);
	}
}


void CMLCDecoder::LevelInterleave(tVitNative *ipbits, 
								  tVitNative *opbits, 
								  int a, 
								  int level, 
								  int axis)
{
	if (a==0) // * No interleaving if a==0 *
	{
		memcpy(opbits, ipbits, (m_Hierarchy==hmmix?1:2)*m_numcells*sizeof(char));
		for (int i=0; i<2*m_numcells; i++) opbits[i]=ipbits[i];
	}
	else if (m_Hierarchy == hmsym && level==2) // * MSB carries VSPP in rx terms *
	{
		LevelPartInterleave(ipbits, opbits, m_numcells*2, a);
	}
	else if (m_Hierarchy == hmmix && level==2 && axis==0)
	{
		LevelPartInterleave(ipbits, opbits, m_numcells, a);
	}
	else if (m_Hierarchy == hmmix)
	{
		// * Part A *
		LevelPartInterleave(ipbits, opbits, m_numcellsA, a);

		// * Part B *
		LevelPartInterleave(ipbits+m_numcellsA, opbits+m_numcellsA, 
				(m_numcells-m_numcellsA), a);
	}
	else
	{
		// * Part A *
		LevelPartInterleave(ipbits, opbits, m_numcellsA*2, a);

		// * Part B *
		LevelPartInterleave(ipbits+m_numcellsA*2, opbits+m_numcellsA*2, 
				2*(m_numcells-m_numcellsA), a);
	}

}

void CMLCDecoder::LevelDeinterleave(tNative *ipmetrics, 
									tNative *opmetrics, 
									int a, 
									int level, 
									int axis)
{
	if (a==0) // * No interleaving if a==0 *
	{
		memcpy(opmetrics, ipmetrics, (m_Hierarchy==hmmix?1:2)*m_numcells*sizeof(tNative));
	}
	else if (m_Hierarchy == hmsym && level==2)
	{
		LevelPartDeinterleave(ipmetrics, opmetrics, m_numcells*2, a);
	}
	else if (m_Hierarchy == hmmix && level==2 && axis==0)
	{
		LevelPartDeinterleave(ipmetrics, opmetrics, m_numcells, a);
	}
	else if (m_Hierarchy == hmmix)
	{
		// * Part A *
		LevelPartDeinterleave(ipmetrics, opmetrics, m_numcellsA, a);

		// * Part B *
		LevelPartDeinterleave(ipmetrics+m_numcellsA, opmetrics+m_numcellsA, 
				(m_numcells-m_numcellsA), a);
	}
	else
	{
		// * Part A * 
		LevelPartDeinterleave(ipmetrics, opmetrics, m_numcellsA*2, a);

		// * Part B *
		LevelPartDeinterleave(ipmetrics+m_numcellsA*2, opmetrics+m_numcellsA*2, 
				2*(m_numcells-m_numcellsA), a);
	}

}

void CMLCDecoder::SetCodeRate(tMLCDefinition MLCDef, 
							  int TotalCells)
{
	// * Calculate how many cells for part A *
	int NumLCMReps = MLCDef.NumBitsA/MLCDef.NetBitsPerLCMA;
	if (NumLCMReps * MLCDef.NetBitsPerLCMA <MLCDef.NumBitsA) NumLCMReps++;
	int NumCellsA = NumLCMReps * MLCDef.CodeRateLCMA;

	int startlevel;
	int l;
	m_constellation = MLCDef.Constellation;
	switch(m_constellation)
	{
	case qpsk: startlevel=2; break;
	case qam16: startlevel=1; break;
	case qam64: default: startlevel=0; break;
	}

	for (l=startlevel; l<NUM_LEVELS; l++)
		for (int a=0; a<(MLCDef.Hierarchy==hmmix ? 2 : 1); a++)
		{
			if (MLCDef.Hierarchy!= sm && l==2 && a==0)
				m_viterbi[l][a]->SetCodeRate(MLCDef.CodeRateIndices[l][a][0], 
					MLCDef.CodeRateIndices[l][a][1],0, // No UEP for VSPP
					TotalCells, MLCDef.TailBitMode, MLCDef.Hierarchy);
			else
				m_viterbi[l][a]->SetCodeRate(MLCDef.CodeRateIndices[l][a][0], 
					MLCDef.CodeRateIndices[l][a][1],NumCellsA, 
					TotalCells, MLCDef.TailBitMode, MLCDef.Hierarchy);
		}


	m_numcells = TotalCells;
	m_numcellsA = NumCellsA;
	m_TailbitMode = MLCDef.TailBitMode;
	m_Hierarchy = MLCDef.Hierarchy;

	// * Set up the interleaver generators. '0' means no interleaving *
	for (l=0; l<NUM_LEVELS; l++) 
		m_InterleaverATable[l][0]=0;
	
	if (m_Hierarchy==sm)
	{
		m_InterleaverATable[1][0]=CSB_INT_A;
		m_InterleaverATable[2][0]=MSB_INT_A; 
	}
	else
	{
		m_InterleaverATable[0][0]=CSB_INT_A; // * this is really level 1 *
		m_InterleaverATable[1][0]=MSB_INT_A; // * this is really level 2 *
		m_InterleaverATable[2][0]=0;		 // * this is really level 0 *
	}
	if (m_Hierarchy==hmmix) // * imaginary axis is the same as for sm *
	{
		m_InterleaverATable[0][1]=0;
		m_InterleaverATable[1][1]=CSB_INT_A;
		m_InterleaverATable[2][1]=MSB_INT_A;
	}

}


void CMLCDecoder::CellsToXYCsi(const tFRAME_CELL *ipcells, 
							   tNative *xy, 
							   tNative *csi, 
							   const int numcells, 
							   int axis)
{
	tNative scale=1024; // *** 1.0 * 2^10

	switch(m_constellation)
	{
	case qam64: scale=1024;  // *** 1.0 * 2^10
		break;
	case qam16: scale = 999; // *** 2 * sqrt(10)/sqrt(42) * 2^10
		break;
	case qpsk: scale = 894;  // *** 4 * sqrt(2)/sqrt(42) * 2^10
		break;
	}
	
	if (m_Hierarchy==hmmix && axis==0)
	{
		for (int i=0; i<numcells; i++)
		{
			xy[i] = (tNative)((scale*ipcells[i].re) >> 10);
			//xy[i] = SCALE(scale, ipcells[i].re);
			csi[i] = ipcells[i].csi;
		}
	}
	else if (m_Hierarchy==hmmix && axis==1)
	{
		for (int i=0; i<numcells; i++)
		{
			xy[i] = (tNative)((scale*ipcells[i].im) >> 10);
			//xy[i] = SCALE(scale, ipcells[i].im);
			csi[i] = ipcells[i].csi;
		}
	}
	else
	{
		for (int i=0; i<numcells; i++)
		{
			xy[2*i] = (tNative)((scale*ipcells[i].re) >> 10);
			xy[2*i+1] = (tNative)((scale*ipcells[i].im) >> 10);
			//xy[2*i] = SCALE(scale, ipcells[i].re);
			//xy[2*i+1] = SCALE(scale, ipcells[i].im);
			csi[i] = ipcells[i].csi;
		}
	}
}

void CMLCDecoder::UnDeal(tVitNative *inbits, 
						 tVitNative *outbits, 
						 int numcells)
{
	// * Hierarchical version *
	int startlevel, part, level, axis;

	int LSBStartIndex=0,CSBStartIndex=0, MSBStartIndex=0;

	int StartIndex[NUM_LEVELS][NUM_AXES][2];

	switch(m_constellation)
	{
	case qpsk: startlevel=2; break;
	case qam16: startlevel=1; break;
	case qam64: default: startlevel=0; break;
	}

	int index=0;
	for (axis=0; axis<(m_Hierarchy==hmmix ? 2: 1); axis++)
	{
		for (level=startlevel; level<NUM_LEVELS; level++)
		{
			for (part=0; part<2; part++)
			{
				StartIndex[level][axis][part] = index;
				index += m_viterbi[level][axis]->NumNetBitsPart(part);
			}
		}
	}

	int InIndex=0, OutIndex=0;

	// * Extract the VSPP first; this is always level 2 axis 0 *
	if (m_Hierarchy != sm)
	{
		int LevelStartIndex = StartIndex[2][0][0];
		int numbits = m_viterbi[2][0]->NumNetBits();
		
		for (InIndex=0; InIndex<numbits; InIndex++)
			outbits[OutIndex++] = inbits[LevelStartIndex+InIndex];
	}

	for (part=0; part<2; part++)                        
	{
		for (level=startlevel; level<NUM_LEVELS; level++)
		{
			for (axis=(m_Hierarchy==hmmix ? 1: 0); axis>=0; axis--)		// * Do imag first in hmmix
			{
				int LevelStartIndex, numbits;
				LevelStartIndex = StartIndex[level][axis][part];
				numbits = m_viterbi[level][axis]->NumNetBitsPart(part);

				if (m_Hierarchy!=sm && level==2 && axis==0) numbits=0;	// * VSPP already extracted: skip
				for (InIndex=0; InIndex<numbits; InIndex++)
					outbits[OutIndex++] = inbits[LevelStartIndex+InIndex];
			}
		}
	}
}






