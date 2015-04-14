// ModeDetect.h: interface for the CModeDetect class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MODEDETECT_H__C51217C3_1A20_11D6_9800_00C04FA11197__INCLUDED_)
#define AFX_MODEDETECT_H__C51217C3_1A20_11D6_9800_00C04FA11197__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../bbc_types.h"

#include "TimeProc.h"

class CModeDetect  
{
public:
	void Reset();
	void Reacquire();
	CModeDetect();
	virtual ~CModeDetect();

	TM_BOOL DetectMode(tNative* data_in, 
					   //short* data_out, 
					   tNative *monitor, 
					   tNative monit_points,
					   TMode& mode, 
					   CDRMConfig* config,
					   tNative* peak);

private:
	CTimeProc m_time_procA;
	CTimeProc m_time_procB;
	CTimeProc m_time_procC;
	CTimeProc m_time_procD;

	tNative m_mode_conf;

	tNative* m_data_outA;
	tNative* m_data_outB;
	tNative* m_data_outC;
	tNative* m_data_outD;
	TMode m_old_mode;


};

#endif // !defined(AFX_MODEDETECT_H__C51217C3_1A20_11D6_9800_00C04FA11197__INCLUDED_)
