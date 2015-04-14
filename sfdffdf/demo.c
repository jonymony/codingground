#include <stdio.h>
#include <string.h>
#include "drmproc.h"
#include <fstream>
#include <iostream>

tNative chaninputbuffer[76800];
short chanoutputbuffer[76800];
int main()
 {
	CDrmProc a;
	tNative a1=10;
	int c=10;
	tNative e=0,f=0;
	TM_BOOL d=true,g=true;

	FILE *file_chanin = NULL;
	FILE *file_chanout = NULL;
 	file_chanin = fopen("F:/Input_Files/DRM_ModeA_48KHz.if12", "rb");
    file_chanout= fopen("F:/chanoutput.bin","wb");

    	fread(chaninputbuffer, sizeof(int),76800,file_chanin);
    	tFACStruct *fa;
     	MonitorType  * mon1;
     	MonitorType  * mon2;
     	tSDCStruct	 * SD;
     	tFRAME_CELL	 * msc;
     	float sample_rate = 48000;
     	sample_rate = (sample_rate/ DRM_SAMPLE_RATE) - 2.0f;
     	tUNativeAcc sam = sample_rate * (1<< 23);
     	a.ProcessBlock(chaninputbuffer, g, mon1,mon2,fa,SD,msc, c, e, f, TRUE, d, 0, &a1,sam );
     	// fwrite(chanoutputbuffer,sizeof(int),76800,file_chanin);   //outbuffer to fileout

    fclose(file_chanin);
    fclose(file_chanout);
}