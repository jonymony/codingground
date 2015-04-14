// PALCellDeinterleaver.h: interface for the CPALCellDeinterleaver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PALCELLDEINTERLEAVER_H__29FE1480_EBAE_11D4_8CF4_00C04FA11AF6__INCLUDED_)
#define AFX_PALCELLDEINTERLEAVER_H__29FE1480_EBAE_11D4_8CF4_00C04FA11AF6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DRMConfig.h"
#include "../common/DeMap.h"

#define CELL_INTERLEAVER_DEPTH 5
#define CELL_INTERLEAVER_R 5

class CPALCellDeinterleaver  
{
public:
	void deinterleave(tFRAME_CELL *data, CDRMConfig *config);
	CPALCellDeinterleaver();
	virtual ~CPALCellDeinterleaver();

private:
	tFRAME_CELL * m_frame_cell_ptr[CELL_INTERLEAVER_DEPTH];
};

#endif // !defined(AFX_PALCELLDEINTERLEAVER_H__29FE1480_EBAE_11D4_8CF4_00C04FA11AF6__INCLUDED_)
