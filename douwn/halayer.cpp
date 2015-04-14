/***********************************************************************
 *
 * $Project:			DRM Receiver								$
 *
 * (c) 2003 British Broadcasting Corporation, Research & Development
 *     All Rights Reserved
 *
 * $Filename:			HALayer.cpp									$
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
 **********************************************************************/
 


#include "../Common_fxp/HALayer.h"
#include "../message_types.h"
#include "../RxStatus.h"


const unsigned long CHALayer::m_freq_table[]={
	10600000, 10000, 10800000, 0, 0
};


CHALayer::CHALayer()
{
	DP(("Creating HAL fixed point version\n"));
	InitInstance();


}

CHALayer::~CHALayer()
{
	
	ExitInstance();


	DP(("deleted HAL\n"));

}

void CHALayer::InitInstance()
{


	tNativeAcc i;

	m_pConfigA=NULL;
	m_pConfigB=NULL;
	m_pSdc_structA=new tSDCStruct;
	m_pFac_structA=new tFACStruct;
	m_pSdc_structB=new tSDCStruct;
	m_pFac_structB=new tFACStruct;
	m_msc_bytes = new UInt8[MAX_LEN_PART_A];

//	m_msc_data_cells1=new tFRAME_CELL[MAX_MSC_PER_LFRAME+ 2*MAX_MSC_PER_SYMBOL];
//	m_msc_data_cells2=new tFRAME_CELL[MAX_MSC_PER_LFRAME+ 2*MAX_MSC_PER_SYMBOL];
	memset(m_pSdc_structA, 0, sizeof(tSDCStruct) );
	memset(m_pFac_structA, 0, sizeof(tFACStruct) );
	memset(m_pSdc_structB, 0, sizeof(tSDCStruct) );
	memset(m_pFac_structB, 0, sizeof(tFACStruct) );


	for (tNative j=0; j<4; j++)
	{
		m_msc_data_cell_pointers[j] = new tFRAME_CELL[MAX_MSC_PER_LFRAME+ 2*MAX_MSC_PER_SYMBOL];

		for(i=0; i< (MAX_MSC_PER_LFRAME+ 2*MAX_MSC_PER_SYMBOL ); i++)
		{
			m_msc_data_cell_pointers[j][i].re=0;
			m_msc_data_cell_pointers[j][i].im=0;
			m_msc_data_cell_pointers[j][i].csi=0;
		}
	}


	m_drm_recA.SetID(1);
	m_drm_recB.SetID(2);

	InitVariables();
}

void CHALayer::InitVariables()
{
	tNative i;
	
	m_msc_data_indexA=0;
	m_msc_data_indexB=0;
	
	m_change_flags = new UInt8[CHANGE_FLAGS];
	m_change_flagsB = new UInt8[CHANGE_FLAGS];
	m_change_mask = new UInt8[CHANGE_FLAGS];
	m_masked_flags = new UInt8[CHANGE_FLAGS];

	m_version_str = new char[VERSION_STR];

	strcpy(m_version_str, SOFTWARE_VERSION_NUMBER);
	
	for(i=0; i< CHANGE_FLAGS; i++)
	{
		m_change_mask[i]= 0xFF;
		m_change_flags[i]= 0x00;
		m_masked_flags[i] = 0x00;
	}
	
    
    m_p_active_stream = NULL;
	
	
	m_freq1_set = m_freq_table[0];
	m_freq1_use = m_freq_table[0];
	m_freq_table_index=0;
	m_scan=FALSE;
	m_freq_read=TRUE;

	m_div_prop = 0xFF;

	
	m_dual_frontend = FALSE;
	
//	m_bDecoderStatus=FALSE;

}


void CHALayer::ExitInstance()
{
	delete [] m_pFac_structA;
	delete [] m_pSdc_structA;
	delete [] m_pFac_structB;
	delete [] m_pSdc_structB;

	for (tNativeAcc j=0; j<4; j++)
		delete [] m_msc_data_cell_pointers[j];


	delete [] m_change_flags;
	delete [] m_change_flagsB;
	delete [] m_change_mask;
	delete [] m_masked_flags;

	delete [] m_version_str;

	delete [] m_msc_bytes;





}

AMSSTMDecoder* CHALayer::GetAMSS(void)
{
	return &m_amss;
}

CDrmProc* CHALayer::GetDRM(void)
{
	return (&m_drm_recA);
}

CDrmProc* CHALayer::GetDRM2(void)
{
//	return (&m_drm_recA);
	return (&m_drm_recB);
//	return NULL;
}

CMSCMLC* CHALayer::GetMSCMLC(void)
{
//	return (NULL);
	return (&m_mlc_proc);
}

CDemultiplex* CHALayer::GetDemultiplex(void)
{
//	return (NULL);
	return (&m_demultiplex);
}
  


CDiversityCombiner* CHALayer::GetDivComb(void)
{
//	return (NULL);
	return(&m_diversity_combiner);
}

TM_BOOL CHALayer::ProcessBufferAM(tUNativeAcc sample_rate_dev, 
								  tNative* pChan0, MonitorType* pMonit_A_1, Int16* pOutMemAudio,
									TM_BOOL& monit_ready, int& audio_ready,
									TM_BOOL& amss, TM_BOOL& amss_sdc_pass)
{
	tNativeAcc i;
	TM_BOOL* sdc_bits = NULL;
	unsigned long sdc_length = 0;
	//tSDCStruct SDCStruct;
	float* pMon = new float[CAPTURE_BUFFER_POINTS_U];//need to change this when we convert to fixed point

	float* pLocal = new float[CAPTURE_BUFFER_POINTS_U];//need to change this when we convert to fixed point

	for(i=0; i< CAPTURE_BUFFER_POINTS_U; i++)
	{
		pLocal[i] = pChan0[i];
	//	pMon[i] = pChan0[i];
	}

// put in when converted to fixed point
	m_am_demod.ProcessBlock(sample_rate_dev, pChan0, pMon, pOutMemAudio, NULL,
		NULL, monit_ready, audio_ready);

	
	

//	m_amss.ProcessBufferAMSS(pLocal,  amss, amss_sdc_pass, &sdc_bits, sdc_length,
//				pMon);

	for(i = 0; i< CAPTURE_BUFFER_POINTS_U; i++)
		pMonit_A_1[i] = pMon[i];

	delete [] pLocal;
	delete [] pMon;

	return TRUE;
}

TM_BOOL CHALayer::ProcessBufferDRM(unsigned char* stream0,
								   unsigned char* stream1,
								   unsigned char* stream2,
								   unsigned char* stream3,
								   char* text0,
								   char* text1,
								   char* text2,
								   char* text3,
								   tNative* pChan0, 
								   tNative* pChan1, 
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
									tUNativeAcc	sample_rate_dev)
								  // short* drm_level)


{

	
	TM_BOOL valid_drm_blockA=FALSE;
	TM_BOOL valid_drm_blockB=FALSE;

	TM_BOOL block_monit_readyA=FALSE, block_monit_readyB=FALSE;

	TM_BOOL monit_ready = FALSE;

	tNative posteq_framenumA=0;
	tNative posteq_symbolnumA=0;
	tNative posteq_framenumB=0;
	tNative posteq_symbolnumB=0;
	tNativeAcc mux_framenum = 0;



	TM_BOOL drm_foundB=false; // dummy - demod B not used for band scan

//try putting these here to make sure the pointers are intialised
	m_pConfigA=m_drm_recA.GetConfig();
	m_pConfigB=m_drm_recB.GetConfig();

	WritePC(40510);

	TM_BOOL CellsReady = false;
	TM_BOOL MLCOn = false;
	tNativeAcc ConfigIndex = 0;

//	printf("here\n");


	if( m_pConfigA->DiversityMode() != divOffA )
	{
		// *** If diversity is used ***
		// *** Process Block B ***
		valid_drm_blockB=m_drm_recB.ProcessBlock(pChan1,
												 block_monit_readyB, 
												pMonit_B_1,
												pMonit_B_2,
												 m_pFac_structB, 
												 m_pSdc_structB, 
												 m_msc_data_cell_pointers[INPUT_B_BUFFER], 
												 m_msc_data_indexB,
												 posteq_framenumB, 
												 posteq_symbolnumB,
												 Band_scan,
												 drm_foundB, 
												 m_change_flagsB,
												DRMLevelB,
												sample_rate_dev);
										
	}

	if(drm_foundB){ DP(("\n\nFound drm B\n\n")); }

	monit_readyB = valid_drm_blockB;

	WritePC(40520);

	monit_ready=FALSE; 
	
	// *** Process Block A ***
	valid_drm_blockA=m_drm_recA.ProcessBlock(pChan0,		// * Data in/out *
											 block_monit_readyA,	// * Monitor ready ? *
											pMonit_A_1,
											pMonit_A_2,
											 m_pFac_structA,	// * FAC struct pointer *
											 m_pSdc_structA,	// * SDC struct pointer *
											 m_msc_data_cell_pointers[INPUT_A_BUFFER],  // * MSC data cell pointers *
											 m_msc_data_indexA, // * MSC data index *
											 posteq_framenumA, 
											 posteq_symbolnumA,
											 Band_scan,
											 drm_found, 
											 m_change_flags,
											 DRMLevelA,
											 sample_rate_dev);
										

	monit_readyA = valid_drm_blockA;


	WritePC(40530);                             

	

	m_pConfigA=m_drm_recA.GetConfig();
	m_pConfigB=m_drm_recB.GetConfig();

	
	// *** Diversity Combiner ***
	m_diversity_combiner.SetDiversityMode(m_pConfigA->DiversityMode());

	m_diversity_combiner.UpdateDiversityState(m_pFac_structA->FACReadyFlag, 
											  m_pFac_structB->FACReadyFlag, 
											  m_pConfigA->mlc_stat(), 
											  m_pConfigB->mlc_stat());

	WritePC(40540);


	// *** A DRM block has been decoded ***
	if (valid_drm_blockA)
	{
		m_diversity_combiner.ProcessCells(0, 
										  m_pConfigA, // * Configuration for this channel *
										  m_msc_data_cell_pointers, // * Pointers to MSC data *
										  &m_msc_data_indexA,		// * MSC index * 
										  posteq_framenumA,			// * Frame nr *
										  posteq_symbolnumA,		// * Symbol number *
										  *m_drm_recA.ReadMER(),	// * quality *
										  CellsReady, 
										  MLCOn, 
										  mux_framenum, 
										  ConfigIndex);
	}

	WritePC(40550);

	// *** B DRM block has been decoded ***
	if (valid_drm_blockB)
	{
		m_diversity_combiner.ProcessCells(1, 
										  m_pConfigB, // * Configuration for this channel *
										  m_msc_data_cell_pointers, // * Pointers to MSC data *
										  &m_msc_data_indexB,		// * MSC index * 
										  posteq_framenumB,			// * Frame nr *
										  posteq_symbolnumB,		// * Symbol number *
										  *m_drm_recB.ReadMER(),	// * quality *
										  CellsReady, 
										  MLCOn, 
										  mux_framenum, 
										  ConfigIndex);
	}

	if(ConfigIndex==0)
	{
		memcpy(FACStruct, m_pFac_structA, sizeof(tFACStruct) );
		memcpy(SDCStruct, m_pSdc_structA, sizeof(tSDCStruct) );
		memcpy(change, m_change_flags, CHANGE_FLAGS);
	}
	else
	{
		memcpy(FACStruct, m_pFac_structB, sizeof(tFACStruct) );
		memcpy(SDCStruct, m_pSdc_structB, sizeof(tSDCStruct) );
	}

	WritePC(40560);


	monit_ready = (ConfigIndex==0 ? block_monit_readyA : block_monit_readyB); 


//	UInt8* msc_bytes = new UInt8[MAX_LEN_PART_A];
	

	// *** Main Service Channel MLC Decoding ***
	if(valid_drm_blockA || valid_drm_blockB)
		m_mlc_proc.MLC(
					   ConfigIndex==0 ? m_pConfigA : m_pConfigB, 
					   m_msc_data_cell_pointers[DECODE_BUFFER], 
					   monit_ready, 
					   ConfigIndex==0 ? m_pSdc_structA : m_pSdc_structB, 
					   ConfigIndex==0 ? m_pFac_structA : m_pFac_structB,
					   mux_framenum,
			           CellsReady, MLCOn, m_change_flags, m_msc_bytes);

	WritePC(40570);

	//put this in when commenting out the MLC
//	m_msc_data_index1=0;

	if(m_pSdc_structA->SDCReadyFlag)
	{
		tNativeAcc service=m_demultiplex.config(m_pConfigA->service_selected(), 
										 m_pFac_structA, 
										 m_pSdc_structA,
										 &m_p_active_stream, 
										 stream0, 
										 stream1, 
										 stream2, 
										 stream3);

		m_pConfigA->SetService(service);

		TM_BOOL DFlag=m_pFac_structA->Services[m_pConfigA->service_selected() ].DFlag;

		

	}



	WritePC(40580);


	WritePC(40590);

	//  Demultiplex 
	if(CellsReady && MLCOn)
	{
		WritePC(40591);

		m_demultiplex.demultiplex(m_msc_bytes, 
								  stream0, 
								  stream1, 
								  stream2,
								  stream3,
								  text0,
								  text1,
								  text2,
								  text3,
								  m_text_struct, 
									//NULL,
								  m_change_flags);                     


		WritePC(40597);


	}

	frame_ready = monit_ready;

	WritePC(40599);

//	delete [] msc_bytes;

  

	return (ConfigIndex==0 ? valid_drm_blockA : valid_drm_blockB);

}

void CHALayer::SendMsg(tNativeAcc channel, tNativeAcc type, tNativeAcc length, UInt8 *address, tNativeAcc *respType, tNativeAcc *respLen,
						 UInt8 **respAdd)

{

	tNativeAcc value=0;

	tNativeAcc value1=address[0];
	tNativeAcc value2=address[1];

	value = value1;

	TConfig_Command command;

	CDrmProc* pDRM_1=GetDRM();
	CDrmProc* pDRM_2=GetDRM2();
	CMSCMLC* pMSCMLC=GetMSCMLC();
	CDemultiplex* pDemul=GetDemultiplex();
	CDiversityCombiner* div = GetDivComb();


	CDrmProc* pDRM = NULL;

	if(channel == 2)
		pDRM = pDRM_2;
	else
		pDRM = pDRM_1;

	TM_BOOL sync_stat= FALSE; 
	TM_BOOL FAC = FALSE;
	TM_BOOL SDC = FALSE;
	float prop =0.0;


	switch(type)
	{
		case TODSP_MSG_GET_VER :
			GetVersion(type, address, length, respType, respLen, respAdd);
			break;
		case TODSP_MSG_GET_RF :
		//	pDRM_1->GetRF(type, address, length, respType, &return_len, &string);
			m_rf_levels[0] = pDRM_1->GetRF();
			m_rf_levels[1] = pDRM_2->GetRF();
		//	m_rf_levels[2] = pDRM_1->GetDRMLevel();
		//	m_rf_levels[3] = pDRM_2->GetDRMLevel();
			*respType=type+0x80;
			*respLen = 2;
			*respAdd = (UInt8*) &m_rf_levels;
			break;
		case TODSP_MSG_GET_DIV_PROP :
			prop = div->ReadDivProportion();
			if (prop<0.0f) 
				m_div_prop = 0xFF;
			else 
				m_div_prop = (UInt8) (prop * 100);
			*respType=type+0x80;
			*respLen = 1;
			*respAdd = (UInt8*) &m_div_prop;
		
			break;
		case TODSP_MSG_GET_SDC_RAW :
			pDRM->GetSDCRaw(type, address, length, respType, respLen, respAdd);
			break;
		case TODSP_MSG_GET_FAC_RAW :
			pDRM->GetFACRaw(type, address, length, respType, respLen, respAdd);
			break;
		case TODSP_MSG_GET_BER :
			pMSCMLC->GetBER(type, address, length, respType, respLen, respAdd);
			break;
		case TODSP_MSG_GET_DIAG_MER :
			pDRM->GetMER(type, address, length, respType, respLen, respAdd);
			break;
		case TODSP_MSG_GET_DIAG_DOP :
			pDRM->GetDop(type, address, length, respType, respLen, respAdd);
			break;
		case TODSP_MSG_GET_DIAG_DEL :
			pDRM->GetDel(type, address, length, respType, respLen, respAdd);
			break;
		case TODSP_MSG_GET_DIAG_CIR :
			pDRM->GetCIR(type, address, length, respType, respLen, respAdd);
			break;
		case TODSP_MSG_GET_DIAG_CW :
			pDRM->GetCW(type, address, length, respType, respLen, respAdd);
			break;
		case TODSP_MSG_GET_SERVICE :
			pDRM->GetServices(type, address, length, respType, respLen, respAdd);
			m_change_flags[0] &= ~NEW_AUDIO_SERVICES_MASK; //clear the flag
			break;
		case TODSP_MSG_GET_SERVICE_ACT :
			pDRM->GetServiceAct(type, address, length, respType, respLen, respAdd);
			break;
	//	case TODSP_MSG_SET_DUAL_FRONTEND :
	//		pDRM->SetDualFrontend(type, address, length, respType, &respLen, &respAdd);
	//		break;
		case TODSP_MSG_GET_STATUS :
			sync_stat=pDRM->ReadSyncStatus();
			FAC=pDRM->ReadFACStatus();
			SDC=pDRM->ReadSDCStatus();

			m_rx_status = 0xFFFF;

			if(sync_stat) m_rx_status  &= ~POST_FFT_TIME_SYNC;
			if(FAC) m_rx_status  &= ~FAC_SYNC;
			if(SDC) m_rx_status  &= ~SDC_SYNC;
			// todo diversity m_rx_status &= 

			*respType=type+0x80;
			*respLen = 2;
			*respAdd = (UInt8*) &m_rx_status;
			break;
		case TODSP_MSG_GET_CHANGE :
			GetChangeReg(type, address, length, respType, respLen, respAdd);// put back
			break;
		case TODSP_MSG_SET_CHGMASK :
			SetChangeMask(type, address, length, respType, respLen, respAdd);
			break;
//		case TODSP_MSG_GET_NEW_FREQ_1 :
//			g_pHWLayer->GetNewFreq1(type, address, length, respType, &respAdd, &respAdd);
//			break;
		case TODSP_MSG_SET_NEW_FREQ_1 :
			//set the receiver to resync etc
			command.operation=set_mode;
			command.value=(int) RECEIVER_START_MODE;
			//g_pHWLayer->ChangeMode(&command);
			pDRM_1->ReacquireMode(RECEIVER_START_MODE);
			pMSCMLC->Reacquire();
			//g_pHWLayer->GetHALayer()->Reacquire();
			//pHALayer->Reacquire();
			break;
		case TODSP_MSG_GET_NEW_FREQ_2 :
			//g_pHWLayer->GetNewFreq2(type, address, length, respType, &return_len, &string);
			break;
		case TODSP_MSG_SET_NEW_FREQ_2 :
			//set the receiver to resync etc
			command.operation=set_mode;
			command.value=(int) RECEIVER_START_MODE;
			//g_pHWLayer->ChangeMode(&command);
			pDRM_2->ReacquireMode(RECEIVER_START_MODE);
			pMSCMLC->Reacquire();
			break;
		case TODSP_MSG_SET_MON_CHAN0 :
		//	value1=xioRead(DPRAM_MON_CHAN0_BASE);
		//	value2=xioRead(DPRAM_MON_CHAN0_BASE+1);
		//	DP(("Set Mon %i %i %i\n", value, value1, value2));
			command.operation=set_mon1;
			command.value=(value2 << 8) + value1;
			pDRM->Config_Change_Command(&command);
			break;
		case TODSP_MSG_SET_MON_CHAN1 : 
		//	value1=xioRead(DPRAM_MON_CHAN1_BASE);
		//	value2=xioRead(DPRAM_MON_CHAN1_BASE+1);
			command.operation=set_mon2;
			command.value=(value2 << 8) + value1;
			pDRM->Config_Change_Command(&command);
			break;
		case TODSP_MSG_SET_SCALE_CHAN0 : 
		//	value1=xioRead(DPRAM_SCALE_CHAN0_BASE);
		//	value2=xioRead(DPRAM_SCALE_CHAN0_BASE+1);
			command.operation=set_scale1;
			command.value=(value1 << 8) + value2;
			pDRM->Config_Change_Command(&command);
			break;
		case TODSP_MSG_SET_STYLE_CHAN0 : 
		//	value1=xioRead(DPRAM_PLOT_CHAN0_BASE);
		//	value2=xioRead(DPRAM_PLOT_CHAN0_BASE+1);
			command.operation=set_plot0;
			command.value=(value1 << 8) + value2;
			pDRM->Config_Change_Command(&command);
			break;
		case TODSP_MSG_SET_SCALE_CHAN1 : 
		//	value1=xioRead(DPRAM_SCALE_CHAN1_BASE);
		//	value2=xioRead(DPRAM_SCALE_CHAN1_BASE+1);
			command.operation=set_scale2;
			command.value=(value1 << 8) + value2;
			pDRM->Config_Change_Command(&command);
			break;
		case TODSP_MSG_SET_STYLE_CHAN1 : 
		//	value1=xioRead(DPRAM_PLOT_CHAN1_BASE);
		//	value2=xioRead(DPRAM_PLOT_CHAN1_BASE+1);
			command.operation=set_plot1;
			command.value=(value1 << 8) + value2;
			pDRM->Config_Change_Command(&command);
			break;
		case TODSP_MSG_SPECTRAL_INV : 
			*respLen = 0;
			*respAdd = NULL;
		//	value1=xioRead(DPRAM_SPEC_INV_BASE);
		//	value2=xioRead(DPRAM_SPEC_INV_BASE+1);
			command.operation=set_spec_invert;
		//	command.value=(value1 << 8) + value2;
			command.value = value1;
			pDRM_1->Config_Change_Command(&command);
			pDRM_2->Config_Change_Command(&command);
			break;
		case TODSP_MSG_SET_CHAN_FILT_DRM :
			*respLen =0;
			*respAdd =NULL;
		//	value1=xioRead(DPRAM_CHAN_FILT_BASE);
		//	value2=xioRead(DPRAM_CHAN_FILT_BASE+1);
			command.operation=set_chan_filt;
			command.value=value1;
		//	pDRM->SetChannelFilter(type, address, length, respType, &return_len, &string);
			pDRM_1->Config_Change_Command(&command);
			pDRM_2->Config_Change_Command(&command);
		//	g_pHWLayer->SetChanFilt((value1 << 8) + value2);
			break;
		case TODSP_MSG_SET_AFS_MODE :
			*respLen =0;
			*respAdd =NULL;
		//	value1=xioRead(DPRAM_CHANNEL_SELECT_BASE);
		//	value2=xioRead(DPRAM_CHANNEL_SELECT_BASE+1);
		//	g_pHWLayer->SetChannel( (value1 << 8) + value2);
			command.operation = set_div_mode;
		//	command.value=(value1 << 8) + value2;
			command.value= value1;
			pDRM_1->Config_Change_Command(&command);
			break;
		case TODSP_MSG_DEMOD_MODE :
			*respLen =0;
			*respAdd =NULL;
		//	value1=xioRead(DPRAM_MODE_BASE);
		//	value2=xioRead(DPRAM_MODE_BASE+1);
		//	switch(value1)
		//	switch((value2 << 8) + value1)
			switch((value1 << 8) + value2)
			{
			case (MODE_AUTO) :
			//	g_pHWLayer->SetDemodType(drm);
				command.operation=set_mode_detect;
				command.value=(int) on;
				pDRM_1->Config_Change_Command(&command);
				break;
			case (MODE_A) :
			//	g_pHWLayer->SetDemodType(drm);
				command.operation=set_mode;
				command.value=(int) ground;
				pDRM_1->Config_Change_Command(&command);
				pDRM_2->Config_Change_Command(&command);
			//	g_pHWLayer->ChangeMode(&command);
				break;
			case (MODE_B) :
			//	g_pHWLayer->SetDemodType(drm);
				command.operation=set_mode;
				command.value=(int) sky;
				pDRM_1->Config_Change_Command(&command);
				pDRM_2->Config_Change_Command(&command);
			//	g_pHWLayer->ChangeMode(&command);
				break;
			case (MODE_C) :
			//	g_pHWLayer->SetDemodType(drm);
				command.operation=set_mode;
				command.value=(int) robust1;
				pDRM_1->Config_Change_Command(&command);
				pDRM_2->Config_Change_Command(&command);
			//	g_pHWLayer->ChangeMode(&command);
				break;
			case (MODE_D) :
			//	g_pHWLayer->SetDemodType(drm);
				command.operation=set_mode;
				command.value=(int) robust2;
				pDRM_1->Config_Change_Command(&command);
				pDRM_2->Config_Change_Command(&command);
			//	g_pHWLayer->ChangeMode(&command);
				break;
			default : //DRM AUTO MODE
			//	g_pHWLayer->SetDemodType(drm);
				command.operation=set_mode_detect;
				command.value=(int) on;
				pDRM_1->Config_Change_Command(&command);
				pDRM_2->Config_Change_Command(&command);
				break;
			}
			break;
		case TODSP_MSG_RESET_BER :
			command.operation=reset_CRC_count;
			pDRM_1->Config_Change_Command(&command);
			pDRM_2->Config_Change_Command(&command);
			pMSCMLC->ResetBERCount();
			break;
/*		case TODSP_MSG_GET_READY_STATUS :
			g_pHWLayer->SetBootFlag();
			break;
		case TODSP_MSG_GET_WATCHDOG :
			if(!g_pHWLayer->GetWatchDog() )
				*respType = TODSP_MSG_ANS_ERR;
			break;*/
		case TODSP_MSG_RESTART :
			command.operation=set_mode;
			command.value=(int) RECEIVER_START_MODE;
		//	g_pHWLayer->ChangeMode(&command);
			//pDRM_1->ReacquireMode(RECEIVER_START_MODE); // Now done by SET_NEW_FREQ_1
			//pDRM_2->ReacquireMode(RECEIVER_START_MODE); // Now done by SET_NEW_FREQ_2
			pMSCMLC->Reacquire();
			break;
		case TODSP_MSG_SET_INTER :
			pDRM_1->SetInter(value1);
			break;
		case TODSP_MSG_SEL_SERVICE :
		//	value1=xioRead(DPRAM_DRM_SERVICE_BASE);
		//	value2=xioRead(DPRAM_DRM_SERVICE_BASE+1);
			command.operation=set_service;
			command.value=(value1);// << 8) + value2;
			pDRM_1->Config_Change_Command(&command);
			pDRM_2->Config_Change_Command(&command);
			printf("\n\n............. sel service %i \n\n", value1);
			break;
		case TODSP_MSG_SET_AUDIO_OUT :
		//	value1=xioRead(DPRAM_VOLUME_CONTROL_BASE);
		//	value2=xioRead(DPRAM_VOLUME_CONTROL_BASE+1);
			*respLen =0;
			*respAdd =NULL;
		//	pDecoder->SetVolume(value1, value2);
		//	pAM->SetVolume(value1, value2);
			break;
		case TODSP_MSG_GET_AUDIO_OUT :
		//	value1=xioRead(DPRAM_VOLUME_CONTROL_BASE);
		//	value2=xioRead(DPRAM_VOLUME_CONTROL_BASE+1);
			*respLen =0;
			*respAdd =NULL;
		//	pDecoder->GetVolume(type, address, length, respType, &return_len, &string);
			break;
		case TODSP_MSG_GET_LABEL :
			
			if(value > 4 ) value = 4;
			pDRM->GetLabel(type, address, length, respType, respLen, respAdd, value);

			/*(if(m_demod_type == drm)
				pDRM->GetLabel(type, address, length, retType, retLen, retAdd, service);
			else
				pAMSS->GetLabel(type, address, length, retType, retLen, retAdd, service);*/

			value+=2;
			if(value == 6) value = 1;

			//clear the flag
			m_change_flags[value] &= ~NEW_LABEL_MASK;
			
			break;
		case TODSP_MSG_GET_LABEL_NEW : 
			if(value2 > 4 ) value = 4;
			pDRM->GetLabelNewSpec(type, address, length, respType, respLen, respAdd, value);

			value+=2;
			if(value == 6) value = 1;

			//clear the flag
		//	m_change_flags[service] &= ~NEW_LABEL_MASK;
			m_change_flags[value] &= ~NEW_LABEL_MASK;
			break;

		case TODSP_MSG_GET_SERVICE_MISC :
			pDRM->GetMisc(type, address, length, respType, respLen, respAdd);
			m_change_flags[0] &= ~NEW_MISC_INFO;
			break;

		case TODSP_MSG_GET_TEXT : 
			if(value > 4 ) value = 4;
				
			pDemul->GetText(type, address, length, respType, respLen, respAdd, value);

			value+=2;
			if(value == 6) value = 1;

			m_change_flags[value2] &= ~NEW_TEXT_MASK;
			break;
		case TODSP_MSG_GET_SERVICE_ID :
			if(value > 4 ) value = 4;
			pDRM->GetServiceID(type, address, length, respType, respLen, respAdd, value);
			m_change_flags[value] &= ~NEW_SERVICE_ID;
			break;
		case TODSP_MSG_GET_LANG :
			if(value > 4 ) value = 4;
			pDRM->GetLang(type, address, length, respType, respLen, respAdd, value);
			value+=2;
			m_change_flags[value] &= ~NEW_LANGUAGE_MASK;
			break;
		case TODSP_MSG_GET_AUDIO :
			if(value > 4 ) value = 4;
			pDRM->GetAudioType(type, address, length, respType, respLen, respAdd, value);
			m_change_flags[value] &= ~NEW_AUDIO_MASK;
			break;
		case TODSP_MSG_GET_PROG :
			if(value > 4 ) value = 4;
			pDRM->GetProg(type, address, length, respType, respLen, respAdd, value);
			m_change_flags[value] &= ~NEW_PROGRAMME_MASK;
			break;

		default : 
			*respType = TODSP_MSG_ANS_ERR;
			break;
	}


	
//	*respLen=return_len;
//	*respAdd=(UInt8*) string;



}

void CHALayer::GetChangeReg(tNativeAcc type, UInt8 *address, tNativeAcc length,
							   tNativeAcc *retType, tNativeAcc *retLen, UInt8 **retAdd)
{

	for(tNativeAcc i=0; i<CHANGE_FLAGS; i++)
		m_masked_flags[i] = m_change_mask[i] & m_change_flags[i];

	*retType=type+0x80;

	*retLen = CHANGE_FLAGS;
	*retAdd = (UInt8*) m_masked_flags;

}

void CHALayer::SetChangeMask(tNativeAcc type, UInt8 *address, tNativeAcc length,
							   tNativeAcc *retType, tNativeAcc *retLen, UInt8 **retAdd)
{
	tNativeAcc i;

	*retType=type+0x80;

	*retLen = 0;
	*retAdd = NULL;

	for(i=0; i< length; i++)
		m_change_mask[i]=address[i];
}

UInt8* CHALayer::GetMask()
{
	return m_change_mask;
}

UInt8* CHALayer::GetFlags()
{
	return m_change_flags;
}

unsigned long* CHALayer::GetFreq()
{
	return &m_freq1_set;
}

void CHALayer::Reacquire()
{
//	m_bDecoderStatus=FALSE;
}

void CHALayer::WritePC(UInt16 value)
{

	tNativeAcc stop=sizeof(UInt16);
	tNativeAcc i;

#ifndef _CAB
	for(i=0; i< stop; i++)
		xioWrite(DPRAM_DRM_PC_BASE+i, (char) (value >> (i*8) ) );
#endif
}

char* CHALayer::GetVersion(tNativeAcc type, unsigned char* data, tNativeAcc length,
		tNativeAcc* return_type, tNativeAcc* return_length, UInt8** return_data)
{
	*return_type = type+0x80;
	*return_data = (UInt8*) m_version_str;
	*return_length = strlen(m_version_str) +1;

	return m_version_str;

}


/*void CHALayer::CopyMLCStats(unsigned char *buffer)
{
	int i=0,level,j;
	char* Flags;


	tMLCErrorStruct Part_A=ReadPartA_BER();
	tMLCErrorStruct Part_B=ReadPartB_BER();

	//todo
//	Flags = ReadCorruptFlags();

	
	buffer[i++]=(char) ReadStatus();
	buffer[i++]=(char) (ReadStatus() >> 8);

	charint PartA_err;
	charint PartA_bit;
	charint PartB_err;
	charint PartB_bit;

	PartA_err.i=0;
	PartA_bit.i=0;
	PartB_err.i=0;
	PartB_bit.i=0;

	//sum up the errors
	for(level=0; level < NUM_LEVELS; level++){
		PartA_err.i +=Part_A.errors[level].i;
		PartA_bit.i +=Part_A.bits[level].i;
		PartB_err.i +=Part_B.errors[level].i;
		PartB_bit.i +=Part_B.bits[level].i;
	}

	int start =0;
	int stop =2;


	//stream 0
	for(j=start; j<stop; j++)
		buffer[i++]=PartA_err.c[j];
	for(j=start; j<stop; j++)
		buffer[i++]=PartA_bit.c[j];
	for(j=start; j<stop; j++)
		buffer[i++]=PartB_err.c[j];
	for(j=start; j<stop; j++)
		buffer[i++]=PartB_bit.c[j];

	//stream 1
	for(j=start; j<stop; j++)
		buffer[i++]=0;//PartA_err.c[j];
	for(j=0; j<stop; j++)
		buffer[i++]=0;//PartA_bit.c[j];
	for(j=0; j<stop; j++)
		buffer[i++]=0;//PartB_err.c[j];
	for(j=0; j<stop; j++)
		buffer[i++]=0;//PartB_bit.c[j];

	//stream 2
	for(j=start; j<stop; j++)
		buffer[i++]=0;//PartA_err.c[j];
	for(j=start; j<stop; j++)
		buffer[i++]=0;//PartA_bit.c[j];
	for(j=start; j<stop; j++)
		buffer[i++]=0;//PartB_err.c[j];
	for(j=start; j<stop; j++)
		buffer[i++]=0;//PartB_bit.c[j];

	//stream 3
	for(j=start; j<stop; j++)
		buffer[i++]=0;//PartA_err.c[j];
	for(j=start; j<stop; j++)
		buffer[i++]=0;//PartA_bit.c[j];
	for(j=start; j<stop; j++)
		buffer[i++]=0;//PartB_err.c[j];
	for(j=start; j<stop; j++)
		buffer[i++]=0;//PartB_bit.c[j];



//	char string[100];
//	sprintf(string, "%X %X %X %X %X %X %X %X %X %X", buffer[0], buffer[1], buffer[6], buffer[7],
//		buffer[8], buffer[9] ,buffer[34], buffer[35],  buffer[40], buffer[41] );
//	XIO->DPRAMwrite(MSG_TYPE_DEBUG, string);



//	buffer[i++]=(char) ReadDRMError(); todo


//	buffer[i++]=(char) ReadVULeft();
//	buffer[i++]=(char) ReadVURight();

//	ResetNumAudioFrames(); todo

	m_mlc_stat_length=i;
}*/

/*unsigned char* CHALayer::ReadMLCStats(int& length)
{
	length=m_mlc_stat_length;

	memcpy(m_mlc_stats, m_mlc_stats1, MLC_STAT_SIZE );
	if(m_bMSCStats){
		memcpy(m_mlc_stats1, m_mlc_stats2, MLC_STAT_SIZE );
		m_bMSCStats=FALSE;
		m_bReadMLC=FALSE;
	}
	else{
		m_bReadMLC=TRUE;
	}

	return(m_mlc_stats);
}*/

int CHALayer::Dummy(void)
{
	return TRUE;
}