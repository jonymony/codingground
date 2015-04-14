// DiversityCombiner.cpp: implementation of the CDiversityCombiner class.
//
//////////////////////////////////////////////////////////////////////

#include "DiversityCombiner.h"
#include <string.h>
#include <stdio.h>
#include "../message_types.h"
#include "../pc_types.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CDiversityCombiner::CDiversityCombiner()
{
	m_BufConState = BufStoreSpare;
	m_DivConState = DivNeither;
	m_StoreFrameNum = 0;
	m_StoreQuality = 0.0f;
	//m_DivMode = divFrameSwitched;
	//m_DivMode = divOffA;
	//m_DivMode = divCellSwitched;
	//m_DivMode = divMaxRatio;
	m_DivMode = divOffB;
	m_DivProportion = 0.0f;
}

CDiversityCombiner::~CDiversityCombiner()
{

}

void CDiversityCombiner::ProcessCells(
		int DemodIndex, CDRMConfig *pConfig, tFRAME_CELL *BufferPointerArray[], 
		int *pDataIndex, int PosteqFrameNum, int PosteqSymbolNum, 
		float Quality, TM_BOOL &CellsReady, TM_BOOL &MLCOn, int &MLCMuxFrame, int &ConfigIndex)
{
//	char DebugString[100];

	// What if cells per frame is rubbish?
	short CellsPerFrame = pConfig->data_cell_per_frame();

	// Determine which channel(s) used as function of diversity state
	short ChannelUsed;
	switch(m_DivConState)
	{
	case DivNeither: ChannelUsed=-1; break; // -1 = no channel
	case DivABad: case DivA: ChannelUsed=0; break; // 0 = A
	case DivBBad: case DivB: ChannelUsed=1; break; // 1 = B
	case DivBoth: ChannelUsed=2; break; // 2 = both
	default: ChannelUsed=-1;
	}
	ConfigIndex = (ChannelUsed==1 ? 1:0); // To decide which config etc. to use

	// MLCOn tells the MSCMLC process whether there will ever be audio - if not
	// it will output the monitoring when the monitoring is ready instead of waiting
	// for the audio
	MLCOn = (ChannelUsed!=-1);

	// are there one frame's worth of cells in the input buffer?
	TM_BOOL InputBufferFull = (*pDataIndex >= CellsPerFrame);

	// Find the multiplex frame number. NB not the same as the tx frame number
	// MF 0 finishes in TF1, MF1 near the start of TF2, MF2 at end of TF2
	short MuxFrame;
	if (PosteqFrameNum==1)
		MuxFrame = 0;
	else if (PosteqFrameNum==2 && PosteqSymbolNum== pConfig->syms_per_frame()-1)
		MuxFrame = 2;
	else 
		MuxFrame = 1;

	short NextFrameNum = (m_StoreFrameNum + 1) % 3;
	// Buffer control state machine
	switch (m_BufConState)
	{
	case BufStoreSpare: // the store buffer is empty
		if (InputBufferFull) // make sure we do something with it!
		{
			if (ChannelUsed==-1) // No channel is being decoded
			{

				// Discard the input
				CopySurplusCells(BufferPointerArray, DemodIndex, DemodIndex, CellsPerFrame, pDataIndex);
			}
			else if (ChannelUsed==DemodIndex) // only one channel used, its input buffer is ready
			{

				// Move input cells into the decode buffer
				SwapCellBuffers(BufferPointerArray, DemodIndex, DECODE_BUFFER);
				// Tell the MLC process to decode it
				CellsReady = true; ConfigIndex = DemodIndex; MLCMuxFrame = MuxFrame;
				m_DivProportion = (DemodIndex==0 ? 1.0f : 0.0f);
				// Copy the surplus cells back into the input buffer (they belong to the next mux frame)
				CopySurplusCells(BufferPointerArray, DemodIndex, DECODE_BUFFER, CellsPerFrame, pDataIndex);
			}
			else if (ChannelUsed==2) // using both channels
			{

				
				// Move input cells into the store buffer
				SwapCellBuffers(BufferPointerArray, DemodIndex, STORE_BUFFER);
				m_StoreFrameNum = MuxFrame;
				m_StoreQuality = Quality;
				// change state to remember that store buffer contains this channel
				m_BufConState = (DemodIndex==0 ? BufStoreA : BufStoreB);
				// Copy the surplus cells back into the input buffer (they belong to the next mux frame)
				CopySurplusCells(BufferPointerArray, DemodIndex, STORE_BUFFER, CellsPerFrame, pDataIndex);
			}
			else // Must be using the other channel
			{


				// Discard the input
				CopySurplusCells(BufferPointerArray, DemodIndex, DemodIndex, CellsPerFrame, pDataIndex);
			}
		}

		break;

		// The following states are mirror images of each other. Deal with both together
		// to try to avoid mistakes
	case BufStoreA: case BufStoreB:
		int StoreChannel = (m_BufConState == BufStoreA ? 0 : 1);
		if (ChannelUsed != 2) // not using both channels (maybe neither)
		{
			if (ChannelUsed == StoreChannel)
			{
				// will decode the stored buffer
				SwapCellBuffers(BufferPointerArray, STORE_BUFFER, DECODE_BUFFER);
				CellsReady = true; ConfigIndex = ChannelUsed; MLCMuxFrame = m_StoreFrameNum;
				m_DivProportion = (StoreChannel==0 ? 1.0f : 0.0f);

				m_BufConState = BufStoreSpare;

				if (InputBufferFull && ChannelUsed==DemodIndex)
				{

					// Move input cells into the store buffer
					SwapCellBuffers(BufferPointerArray, DemodIndex, STORE_BUFFER);
					m_StoreFrameNum = MuxFrame;
					m_StoreQuality = Quality;
					// change state to remember that store buffer contains this channel
					// Next time it will decode the store and move to state BufStoreSpare
					m_BufConState = (DemodIndex==0 ? BufStoreA : BufStoreB);
					// Copy the surplus cells back into the input buffer (they belong to the next mux frame)
					CopySurplusCells(BufferPointerArray, DemodIndex, STORE_BUFFER, CellsPerFrame, pDataIndex);
				}
				else if (InputBufferFull)
				{

					// Discard the input
					CopySurplusCells(BufferPointerArray, DemodIndex, DemodIndex, CellsPerFrame, pDataIndex);
				}
			}
			else // not using the store channel
			{
				// Discard the store

				m_BufConState = BufStoreSpare;

				if (InputBufferFull && ChannelUsed==DemodIndex)
				{


					// Move input cells into the decode buffer
					SwapCellBuffers(BufferPointerArray, DemodIndex, DECODE_BUFFER);
					// Tell the MLC process to decode it
					CellsReady = true; ConfigIndex = ChannelUsed; MLCMuxFrame = MuxFrame;
					m_DivProportion = (DemodIndex==0 ? 1.0f : 0.0f);
					// Copy the surplus cells back into the input buffer (they belong to the next mux frame)
					CopySurplusCells(BufferPointerArray, DemodIndex, DECODE_BUFFER, CellsPerFrame, pDataIndex);
				}
				else if (InputBufferFull) // input full but not using this channel
				{


					// Discard the input
					CopySurplusCells(BufferPointerArray, DemodIndex, DemodIndex, CellsPerFrame, pDataIndex);
				}
			}
		}
		else if (InputBufferFull) // and using both channels
		{
			if (DemodIndex == StoreChannel) // another frame from the stored channel
			{


				// will decode the stored buffer
				SwapCellBuffers(BufferPointerArray, STORE_BUFFER, DECODE_BUFFER);
				CellsReady = true; ConfigIndex = DemodIndex; MLCMuxFrame = m_StoreFrameNum;
				m_DivProportion = (StoreChannel==0 ? 1.0f : 0.0f);
				
				// And store the current input
				SwapCellBuffers(BufferPointerArray, DemodIndex, STORE_BUFFER);
				m_StoreFrameNum = MuxFrame;
				m_StoreQuality = Quality;
				// stay in this state
				// Copy the surplus cells back into the input buffer (they belong to the next mux frame)
				CopySurplusCells(BufferPointerArray, DemodIndex, STORE_BUFFER, CellsPerFrame, pDataIndex);
			}
			else if (MuxFrame==m_StoreFrameNum)// a frame from the other channel, expected frame number
			{


				// Combine the input frame and the store frame and decode
				float InputProportion; // proportion used from input channel
				InputProportion = DiversityCombine(BufferPointerArray[DECODE_BUFFER],
					BufferPointerArray[DemodIndex], BufferPointerArray[STORE_BUFFER], 
					Quality, m_StoreQuality, CellsPerFrame);

				CellsReady = true; ConfigIndex = DemodIndex; MLCMuxFrame = m_StoreFrameNum;
				m_DivProportion = (DemodIndex==0 ? InputProportion : 1.0f-InputProportion);
				// store is now spare
				m_BufConState = BufStoreSpare;
				// copy surplus back to input buffer
				CopySurplusCells(BufferPointerArray, DemodIndex, DemodIndex, CellsPerFrame, pDataIndex);
			}
			else if (MuxFrame == NextFrameNum) // a frame from the other channel, but the next frame
			{

				// will decode the stored buffer
				SwapCellBuffers(BufferPointerArray, STORE_BUFFER, DECODE_BUFFER);
				CellsReady = true; ConfigIndex = StoreChannel;MLCMuxFrame = m_StoreFrameNum;
				m_DivProportion = (StoreChannel==0 ? 1.0f : 0.0f);
				
				// And store the current input
				SwapCellBuffers(BufferPointerArray, DemodIndex, STORE_BUFFER);
				m_StoreFrameNum = MuxFrame;
				m_StoreQuality = Quality;
				// Store now contains the other channel - change state
				m_BufConState = (DemodIndex==0 ? BufStoreA : BufStoreB);
				// Copy the surplus cells back into the input buffer (they belong to the next mux frame)
				CopySurplusCells(BufferPointerArray, DemodIndex, STORE_BUFFER, CellsPerFrame, pDataIndex);
			}
			else // a frame from the other channel, but out of sequence - something wrong - just discard it
			{

				CopySurplusCells(BufferPointerArray, DemodIndex, DemodIndex, CellsPerFrame, pDataIndex);
				// stay in this state
			}

		}
		else // input buffer not full. Nothing to do
		{
		}
	}

}

void CDiversityCombiner::UpdateDiversityState(TM_BOOL FACReadyA, TM_BOOL FACReadyB, TM_BOOL MLCOnA, TM_BOOL MLCOnB)
{
	if (m_DivMode==divOffA)
		m_DivConState = (MLCOnA? DivA: DivNeither);
	else if (m_DivMode==divOffB)
		m_DivConState = (MLCOnB? DivB: DivNeither);
	else 
	{
		switch (m_DivConState)
		{
		case DivNeither:
			if (MLCOnA)
				m_DivConState = DivA;
			else if (MLCOnB)
				m_DivConState = DivB;
			break;
		case DivABad:
			if (!MLCOnB)
				m_DivConState = DivNeither;
			else if (FACReadyA)
				m_DivConState = DivA;
			else if (FACReadyB)
				m_DivConState = DivB;
			break;
		case DivBBad:
			if (!MLCOnA)
				m_DivConState = DivNeither;
			else if (FACReadyA)
				m_DivConState = DivA;
			else if (FACReadyB)
				m_DivConState = DivB;
			break;
		case DivA:
			if (!MLCOnA && !MLCOnB)
				m_DivConState = DivNeither;
			else if (!MLCOnA)
				m_DivConState = DivB;
			else if (!FACReadyA)
				m_DivConState = DivABad;
			else if (FACReadyB)
				m_DivConState = DivBoth;
			break;
		case DivB:
			if (!MLCOnA && !MLCOnB)
				m_DivConState = DivNeither;
			else if (!MLCOnB)
				m_DivConState = DivA;
			else if (!FACReadyB)
				m_DivConState = DivBBad;
			else if (FACReadyA)
				m_DivConState = DivBoth;
			break;
		case DivBoth:
			if (!MLCOnA && !MLCOnB)
				m_DivConState = DivNeither;
			else if (!FACReadyA && !FACReadyB)
				m_DivConState = DivABad;
			else if (!MLCOnB || !FACReadyB)
				m_DivConState = DivA;
			else if (!MLCOnA || !FACReadyA)
				m_DivConState = DivB;
		}
	}

}

void CDiversityCombiner::SwapCellBuffers(tFRAME_CELL *BufferPointerArray[], 
										 int Buffer1No, int Buffer2No)
{
	//Swap the pointers in the BufferPointerArray at indexes Buffer1No and Buffer2No
	tFRAME_CELL * temp;
	temp = BufferPointerArray[Buffer1No];
	BufferPointerArray[Buffer1No] = BufferPointerArray[Buffer2No];
	BufferPointerArray[Buffer2No] = temp;
}

void CDiversityCombiner::CopySurplusCells(tFRAME_CELL *BufferPointerArray[], 
										  int DestBufferNo, int SourceBufferNo, int CellsPerFrame, 
										  int *pDataIndex)
{
	// If there are some extra cells that belong to the next frame, copy them to the beginning of
	// SourceBufferNo
	memcpy(BufferPointerArray[DestBufferNo], BufferPointerArray[SourceBufferNo]+CellsPerFrame, (*pDataIndex-CellsPerFrame)*sizeof(tFRAME_CELL));

	// Update the data index to point to the next write position in the dest buffer
	(*pDataIndex) -= CellsPerFrame;
}

float CDiversityCombiner::DiversityCombine(tFRAME_CELL *pDest, 
										  tFRAME_CELL *pSource1, tFRAME_CELL *pSource2, 
										  float Quality1, float Quality2, int CellsPerFrame)
{
	char DebugString[256];
	float Source1Proportion;

	
	// Combine the input frames into the output frame according to the selected method.

	// Choose cell by cell the channel with the better CSI
	if (m_DivMode == divCellSwitched)
	{
		int i;
		int Source1Count = 0;
		for (i=0; i<CellsPerFrame; i++)
		{
			if (pSource1[i].csi > pSource2[i].csi)
			{
				pDest[i] = pSource1[i];
				Source1Count++;
			}
			else
				pDest[i] = pSource2[i];
		}
		Source1Proportion = (float)Source1Count / (float)CellsPerFrame;
	}
	else if (m_DivMode == divMaxRatio)
	{
		int i;
		float csi1, csi2, csi, csi_recip;
		float Source1TotalWeighting=0.0f;
		for (i=0; i<CellsPerFrame; i++)
		{
			csi1 = pSource1[i].csi;
			csi2 = pSource2[i].csi;
			csi = csi1+csi2;
			csi_recip = 1.0f/csi;

//#ifdef DEMAP_FLP
//			pDest[i].re = (pSource1[i].re * csi1 + pSource2[i].re * csi2) * csi_recip;
//			pDest[i].im = (pSource1[i].im * csi1 + pSource2[i].im * csi2) * csi_recip;

//#else
//			pDest[i].re = ((pSource1[i].re * csi1 + pSource2[i].re * csi2)/(1<<10) * csi_recip);///(1<<10);//AEGN
//			pDest[i].im = ((pSource1[i].im * csi1 + pSource2[i].im * csi2)/(1<<10) * csi_recip);///(1<<10);
			pDest[i].re = ((pSource1[i].re * csi1 + pSource2[i].re * csi2) * csi_recip);///(1<<10);//AEGN
			pDest[i].im = ((pSource1[i].im * csi1 + pSource2[i].im * csi2) * csi_recip);///(1<<10);
//#endif
			pDest[i].csi = csi;
			Source1TotalWeighting += csi1 * csi_recip;
		}



		Source1Proportion = Source1TotalWeighting / (float)CellsPerFrame;

	}

	// Use the whole frame from the channel with higher quality.
	else //if (m_DivMode == divFrameSwitched)
	{
		sprintf(DebugString, "Combining: QualityA=%f\tQualityB=%f", Quality1, Quality2);


		int i;
		tFRAME_CELL *pSource;
		if (Quality1>Quality2)
		{
			pSource = pSource1;
			Source1Proportion = 1.0f;
		}
		else
		{
			pSource = pSource2;
			Source1Proportion = 0.0f;
		}
		for (i=0; i<CellsPerFrame; i++)
		{
			pDest[i] = pSource[i];
		}
	}
	return Source1Proportion;
}



void CDiversityCombiner::SetDiversityMode(TDiversityMode mode)
{
	m_DivMode = mode;
}

tDiversityControlState CDiversityCombiner::ReadDiversityState()
{
	return m_DivConState;
}

float CDiversityCombiner::ReadDivProportion()
{
	if (m_DivConState == DivNeither)
		return -1.0f;
	else
		return m_DivProportion;
}

int CDiversityCombiner::ReadActiveDemodIndex()
{
	return (m_DivConState==DivBBad || m_DivConState==DivB ? 1 : 0);
}
