
#include "cwinter.h"
#include "../pc_types.h"
#include <stdio.h>
#include "../S_Defs.h"
#include <string.h>
#include <math.h>

#define NONE -1000

CCWInter::CCWInter()
{
	tNative i;

	for(i = 0; i< MAX_NO_INTERS; i++)
		m_old_freq[i] = NONE;
	
}

CCWInter::~CCWInter()
{


}

void CCWInter::Detect(tNative* data_in, tNative* data_out, tNative size, tNativeAcc* freq,
					  tNative* bin_1, tNative* bin_2)
{
	tNative* restrict mag_store = data_out;
	tNative i;

	tUNativeAcc mag_tot = 0;

	//float mag;

//	tNative re, im;
	tUNative mag,filt;

	tNativeAcc re, im;

	//take the mag of the data to find the mean
	for(i= 0; i< 2*size; i+=2)
	{
		re = data_in[i];
		im = data_in[i+1];

		mag = NATIVE_CAST_UN(( re*re + im*im )>>16);//watch this space
		
		mag_store[i] = mag;
		mag_tot += mag;
	}

//	mag_tot = sqrt(mag_tot);

	DP(("mag 1 %i \n", mag_tot));

	mag_tot /= size;


	//set a threshold
	//TODO scale it with FFT size
	//float scale = FLOAT(1.0f/sqrt((float) size) );
	tNative thres = 10 * mag_tot;

	DP(("mag %i \n", mag_tot));


	tNative bin1 = NONE, bin2 = NONE;

	tNative temp=0;
	//now go thorugh and look for bin higher than average
	for(i = 0; i< 2*size; i +=2)
	{
		if(mag_store[i] > thres)
		{
			if(mag_store[i] > temp)
			{
				temp = mag_store[i];
				bin1 = i;
				//break;
			}
		}
		
	}

//	bin = NONE;

	if(bin1 == NONE)
	{
		freq[0] = -(1<<23);

		DP(("NOT FOUND ret %i\n", freq[0]));
		return;
	}
	else
	{
		*bin_1 = bin1;
//		m_num_inters = 1;
	}

	CalculateFreqs(mag_store, bin1, 0, size, freq);

	//now look for a second interferer, but ignore the first
	//need to break the loop into two parts
	tNative end = bin1 - 10;
	if (end < 0) end = 0;
	tNative start = bin1 + 10;
	if (start > 2* size) start = 2* size;

	temp  = 0;
	for(i=0; i < end; i+=2)
	{
		if(mag_store[i] > thres)
		{
			if(mag_store[i] > temp)
			{
				temp = mag_store[i];
				bin2 = i;
				//break;
			}
		}
	}

	for(i = start; i< 2*size; i +=2)
	{
		if(mag_store[i] > thres)
		{
			if(mag_store[i] > temp)
			{
				temp = mag_store[i];
				bin2 = i;
				//break;
			}
		}
	}


	if(bin2 == NONE)
	{
		freq[1] = -(1<<23);

		//DP(("ret %f\n", freq[1]));
		return;
	}
	else
	{
		*bin_2 = bin2;
//		m_num_inters = 2;
	}

	CalculateFreqs(mag_store, bin2, 1, size, freq);

	DP(("FOUND Detected bin %i\n", bin2));


//	DP(("freq %f \n", Freq));

//	DP(("----------> Inter bin %f %f freq %f\n", t_pos, bin_splice, Freq));


}

void CCWInter::CalculateFreqs(tNative* mag_store, tNative bin,
							  tNative num, tNative size, tNativeAcc* freq)
{

		//do some interpolation as the interferer may not fall exactly on the grid
	tNative mag1 = mag_store[bin-2];
	tNative mag2 = mag_store[bin];
	tNative mag3 = mag_store[bin+2];

	tNative t_bin = bin;

	t_bin /=2;
	t_bin -= size/4;

	//DP(("mag_store %i %i %i %i %i\n", mag_store[bin-2], mag_store[bin], mag_store[bin+2], bin, t_bin ));

	tNative bin_splice = bin;
	tNative bin_splice2 = bin;

	tNativeAcc grad1, grad2;

	int type = 0;

	if(mag1 > 2*mag3) //peak is between 1 and 2
	{
		
		bin_splice -=1;	

	//	DP((" 1 and 2 %f \n", grad));

	}

	if(mag3 > 2*mag1) //peak is between 2 and 3
	{
		
		bin_splice +=1;

	//	DP((" 2 and 3 %f \n", grad));

	}

	//this is a better estimate
	if(mag1 > mag3) //peak is between 1 and 2
	{
		type = 1;
		grad1 = (1<< 16)* mag1/(mag2+mag1);
		grad2 = (1<< 16)* mag2/(mag2+mag1);
	//	DP(("1 and 2 %i %i %i\n", grad1, grad2, bin_splice2));

	}

	else
	//if(mag3 > mag1) //peak is between 2 and 3
	{
		type = 2;
		grad1 = (1<< 16) * mag2/(mag3+mag2);
		grad2 = (1<< 16) * mag3/(mag3+mag2);
	//	DP(("2 and 3 %i %i %i\n", grad1, grad2, bin_splice2));

	}


	tNativeAcc t_pos;

	if(type == 1)
	{
		t_pos = grad1*(bin_splice2-2)+grad2*bin_splice2;

		//t_pos /= (1<< 16);

	//	DP(("1 t_pos %i\n", t_pos));
	}
	else
	{
		t_pos = grad1*(bin_splice2)+grad2*(bin_splice2+2);

		//t_pos /= (1<< 16);

	//	DP(("2 t_pos %i\n", t_pos));
	}

	t_pos >>= 1; //divide by 2

	t_pos -= (size * (1 << 14) ); //subtract by size/4

	t_pos;

	//DP(("T POS %i \n", t_pos));

	//t_pos /=2.0f;
	//t_pos -= size/4.0f;

	bin_splice /=2; //because bin was calculated over complex points

	//to convert to k
	bin_splice -= size/4;

	//DP(("t_pos %f", t_pos));
	//DP(("slice %f %f\n", bin_splice, t_pos));

	//tNative Freq = (float)(bin_splice) / (float)size;

	//tNativeAcc Freq = (t_pos)*(1<< 7)/ size;

	tNativeAcc Freq = (t_pos)/ size;

	//float Freq2 = (float) (t_pos)/(size * (1<< 16) );
	//tNativeAcc Freq = (1<<7)*(t_pos) / (size * (1<<16));



	//DP(("freq %i %f\n", Freq, Freq2));

	tNative CTPF_BITS=14;
	tNative CTPF_ONE=(1<<CTPF_BITS);

	tNativeAcc k = 1638;// 0.1f
	tNativeAcc kk = (CTPF_ONE - k);

	//DP(("Freq %i\n", Freq));

	//average the freq
	Freq = k*Freq+kk*m_old_freq[num];
	Freq /= CTPF_ONE;

	//DP(("Freq Old %i %i\n", Freq, m_old_freq));


	m_old_freq[num] = Freq;

	Freq *=  (1<<7); //to scale Freq up to 2^23

	//Freq -= 0.25; // to correct for frequency shift
	Freq -= 2097152; // 0.25 * 2^23 to correct for frequency shift

	freq[num] = Freq;


}
