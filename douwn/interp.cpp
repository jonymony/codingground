// Interp.cpp: implementation of the CInterp class.
//
//////////////////////////////////////////////////////////////////////

#include "Interp.h"

#include <stdio.h>
#include <string.h>
#include <math.h>


#include "../pc_types.h"

    
#define INTERP_LEN 8

//#define PULL_DOWN 24 //49490

// *** Multiply Macro ***
#define TAUM(Z,F) NATIVE_CAST (((Z) * (F>>(23-bits)))>>bits)


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//***************************************************************************
//*			*** Constructor ***     Fixed Point Version 
//***************************************************************************

CInterp::CInterp()
{

	m_del_interp=new tNative[INTERP_LEN];
	
	if(!m_del_interp) 
	{
		DP(("failed to allocate del_interp\n"));
	}

	m_frame_store=new tNative[2][2*TOTAL_SYMBOL_MALLOC];

	tNative i;
	
	for(i=0;i<INTERP_LEN;i++) 
		m_del_interp[i]=0;
	
	m_state=0;
	m_tau=0;
	m_out_sel=0;
	m_out_ptr=0;
	m_jump_flag=FALSE;

}

CInterp::~CInterp()
{
	delete [] m_del_interp;
	delete [] m_frame_store;


}

//***************************************************************************
//*
//*			Function name : *** interp ***
//*
//*			Description : 
//*
//*			Returns :
//*
//*			Input :
//*
//*			Output :
//*
//*			Tree : CDrmProc::main_process
//*							\->Class::function
//*										\->Class::function
//***************************************************************************

//***************************************************************************
//*			*** interp ***     Fixed Point Version 
//***************************************************************************

void CInterp::interp(tNative    *data_in, 
					 tNativeAcc      freq,     //fxp base 23
					 tNative    jump, 
					 TM_BOOL  &symbol_valid, 
					 tNative    total_symbol)
{
	tNative bits=12;

	tNative i;
  
	tNative total_symbol_complex=2*total_symbol;

	tNative zr,zi;

	tNative* restrict data=data_in;
	tNative* del_interp=m_del_interp;

	tNative del0r, del1r, del2r, del3r;
	tNative del0i, del1i, del2i, del3i;

	int tau; // * 32 bit * 

	symbol_valid=FALSE;

	m_out_ptr -= 2*jump;
	
	if(m_out_ptr<0) 
		m_out_ptr += 2*total_symbol;
  
	if(jump!=0) 
		m_jump_flag=TRUE;

	// *** Get the delay line contents back ***

	del0r = del_interp[0];
	del0i = del_interp[1];
	del1r = del_interp[2];
	del1i = del_interp[3];
	del2r = del_interp[4];
	del2i = del_interp[5];
	del3r = del_interp[6];
	del3i = del_interp[7];
	tau = m_tau;

	for(i=0 ; i<DRM_BUFFER_POINTS_U; i+=2)
	{

		// *** Clock the delay line in any case *** 
		del0r=del1r; del1r=del2r; del2r=del3r; del3r=data[i];
		del0i=del1i; del1i=del2i; del2i=del3i; del3i=data[i+1];

		switch (m_state)
		{	
			// *** Calculate output value ***
			case 0:      
	  			// * First stage *
/*				zr = -del0r + 3*del1r - 3*del2r + del3r;
				zr = TAUM(zr , tau)  ;

				zi = -del0i + 3*del1i - 3*del2i + del3i;
				zi = TAUM(zi , tau ) ;
  
				// * Second stage *
				zr += 3*del0r - 6*del1r + 3*del2r;
				zr = TAUM(zr , tau)  ;

	  
				zi += 3*del0i - 6*del1i + 3*del2i;
				zi = TAUM(zi , tau)  ;
		  
				// * Third stage *
				zr += -3*del0r + 3*del2r;
				zr = TAUM(zr , tau) ;

		
				zi += -3*del0i + 3*del2i;
				zi = TAUM(zi , tau) ;

				// * Fourth stage *
				zr += del0r + 4*del1r + del2r;
				zi += del0i + 4*del1i + del2i;*/

				// * First stage *
				zr = (-341 * del0r + 1024*del1r - 1024*del2r + 341*del3r)>>11;
				zr = TAUM(zr , tau)  ;

				zi = (-341*del0i + 1024*del1i - 1024*del2i + 341*del3i)>>11;
				zi = TAUM(zi , tau ) ;
  
				// * Second stage *
				zr += (((1024*del0r  + 1024*del2r)>>11)- del1r);
				zr = TAUM(zr , tau)  ;

	  
				zi += (((1024*del0i  + 1024*del2i)>>11)- del1i);
				zi = TAUM(zi , tau)  ;
		  
				// * Third stage *
				zr += ((-1024*del0r + 1024*del2r)>>11);
				zr = TAUM(zr , tau) ;

		
				zi += ((-1024*del0i + 1024*del2i)>>11);
				zi = TAUM(zi , tau) ;

				// * Fourth stage *
				zr += ((341*del0r + 1365*del1r + 341*del2r)>>11);
				zi += ((341*del0i + 1365*del1i + 341*del2i)>>11);
			

				// * Store the filter output *

				m_frame_store[m_out_sel][m_out_ptr]=  zr;
				m_frame_store[m_out_sel][m_out_ptr+1]= zi;
				m_out_ptr+=2;

				if (m_out_ptr >= 2*total_symbol)
				{
					m_out_ptr=0;
					m_out_sel = 1 - m_out_sel;
					
					if(m_jump_flag)
						m_jump_flag=FALSE;
					else
						symbol_valid = TRUE;
				}
				
    			// OPH: allow pulls either way
				tau += (1<<23)+freq;

				if (tau >= (1<<23))
					m_state = 1;
			
				break;
			
			// *** Update accumulator ***
			case 1:     	
				tau -= (1<<23);
				
				if (tau < (1<<23))
					m_state = 0;  		
		
			break;
			
			// *** Wait state - drop a cycle ***

			
			default:
				break;
		}     
	}
  
	// *** Put the delay line contents back ***
	del_interp[0] = del0r;
	del_interp[1] = del0i; 
	del_interp[2] = del1r; 
	del_interp[3] = del1i; 
	del_interp[4] = del2r;
	del_interp[5] = del2i;
	del_interp[6] = del3r;
	del_interp[7] = del3i;

	m_tau = tau;

	if (symbol_valid)
		for (i=0; i<2*total_symbol; i++)
			data[i] =NATIVE_CAST((m_frame_store[1-m_out_sel][i]));     
}


