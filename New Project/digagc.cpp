// DigAGC.cpp: implementation of the CDigAGC class.
//
//////////////////////////////////////////////////////////////////////

#include "DigAGC.h"

#include <math.h>
#include <stdio.h>

#include "../S_Defs.h"

#include "../pc_types.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDigAGC::CDigAGC()
{
	m_mean_power = 0.002f;
}

CDigAGC::~CDigAGC()
{

}

float CDigAGC::AGC(float *data, int numComplex, float thetarget,float kAGC)
{
	int i=0;
	
	float mean_power=m_mean_power;

	float gain = FLOAT (FSQRT(thetarget/mean_power) );
	
	for (i=0; i<numComplex*2; i++)
	{
		float x=data[i];
		mean_power += kAGC*(x*x-mean_power); // Recursive averaging of power
		
		//To prevent NaN
		m_test1=(UInt32*)&gain;
		if(*m_test1 == 0x7FC00000 || *m_test1 == 0xFFFFFFFF || *m_test1 == 0x7F800000 || *m_test1 == 0xFF800000 )
			gain=15000.0f;
					
		x *= gain;

		

		data[i] = x;
	}

	m_mean_power = mean_power;

	return(gain);

}
