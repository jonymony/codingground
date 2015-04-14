#if !defined(CLASS_CHALAYER_981218_1011_INCLUDED_)
#define CLASS_CHALAYER_981218_1011_INCLUDED_

/***********************************************************************
 *
 * $Project:			DRM Receiver								$
 *
 * (c) 2003 British Broadcasting Corporation, Research & Development
 *     All Rights Reserved
 *
 * $Filename:			HALayer.h									$
 *
 * $Initial Date:		12.02.99									$
 *
 * $Initial Author:		John Elliott								$
 *
 * $Restrictions:		none										$
 *
 * $Compiler:			TriMedia TMCC								$
 *
 * $System Libraries:	none										$
 *
 * $Content:			none										$
 *
 * $Status:															$
 *
 **********************************************************************/
 
//#include "AMDemod.h"
#include "DrmProc.h"
#include "../common/MSCMLC.h"
#include "../common/Demultiplex.h"
#include "DiversityCombiner.h"

#include "../common_fxp/AMDemod.h"
#include "../common_amds/AMSSTMDecoder.h"


#include "../pc_types.h"
//#include <fstream.h>

#define SOFTWARE_VERSION_NUMBER		"0.01P"

class CHALayer
{

public:
	CHALayer();
	~CHALayer();


	void SendMsg(int channel, int type, int length, UInt8 *address, int *respType, int *respLen,
						 UInt8 **respAdd);

	TM_BOOL ProcessBufferAM(tUNativeAcc sample_rate_dev, 
								  tNative* pChan0, MonitorType* pMonit_A_1, Int16* pOutMemAudio,
									TM_BOOL& monit_ready, int& audio_ready,
									TM_BOOL& amss, TM_BOOL& amss_sdc_pass);


	TM_BOOL ProcessBufferDRM(
		unsigned char* stream0, 
		unsigned char* stream1,
		unsigned char* stream2,
		unsigned char* stream3,
		char* text0,
		char* text1,
		char* text2,
		char* text3, 
		tNative* pChan0In,
		tNative* pChan1In,
		tNative* pMonit_A_1,
		tNative* pMonit_A_2,
		tNative* pMonit_B_1,
		tNative* pMonit_B_2,
		TM_BOOL& monit_readyA,
		TM_BOOL& monit_readyB,
		TM_BOOL Band_scan,
		TM_BOOL& drm_found,
		TM_BOOL& frame_ready,
		tFACStruct* FACStruct,
		tSDCStruct* SDCStruct,
		unsigned char* change,
		tNative* DRMLevelA,
		tNative* DRMLevelB,
		tUNativeAcc	sample_rate_dev);
//		short* drm_level);

	
	//accessor methods
	AMSSTMDecoder* GetAMSS(void);
	CMSCMLC*	GetMSCMLC(void);
	CDrmProc*	GetDRM(void);
	CDrmProc*	GetDRM2(void);
	CDemultiplex*	GetDemultiplex(void);
	CDiversityCombiner* GetDivComb(void);
	void Reacquire();

private:


	void InitInstance();
	void ExitInstance(); 
	void InitVariables();
	

	void GetChangeReg(int type, UInt8* address, int length,
		int* retType, int* retLen, UInt8** retAdd);
	void SetChangeMask(int type, UInt8 *address, int length,
		int *retType, int *retLen, UInt8 **retAdd);

	char* GetVersion(int type, unsigned char* data, int length,
		int* return_type, int* return_length, UInt8** return_data);




	//member variables
	AMSSTMDecoder m_amss;
	CAMDemod	m_am_demod;
	CDrmProc	m_drm_recA;
	CDemultiplex m_demultiplex;
	CDrmProc	m_drm_recB;
	CMSCMLC		m_mlc_proc;
	
	CDiversityCombiner m_diversity_combiner;

	tTextStruct m_text_struct;

	char* m_version_str;

	unsigned char* m_p_active_stream;

	unsigned char m_rf_levels[5];
	UInt32	m_rx_status;

	UInt8 m_div_prop;


	TM_BOOL m_dual_frontend;
	TM_BOOL m_freq_read;
	TM_BOOL m_scan;
	int m_freq_table_index;
	unsigned long StepFreq();
	unsigned long m_freq1_set;
	unsigned long m_freq1_use;
	UInt8* m_change_flags;
	UInt8* m_change_flagsB;
	UInt8* m_change_mask;
	UInt8* m_masked_flags;

public:
	tFACStruct* m_pFac_structA;
	tSDCStruct* m_pSdc_structA;
	tFACStruct* m_pFac_structB;
	tSDCStruct* m_pSdc_structB;

	int Dummy(void);

private:
	unsigned char* ReadMLCStats(int& length);
	unsigned long* GetFreq(void);
	UInt8* GetMask(void);
	UInt8* GetFlags(void);

	int m_msc_data_indexA;
	int m_msc_data_indexB;
	CDRMConfig* m_pConfigA;
	CDRMConfig* m_pConfigB;

	tFRAME_CELL* m_msc_data_cell_pointers[4];

	static const unsigned long m_freq_table[];

	UInt8* m_msc_bytes;


	void WritePC(UInt16 value);

	




};

#endif // !defined(CLASS_CHALAYER_981218_1011_INCLUDED_)
