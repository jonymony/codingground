// MLCDecoder.h: interface for the CMLCDecoder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MLCDECODER_H__52D0F191_5D4B_11D4_9377_00C04F08C249__INCLUDED_)
#define AFX_MLCDECODER_H__52D0F191_5D4B_11D4_9377_00C04F08C249__INCLUDED_

#include "../configcommands.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "../bbc_types.h"
#include "../S_Defs.h"
#include "../common/demap.h"
#include "../common_fxp/ViterbiDecoder.h"	// Added by ClassView
#include "../common/PRBSCheck.h"	// Added by ClassView
#include "../structurescommon.h"

#include "../ConfigCommands.h"

#define NUM_LEVELS 3
#define NUM_AXES 2

#define CSB_INT_A 13
#define CSB_INT_P 8192

#define MSB_INT_A 21 // was 17
#define MSB_INT_P 8192



class CMLCDecoder
{
public:

	void UnDeal(tVitNative *inbits, tVitNative *outbits, int numcells);
	void CellsToXYCsi(const tFRAME_CELL *ipcells, tNative *xy, tNative *csi, const int numcells, int axis);
	void SetCodeRate(tMLCDefinition MLCDef, int TotalCells);
	void LevelDeinterleave(tNative *ipmetrics, tNative *opmetrics, int a, int level, int axis);
	void LevelInterleave(tVitNative *ipbits, tVitNative *opbits, int a, int level, int axis);
	void LevelPartDeinterleave(tNative *ipmetrics, tNative *opmetrics, int numbits, int a);
	void LevelPartInterleave(tVitNative *ipbits, tVitNative *opbits, int numbits, int a);
	void BER(tVitNative *data, tMLCErrorStruct errstruct[2], int numbytes, int numbytesVSPP, TM_BOOL reset);
	void Decode(tFRAME_CELL *ipcells, tVitNative *opdata, int &numnetbits, int &numnetbitsVSPP);
	CMLCDecoder();
	virtual ~CMLCDecoder();

private:

	int (*m_InterleaverATable)[NUM_AXES];
	THierarchy m_Hierarchy;
	int m_TailbitMode;
	int m_numcellsA;
	int m_numcells;
	TConstellation m_constellation;
	CPRBSCheck m_prbs_check;
	CViterbiDecoder *m_viterbi[NUM_LEVELS][NUM_AXES];
	
	tNative* m_xy;
	tNative* m_metrics1;
	tNative* m_metrics2;
	tNative* m_csi;
	
	tVitNative* m_recodedbits;
	tVitNative* m_ReintBitsLSB;
	tVitNative* m_ReintBitsCSB;
	tVitNative* m_ReintBitsMSB;
};



#endif // !defined(AFX_MLCDECODER_H__52D0F191_5D4B_11D4_9377_00C04F08C249__INCLUDED_)
