// DiversityCombiner.h: interface for the CDiversityCombiner class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIVERSITYCOMBINER_H__3AC1CF41_B445_11D6_9503_00C04F08C249__INCLUDED_)
#define AFX_DIVERSITYCOMBINER_H__3AC1CF41_B445_11D6_9503_00C04F08C249__INCLUDED_

#include "../configcommands.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../bbc_types.h"
#include "../S_Defs.h"

#include "DRMConfig.h"
#include "PALCellDeinterleaver.h"	


// There are four buffers, define names for the four indices
#define INPUT_A_BUFFER 0
#define INPUT_B_BUFFER 1
#define STORE_BUFFER 2
#define DECODE_BUFFER 3


enum tDiversityControlState 
{
	DivNeither, DivABad, DivBBad, DivA, DivB, DivBoth
};

enum tBufferControlState
{
	BufStoreSpare, BufStoreA, BufStoreB
};


class CDiversityCombiner  
{
public:
	int ReadActiveDemodIndex();
	float ReadDivProportion();
	tDiversityControlState ReadDiversityState();
	void SetDiversityMode(TDiversityMode mode);
	void UpdateDiversityState(TM_BOOL FACReadyA, TM_BOOL FACReadyB, TM_BOOL MLCOnA, TM_BOOL MLCOnB);
	void ProcessCells(int DemodIndex, CDRMConfig *pConfig, tFRAME_CELL *BufferPointerArray[4],int *pDataIndex,int PosteqFrameNum, int PosteqSymbolNum, float Quality,TM_BOOL &CellsReady, TM_BOOL &MLCOn, int &MLCMuxFrame, int &ConfigIndex);
	CDiversityCombiner();
	virtual ~CDiversityCombiner();

private:
	float m_DivProportion;

	TDiversityMode m_DivMode;
	float DiversityCombine(tFRAME_CELL *pDest,  tFRAME_CELL *pSource1, tFRAME_CELL *pSource2, float Quality1, float Quality2, int CellsPerFrame);
	void CopySurplusCells(tFRAME_CELL *BufferPointerArray[], int DestBufferNo, int SourceBufferNo, int CellsPerFrame, int *pDataIndex);
	void SwapCellBuffers(tFRAME_CELL *BufferPointerArray[], int Buffer1No, int Buffer2No);
	float m_StoreQuality;
	int m_StoreFrameNum;
	tBufferControlState m_BufConState;
	tDiversityControlState m_DivConState;
};

#endif // !defined(AFX_DIVERSITYCOMBINER_H__3AC1CF41_B445_11D6_9503_00C04F08C249__INCLUDED_)
