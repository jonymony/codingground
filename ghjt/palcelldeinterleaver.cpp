// PALCellDeinterleaver.cpp: implementation of the CPALCellDeinterleaver class.
//
//////////////////////////////////////////////////////////////////////

#include "PALCellDeinterleaver.h"

#include "../pc_types.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//FXP nothing to convert.


CPALCellDeinterleaver::CPALCellDeinterleaver()
{

	for (int i=0; i<CELL_INTERLEAVER_DEPTH; i++)
	{
		m_frame_cell_ptr[i] = new tFRAME_CELL[MAX_MSC_PER_LFRAME];

		for(int j=0; j< MAX_MSC_PER_LFRAME; j++)
		{
			m_frame_cell_ptr[i][j].re=0;
			m_frame_cell_ptr[i][j].im=0;
			m_frame_cell_ptr[i][j].csi=0;
		}
	}
}

CPALCellDeinterleaver::~CPALCellDeinterleaver()
{
	for (int i=0; i<CELL_INTERLEAVER_DEPTH; i++)
		delete [] m_frame_cell_ptr[i];
}

void CPALCellDeinterleaver::deinterleave(tFRAME_CELL *data_in, CDRMConfig *config)
{
	int k,i;
	tFRAME_CELL *tmp;
	int n;

	tFRAME_CELL* restrict data=data_in;


	// Rotate frame pointers
	
	tmp = m_frame_cell_ptr[CELL_INTERLEAVER_DEPTH-1];
	
	for (k=CELL_INTERLEAVER_DEPTH-1; k>0; k--)
	{
		m_frame_cell_ptr[k] = m_frame_cell_ptr[k-1];
	}
	m_frame_cell_ptr[0] = tmp;
	
	n = config->data_cell_per_frame();

	// Copy input data into store 0
	tFRAME_CELL *frame0 = m_frame_cell_ptr[0];
	for (i=0; i<n; i++) frame0[i] = data[i];

	// Calculate s and q
	int sr,s,q, Pi,mask;
	int inindex;

	sr = n;
	s=1;
	while (sr)
	{
		sr >>= 1;
		s <<= 1;
	}
	if (s==n<<1) s>>=1; /* special case where n==2^i */

	q = s/4-1;
	mask = s-1;

	/* Now do the deinterleaving */
	
	Pi = 0;      
	inindex = 0;
                     
	if (config->interleaver_depth() == shallow)
		/* Short interleaver : one frame only */
	{   
		tFRAME_CELL *inptr;                                   
		inptr = m_frame_cell_ptr[0];                      
		for (i=0; i<s; i++) 
		{
			if (Pi<n) data[Pi]=inptr[inindex++];
			Pi = ((CELL_INTERLEAVER_R * Pi) + q) & mask;
		}
	}
	else
		/* Long interleaver : CELL_INTERLEAVER_DEPTH frames */
	{
		int gamma = CELL_INTERLEAVER_DEPTH-1;

		for (i=0; i<s; i++) 
		{
			if (Pi<n) 
			{
				data[Pi]=m_frame_cell_ptr[gamma][inindex++];
				gamma--;
				if (gamma<0) gamma = CELL_INTERLEAVER_DEPTH-1;
			}
			Pi = ((CELL_INTERLEAVER_R * Pi) + q) & mask;
		}

	}

}
