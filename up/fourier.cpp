////////////////////////////////////////////////////////////////////// 
//Fourier.cpp: implementation of the CFourier class.
//////////////////////////////////////////////////////////////////////

#include "Fourier.h"
#include "math.h"
#include <stdlib.h> //for malloc
#include <stdio.h>


#include "../pc_types.h"

//#define SCALE_DOWN_IFFT_OFF

//*****************************************************
//*		Defs and Macros for Side Band Swap function
//*****************************************************
#define SBS_INOUT_BITS 16
#define SBS_MULTISHIFT (SBS_INOUT_BITS-1)
#define SBS_ONE ((1<<SBS_MULTISHIFT)-1)

//*****************************************************
//*		Defs and Macros for the FFT
//*****************************************************
#define FFT_INOUT_BITS 16 //15+1
#define FFT_MULTISHIFT (FFT_INOUT_BITS-1)
#define FFT_ONE ((1<<FFT_MULTISHIFT)-1)
#define FFT_M(A,B) (NATIVE_CAST(((A) * (B))>>FFT_MULTISHIFT))

//*****************************************************
//*		Defs and Macros for the FFT and IFFT
//*****************************************************

#define IFFT_PASS1 2

#define F576_PASS1 2
#define F576_PASS2 2

#define F512_PASS1 2
#define F512_PASS2 2

#define F352_PASS1 1
#define F352_PASS2 1

#define F224_PASS1 1
#define F224_PASS2 1


//*****************************************************
//*		Constants for the Fixed Point FFT/IFFT
//*****************************************************
// * 15+1 bits *
static const fftw_real 	K923879532 = FFTW_KONST(+30272);
static const fftw_real 	K382683432 = FFTW_KONST(+12539);
static const fftw_real 	K707106781 = FFTW_KONST(+23169);
static const fftw_real 	K939692620 = FFTW_KONST(+30790);
static const fftw_real 	K342020143 = FFTW_KONST(+11206);
static const fftw_real 	K984807753 = FFTW_KONST(+32269);
static const fftw_real 	K173648177 = FFTW_KONST(+5689);
static const fftw_real 	K642787609 = FFTW_KONST(+21062);
static const fftw_real 	K766044443 = FFTW_KONST(+25100);
static const fftw_real 	K500000000 = FFTW_KONST(+16383);
static const fftw_real 	K866025403 = FFTW_KONST(+28377);
static const fftw_real 	K222520933 = FFTW_KONST(+7291);
static const fftw_real 	K900968867 = FFTW_KONST(+29522);
static const fftw_real 	K623489801 = FFTW_KONST(+20429);
static const fftw_real 	K433883739 = FFTW_KONST(+14217);
static const fftw_real 	K974927912 = FFTW_KONST(+31945);
static const fftw_real 	K781831482 = FFTW_KONST(+25618);
static const fftw_real 	K959492973 = FFTW_KONST(+31439);
static const fftw_real 	K654860733 = FFTW_KONST(+21457);
static const fftw_real 	K142314838 = FFTW_KONST(+4663);
static const fftw_real 	K415415013 = FFTW_KONST(+13611);
static const fftw_real 	K841253532 = FFTW_KONST(+27565);
static const fftw_real 	K540640817 = FFTW_KONST(+17715);
static const fftw_real 	K909631995 = FFTW_KONST(+29805);
static const fftw_real 	K989821441 = FFTW_KONST(+32433);
static const fftw_real 	K755749574 = FFTW_KONST(+24763);
static const fftw_real 	K281732556 = FFTW_KONST(+9231);




//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFourier::CFourier()
{	
	W224_16=GenerateTwiddles(224,14); 
	W14_7=GenerateTwiddles(14,2); 
	W32_8=GenerateTwiddles(32,4);
	W512_16=GenerateTwiddles(512,32);
	W36_9=GenerateTwiddles(36,4);
	W576_16=GenerateTwiddles(576,36);
	W64_4=GenerateTwiddles(64,16);
	W128_8=GenerateTwiddles(128,16);
	W1024_8=GenerateTwiddles(1024,128);
	W576_9=GenerateTwiddles(576,64);
	W1152_9=GenerateTwiddles(1152,128);
	W448_4=GenerateTwiddles(448,112);
	W112_7=GenerateTwiddles(112,16);
	W352_8=GenerateTwiddles(352,44);
	W44_4=GenerateTwiddles(44,11);
	W88_8=GenerateTwiddles(88,11);
	W704_8=GenerateTwiddles(704,88);
}

CFourier::~CFourier()
{	
	_cache_free(W224_16);
	_cache_free(W14_7);
	_cache_free(W512_16);
	_cache_free(W32_8);
	_cache_free(W36_9);
	_cache_free(W576_16);
	_cache_free(W64_4);
	_cache_free(W128_8);
	_cache_free(W1024_8);
	_cache_free(W576_9);
	_cache_free(W1152_9);
	_cache_free(W448_4);
	_cache_free(W112_7);
	_cache_free(W352_8);
	_cache_free(W44_4);
	_cache_free(W88_8);
	_cache_free(W704_8);
}





//**************************************************************************
//*
//*			Function name : *** Side Band Swap ***
//*
//*			Description : Swap the sidebands 
//*
//*			Tree : 
//*				
//*				
//**************************************************************************

//********************************************************************
//*			*** SideBandSwap ***     Fixed Point Version 
//********************************************************************

void CFourier::SideBandSwap(tNative *data_in, tNativeAcc nn)
{
	tNative* restrict data = data_in;
	
	tNativeAcc i;
		
	tNative tempr1, tempr2, tempi1,tempi2;
	tNative tempr3, tempr4, tempi3,tempi4;
	
	// *** Swap the sidebands ***
	for (i=0; i<nn; i+=4)
	{
		tempr1=data[i]; 
		tempr2=data[i+2];
		tempi1=data[i+1]; 
		tempi2=data[i+3];
		tempr3=data[i+nn]; 
		tempr4=data[i+nn+2];
		tempi3=data[i+nn+1]; 
		tempi4=data[i+nn+3];

		data[i]=tempr3;
		data[i+1]=tempi3;
		data[i+2]=tempr4;
		data[i+3]=tempi4;

		data[i+nn]=tempr1;
		data[i+nn+1]=tempi1;
		data[i+nn+2]=tempr2;
		data[i+nn+3]=tempi2;
	}

}




void CFourier::fftw_twiddle_8(fftw_complex *A, const fftw_complex *W, tNativeAcc iostride, tNativeAcc m, tNativeAcc dist)
{

	 tNativeAcc i;
     fftw_complex *inout;
     inout = A;
     for (i = m; i > 0; i = i - 1, inout = inout + dist, W = W + 7) {
	  fftw_real tmp7;
	  fftw_real tmp43;
	  fftw_real tmp71;
	  fftw_real tmp76;
	  fftw_real tmp41;
	  fftw_real tmp53;
	  fftw_real tmp56;
	  fftw_real tmp65;
	  fftw_real tmp18;
	  fftw_real tmp77;
	  fftw_real tmp46;
	  fftw_real tmp68;
	  fftw_real tmp30;
	  fftw_real tmp48;
	  fftw_real tmp51;
	  fftw_real tmp64;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp1;
	       fftw_real tmp70;
	       fftw_real tmp6;
	       fftw_real tmp69;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp1 = c_re(inout[0]);
	       tmp70 = c_im(inout[0]);
	       {
		    fftw_real tmp3;
		    fftw_real tmp5;
		    fftw_real tmp2;
		    fftw_real tmp4;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp3 = c_re(inout[4 * iostride]);
		    tmp5 = c_im(inout[4 * iostride]);
		    tmp2 = c_re(W[3]);
		    tmp4 = c_im(W[3]);
		    tmp6 = FFT_M(tmp2 , tmp3) - FFT_M(tmp4 , tmp5);
		    tmp69 = FFT_M(tmp4 , tmp3) + FFT_M(tmp2 , tmp5);
	       }
	       tmp7 = tmp1 + tmp6;
	       tmp43 = tmp1 - tmp6;
	       tmp71 = tmp69 + tmp70;
	       tmp76 = tmp70 - tmp69;
	  }
	  {
	       fftw_real tmp35;
	       fftw_real tmp54;
	       fftw_real tmp40;
	       fftw_real tmp55;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp32;
		    fftw_real tmp34;
		    fftw_real tmp31;
		    fftw_real tmp33;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp32 = c_re(inout[7 * iostride]);
		    tmp34 = c_im(inout[7 * iostride]);
		    tmp31 = c_re(W[6]);
		    tmp33 = c_im(W[6]);
		    tmp35 = FFT_M(tmp31 , tmp32) - FFT_M(tmp33 , tmp34);
		    tmp54 = FFT_M(tmp33 , tmp32) + FFT_M(tmp31 , tmp34);
	       }
	       {
		    fftw_real tmp37;
		    fftw_real tmp39;
		    fftw_real tmp36;
		    fftw_real tmp38;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp37 = c_re(inout[3 * iostride]);
		    tmp39 = c_im(inout[3 * iostride]);
		    tmp36 = c_re(W[2]);
		    tmp38 = c_im(W[2]);
		    tmp40 = FFT_M(tmp36 , tmp37) - FFT_M(tmp38 , tmp39);
		    tmp55 = FFT_M(tmp38 , tmp37) + FFT_M(tmp36 , tmp39);
	       }
	       tmp41 = tmp35 + tmp40;
	       tmp53 = tmp35 - tmp40;
	       tmp56 = tmp54 - tmp55;
	       tmp65 = tmp54 + tmp55;
	  }
	  {
	       fftw_real tmp12;
	       fftw_real tmp44;
	       fftw_real tmp17;
	       fftw_real tmp45;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp9;
		    fftw_real tmp11;
		    fftw_real tmp8;
		    fftw_real tmp10;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp9 = c_re(inout[2 * iostride]);
		    tmp11 = c_im(inout[2 * iostride]);
		    tmp8 = c_re(W[1]);
		    tmp10 = c_im(W[1]);
		    tmp12 = FFT_M(tmp8 , tmp9) - FFT_M(tmp10 , tmp11);
		    tmp44 = FFT_M(tmp10 , tmp9) + FFT_M(tmp8 , tmp11);
	       }
	       {
		    fftw_real tmp14;
		    fftw_real tmp16;
		    fftw_real tmp13;
		    fftw_real tmp15;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp14 = c_re(inout[6 * iostride]);
		    tmp16 = c_im(inout[6 * iostride]);
		    tmp13 = c_re(W[5]);
		    tmp15 = c_im(W[5]);
		    tmp17 = FFT_M(tmp13 , tmp14) - FFT_M(tmp15 , tmp16);
		    tmp45 = FFT_M(tmp15 , tmp14) + FFT_M(tmp13 , tmp16);
	       }
	       tmp18 = tmp12 + tmp17;
	       tmp77 = tmp12 - tmp17;
	       tmp46 = tmp44 - tmp45;
	       tmp68 = tmp44 + tmp45;
	  }
	  {
	       fftw_real tmp24;
	       fftw_real tmp49;
	       fftw_real tmp29;
	       fftw_real tmp50;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp21;
		    fftw_real tmp23;
		    fftw_real tmp20;
		    fftw_real tmp22;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp21 = c_re(inout[iostride]);
		    tmp23 = c_im(inout[iostride]);
		    tmp20 = c_re(W[0]);
		    tmp22 = c_im(W[0]);
		    tmp24 = FFT_M(tmp20 , tmp21) - FFT_M(tmp22 , tmp23);
		    tmp49 = FFT_M(tmp22 , tmp21) + FFT_M(tmp20 , tmp23);
	       }
	       {
		    fftw_real tmp26;
		    fftw_real tmp28;
		    fftw_real tmp25;
		    fftw_real tmp27;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp26 = c_re(inout[5 * iostride]);
		    tmp28 = c_im(inout[5 * iostride]);
		    tmp25 = c_re(W[4]);
		    tmp27 = c_im(W[4]);
		    tmp29 = FFT_M(tmp25 , tmp26) - FFT_M(tmp27 , tmp28);
		    tmp50 = FFT_M(tmp27 , tmp26) + FFT_M(tmp25 , tmp28);
	       }
	       tmp30 = tmp24 + tmp29;
	       tmp48 = tmp24 - tmp29;
	       tmp51 = tmp49 - tmp50;
	       tmp64 = tmp49 + tmp50;
	  }
	  {
	       fftw_real tmp19;
	       fftw_real tmp42;
	       fftw_real tmp63;
	       fftw_real tmp66;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp19 = tmp7 + tmp18;
	       tmp42 = tmp30 + tmp41;
	       c_re(inout[4 * iostride]) = tmp19 - tmp42;
	       c_re(inout[0]) = tmp19 + tmp42;
	       {
		    fftw_real tmp73;
		    fftw_real tmp74;
		    fftw_real tmp67;
		    fftw_real tmp72;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp73 = tmp41 - tmp30;
		    tmp74 = tmp71 - tmp68;
		    c_im(inout[2 * iostride]) = tmp73 + tmp74;
		    c_im(inout[6 * iostride]) = tmp74 - tmp73;
		    tmp67 = tmp64 + tmp65;
		    tmp72 = tmp68 + tmp71;
		    c_im(inout[0]) = tmp67 + tmp72;
		    c_im(inout[4 * iostride]) = tmp72 - tmp67;
	       }
	       tmp63 = tmp7 - tmp18;
	       tmp66 = tmp64 - tmp65;
	       c_re(inout[6 * iostride]) = tmp63 - tmp66;
	       c_re(inout[2 * iostride]) = tmp63 + tmp66;
	       {
		    fftw_real tmp59;
		    fftw_real tmp78;
		    fftw_real tmp62;
		    fftw_real tmp75;
		    fftw_real tmp60;
		    fftw_real tmp61;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp59 = tmp43 - tmp46;
		    tmp78 = tmp76 - tmp77;
		    tmp60 = tmp51 - tmp48;
		    tmp61 = tmp53 + tmp56;
		    tmp62 = FFT_M(K707106781 , (tmp60 - tmp61));
		    tmp75 = FFT_M(K707106781 , (tmp60 + tmp61));
		    c_re(inout[7 * iostride]) = tmp59 - tmp62;
		    c_re(inout[3 * iostride]) = tmp59 + tmp62;
		    c_im(inout[iostride]) = tmp75 + tmp78;
		    c_im(inout[5 * iostride]) = tmp78 - tmp75;
	       }
	       {
		    fftw_real tmp47;
		    fftw_real tmp80;
		    fftw_real tmp58;
		    fftw_real tmp79;
		    fftw_real tmp52;
		    fftw_real tmp57;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp47 = tmp43 + tmp46;
		    tmp80 = tmp77 + tmp76;
		    tmp52 = tmp48 + tmp51;
		    tmp57 = tmp53 - tmp56;
		    tmp58 = FFT_M(K707106781 , (tmp52 + tmp57));
		    tmp79 = FFT_M(K707106781 , (tmp57 - tmp52));
		    c_re(inout[5 * iostride]) = tmp47 - tmp58;
		    c_re(inout[iostride]) = tmp47 + tmp58;
		    c_im(inout[3 * iostride]) = tmp79 + tmp80;
		    c_im(inout[7 * iostride]) = tmp80 - tmp79;
	       }
	  }
     }
}


void CFourier::fftw_twiddle_9(fftw_complex *A, const fftw_complex *W, tNativeAcc iostride, tNativeAcc m, tNativeAcc dist)
{
	 tNativeAcc i;
     fftw_complex *inout;
     inout = A;
     for (i = m; i > 0; i = i - 1, inout = inout + dist, W = W + 8) {
	  fftw_real tmp1;
	  fftw_real tmp99;
	  fftw_real tmp52;
	  fftw_real tmp98;
	  fftw_real tmp105;
	  fftw_real tmp104;
	  fftw_real tmp12;
	  fftw_real tmp49;
	  fftw_real tmp47;
	  fftw_real tmp69;
	  fftw_real tmp86;
	  fftw_real tmp95;
	  fftw_real tmp74;
	  fftw_real tmp85;
	  fftw_real tmp30;
	  fftw_real tmp58;
	  fftw_real tmp82;
	  fftw_real tmp94;
	  fftw_real tmp63;
	  fftw_real tmp83;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp6;
	       fftw_real tmp50;
	       fftw_real tmp11;
	       fftw_real tmp51;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp1 = c_re(inout[0]);
	       tmp99 = c_im(inout[0]);
	       {
		    fftw_real tmp3;
		    fftw_real tmp5;
		    fftw_real tmp2;
		    fftw_real tmp4;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp3 = c_re(inout[3 * iostride]);
		    tmp5 = c_im(inout[3 * iostride]);
		    tmp2 = c_re(W[2]);
		    tmp4 = c_im(W[2]);
		    tmp6 = FFT_M(tmp2 , tmp3) - FFT_M(tmp4 , tmp5);
		    tmp50 = FFT_M(tmp4 , tmp3) + FFT_M(tmp2 , tmp5);
	       }
	       {
		    fftw_real tmp8;
		    fftw_real tmp10;
		    fftw_real tmp7;
		    fftw_real tmp9;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp8 = c_re(inout[6 * iostride]);
		    tmp10 = c_im(inout[6 * iostride]);
		    tmp7 = c_re(W[5]);
		    tmp9 = c_im(W[5]);
		    tmp11 = FFT_M(tmp7 , tmp8) - FFT_M(tmp9 , tmp10);
		    tmp51 = FFT_M(tmp9 , tmp8) + FFT_M(tmp7 , tmp10);
	       }
	       tmp52 = FFT_M(K866025403 , (tmp50 - tmp51));
	       tmp98 = tmp50 + tmp51;
	       tmp105 = tmp99 - FFT_M(K500000000 , tmp98);
	       tmp104 = FFT_M(K866025403 , (tmp11 - tmp6));
	       tmp12 = tmp6 + tmp11;
	       tmp49 = tmp1 - FFT_M(K500000000 , tmp12);
	  }
	  {
	       fftw_real tmp35;
	       fftw_real tmp71;
	       fftw_real tmp40;
	       fftw_real tmp66;
	       fftw_real tmp45;
	       fftw_real tmp67;
	       fftw_real tmp46;
	       fftw_real tmp72;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp32;
		    fftw_real tmp34;
		    fftw_real tmp31;
		    fftw_real tmp33;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp32 = c_re(inout[2 * iostride]);
		    tmp34 = c_im(inout[2 * iostride]);
		    tmp31 = c_re(W[1]);
		    tmp33 = c_im(W[1]);
		    tmp35 = FFT_M(tmp31 , tmp32) - FFT_M(tmp33 , tmp34);
		    tmp71 = FFT_M(tmp33 , tmp32) + FFT_M(tmp31 , tmp34);
	       }
	       {
		    fftw_real tmp37;
		    fftw_real tmp39;
		    fftw_real tmp36;
		    fftw_real tmp38;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp37 = c_re(inout[5 * iostride]);
		    tmp39 = c_im(inout[5 * iostride]);
		    tmp36 = c_re(W[4]);
		    tmp38 = c_im(W[4]);
		    tmp40 = FFT_M(tmp36 , tmp37) - FFT_M(tmp38 , tmp39);
		    tmp66 = FFT_M(tmp38 , tmp37) + FFT_M(tmp36 , tmp39);
	       }
	       {
		    fftw_real tmp42;
		    fftw_real tmp44;
		    fftw_real tmp41;
		    fftw_real tmp43;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp42 = c_re(inout[8 * iostride]);
		    tmp44 = c_im(inout[8 * iostride]);
		    tmp41 = c_re(W[7]);
		    tmp43 = c_im(W[7]);
		    tmp45 = FFT_M(tmp41 , tmp42) - FFT_M(tmp43 , tmp44);
		    tmp67 = FFT_M(tmp43 , tmp42) + FFT_M(tmp41 , tmp44);
	       }
	       tmp46 = tmp40 + tmp45;
	       tmp72 = tmp66 + tmp67;
	       {
		    fftw_real tmp65;
		    fftw_real tmp68;
		    fftw_real tmp70;
		    fftw_real tmp73;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp47 = tmp35 + tmp46;
		    tmp65 = tmp35 - FFT_M(K500000000 , tmp46);
		    tmp68 = FFT_M(K866025403 , (tmp66 - tmp67));
		    tmp69 = tmp65 + tmp68;
		    tmp86 = tmp65 - tmp68;
		    tmp95 = tmp71 + tmp72;
		    tmp70 = FFT_M(K866025403 , (tmp45 - tmp40));
		    tmp73 = tmp71 - FFT_M(K500000000 , tmp72);
		    tmp74 = tmp70 + tmp73;
		    tmp85 = tmp73 - tmp70;
	       }
	  }
	  {
	       fftw_real tmp18;
	       fftw_real tmp60;
	       fftw_real tmp23;
	       fftw_real tmp55;
	       fftw_real tmp28;
	       fftw_real tmp56;
	       fftw_real tmp29;
	       fftw_real tmp61;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp15;
		    fftw_real tmp17;
		    fftw_real tmp14;
		    fftw_real tmp16;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp15 = c_re(inout[iostride]);
		    tmp17 = c_im(inout[iostride]);
		    tmp14 = c_re(W[0]);
		    tmp16 = c_im(W[0]);
		    tmp18 = FFT_M(tmp14 , tmp15) - FFT_M(tmp16 , tmp17);
		    tmp60 = FFT_M(tmp16 , tmp15) + FFT_M(tmp14 , tmp17);
	       }
	       {
		    fftw_real tmp20;
		    fftw_real tmp22;
		    fftw_real tmp19;
		    fftw_real tmp21;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp20 = c_re(inout[4 * iostride]);
		    tmp22 = c_im(inout[4 * iostride]);
		    tmp19 = c_re(W[3]);
		    tmp21 = c_im(W[3]);
		    tmp23 = FFT_M(tmp19 , tmp20) - FFT_M(tmp21 , tmp22);
		    tmp55 = FFT_M(tmp21 , tmp20) + FFT_M(tmp19 , tmp22);
	       }
	       {
		    fftw_real tmp25;
		    fftw_real tmp27;
		    fftw_real tmp24;
		    fftw_real tmp26;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp25 = c_re(inout[7 * iostride]);
		    tmp27 = c_im(inout[7 * iostride]);
		    tmp24 = c_re(W[6]);
		    tmp26 = c_im(W[6]);
		    tmp28 = FFT_M(tmp24 , tmp25) - FFT_M(tmp26 , tmp27);
		    tmp56 = FFT_M(tmp26 , tmp25) + FFT_M(tmp24 , tmp27);
	       }
	       tmp29 = tmp23 + tmp28;
	       tmp61 = tmp55 + tmp56;
	       {
		    fftw_real tmp54;
		    fftw_real tmp57;
		    fftw_real tmp59;
		    fftw_real tmp62;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp30 = tmp18 + tmp29;
		    tmp54 = tmp18 - FFT_M(K500000000 , tmp29);
		    tmp57 = FFT_M(K866025403 , (tmp55 - tmp56));
		    tmp58 = tmp54 + tmp57;
		    tmp82 = tmp54 - tmp57;
		    tmp94 = tmp60 + tmp61;
		    tmp59 = FFT_M(K866025403 , (tmp28 - tmp23));
		    tmp62 = tmp60 - FFT_M(K500000000 , tmp61);
		    tmp63 = tmp59 + tmp62;
		    tmp83 = tmp62 - tmp59;
	       }
	  }
	  {
	       fftw_real tmp96;
	       fftw_real tmp13;
	       fftw_real tmp48;
	       fftw_real tmp93;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp96 = FFT_M(K866025403 , (tmp94 - tmp95));
	       tmp13 = tmp1 + tmp12;
	       tmp48 = tmp30 + tmp47;
	       tmp93 = tmp13 - FFT_M(K500000000 , tmp48);
	       c_re(inout[0]) = tmp13 + tmp48;
	       c_re(inout[3 * iostride]) = tmp93 + tmp96;
	       c_re(inout[6 * iostride]) = tmp93 - tmp96;
	  }
	  {
	       fftw_real tmp101;
	       fftw_real tmp97;
	       fftw_real tmp100;
	       fftw_real tmp102;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp101 = FFT_M(K866025403 , (tmp47 - tmp30));
	       tmp97 = tmp94 + tmp95;
	       tmp100 = tmp98 + tmp99;
	       tmp102 = tmp100 - FFT_M(K500000000 , tmp97);
	       c_im(inout[0]) = tmp97 + tmp100;
	       c_im(inout[6 * iostride]) = tmp102 - tmp101;
	       c_im(inout[3 * iostride]) = tmp101 + tmp102;
	  }
	  {
	       fftw_real tmp53;
	       fftw_real tmp106;
	       fftw_real tmp76;
	       fftw_real tmp107;
	       fftw_real tmp80;
	       fftw_real tmp103;
	       fftw_real tmp77;
	       fftw_real tmp108;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp53 = tmp49 + tmp52;
	       tmp106 = tmp104 + tmp105;
	       {
		    fftw_real tmp64;
		    fftw_real tmp75;
		    fftw_real tmp78;
		    fftw_real tmp79;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp64 = FFT_M(K766044443 , tmp58) + FFT_M(K642787609 , tmp63);
		    tmp75 = FFT_M(K173648177 , tmp69) + FFT_M(K984807753 , tmp74);
		    tmp76 = tmp64 + tmp75;
		    tmp107 = FFT_M(K866025403 , (tmp75 - tmp64));
		    tmp78 = FFT_M(K766044443 , tmp63) - FFT_M(K642787609 , tmp58);
		    tmp79 = FFT_M(K173648177 , tmp74) - FFT_M(K984807753 , tmp69);
		    tmp80 = FFT_M(K866025403 , (tmp78 - tmp79));
		    tmp103 = tmp78 + tmp79;
	       }
	       c_re(inout[iostride]) = tmp53 + tmp76;
	       tmp77 = tmp53 - FFT_M(K500000000 , tmp76);
	       c_re(inout[7 * iostride]) = tmp77 - tmp80;
	       c_re(inout[4 * iostride]) = tmp77 + tmp80;
	       c_im(inout[iostride]) = tmp103 + tmp106;
	       tmp108 = tmp106 - FFT_M(K500000000 , tmp103);
	       c_im(inout[4 * iostride]) = tmp107 + tmp108;
	       c_im(inout[7 * iostride]) = tmp108 - tmp107;
	  }
	  {
	       fftw_real tmp81;
	       fftw_real tmp110;
	       fftw_real tmp88;
	       fftw_real tmp111;
	       fftw_real tmp92;
	       fftw_real tmp109;
	       fftw_real tmp89;
	       fftw_real tmp112;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp81 = tmp49 - tmp52;
	       tmp110 = tmp105 - tmp104;
	       {
		    fftw_real tmp84;
		    fftw_real tmp87;
		    fftw_real tmp90;
		    fftw_real tmp91;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp84 = FFT_M(K173648177 , tmp82) + FFT_M(K984807753 , tmp83);
		    tmp87 = FFT_M(K342020143 , tmp85) - FFT_M(K939692620 , tmp86);
		    tmp88 = tmp84 + tmp87;
		    tmp111 = FFT_M(K866025403 , (tmp87 - tmp84));
		    tmp90 = FFT_M(K173648177 , tmp83) - FFT_M(K984807753 , tmp82);
		    tmp91 = FFT_M(K342020143 , tmp86) + FFT_M(K939692620 , tmp85);
		    tmp92 = FFT_M(K866025403 , (tmp90 + tmp91));
		    tmp109 = tmp90 - tmp91;
	       }
	       c_re(inout[2 * iostride]) = tmp81 + tmp88;
	       tmp89 = tmp81 - FFT_M(K500000000 , tmp88);
	       c_re(inout[8 * iostride]) = tmp89 - tmp92;
	       c_re(inout[5 * iostride]) = tmp89 + tmp92;
	       c_im(inout[2 * iostride]) = tmp109 + tmp110;
	       tmp112 = tmp110 - FFT_M(K500000000 , tmp109);
	       c_im(inout[5 * iostride]) = tmp111 + tmp112;
	       c_im(inout[8 * iostride]) = tmp112 - tmp111;
	  }
     }

}

void CFourier::fftw_twiddle_16(fftw_complex *A, const fftw_complex *W, tNativeAcc iostride, tNativeAcc m, tNativeAcc dist)
{
	 tNativeAcc i;
     fftw_complex *inout;
     inout = A;
     for (i = m; i > 0; i = i - 1, inout = inout + dist, W = W + 15) {
	  fftw_real tmp7;
	  fftw_real tmp91;
	  fftw_real tmp180;
	  fftw_real tmp193;
	  fftw_real tmp18;
	  fftw_real tmp194;
	  fftw_real tmp94;
	  fftw_real tmp177;
	  fftw_real tmp77;
	  fftw_real tmp88;
	  fftw_real tmp161;
	  fftw_real tmp128;
	  fftw_real tmp144;
	  fftw_real tmp162;
	  fftw_real tmp163;
	  fftw_real tmp164;
	  fftw_real tmp123;
	  fftw_real tmp143;
	  fftw_real tmp30;
	  fftw_real tmp152;
	  fftw_real tmp100;
	  fftw_real tmp136;
	  fftw_real tmp41;
	  fftw_real tmp153;
	  fftw_real tmp105;
	  fftw_real tmp137;
	  fftw_real tmp54;
	  fftw_real tmp65;
	  fftw_real tmp156;
	  fftw_real tmp117;
	  fftw_real tmp141;
	  fftw_real tmp157;
	  fftw_real tmp158;
	  fftw_real tmp159;
	  fftw_real tmp112;
	  fftw_real tmp140;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp1;
	       fftw_real tmp179;
	       fftw_real tmp6;
	       fftw_real tmp178;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp1 = c_re(inout[0]);
	       tmp179 = c_im(inout[0]);
	       {
		    fftw_real tmp3;
		    fftw_real tmp5;
		    fftw_real tmp2;
		    fftw_real tmp4;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp3 = c_re(inout[8 * iostride]);
		    tmp5 = c_im(inout[8 * iostride]);
		    tmp2 = c_re(W[7]);
		    tmp4 = c_im(W[7]);
		    tmp6 = FFT_M(tmp2 , tmp3) - FFT_M(tmp4 , tmp5);
		    tmp178 = FFT_M(tmp4 , tmp3) + FFT_M(tmp2 , tmp5);
	       }
	       tmp7 = tmp1 + tmp6;
	       tmp91 = tmp1 - tmp6;
	       tmp180 = tmp178 + tmp179;
	       tmp193 = tmp179 - tmp178;
	  }
	  {
	       fftw_real tmp12;
	       fftw_real tmp92;
	       fftw_real tmp17;
	       fftw_real tmp93;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp9;
		    fftw_real tmp11;
		    fftw_real tmp8;
		    fftw_real tmp10;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp9 = c_re(inout[4 * iostride]);
		    tmp11 = c_im(inout[4 * iostride]);
		    tmp8 = c_re(W[3]);
		    tmp10 = c_im(W[3]);
		    tmp12 = FFT_M(tmp8 , tmp9) - FFT_M(tmp10 , tmp11);
		    tmp92 = FFT_M(tmp10 , tmp9) + FFT_M(tmp8 , tmp11);
	       }
	       {
		    fftw_real tmp14;
		    fftw_real tmp16;
		    fftw_real tmp13;
		    fftw_real tmp15;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp14 = c_re(inout[12 * iostride]);
		    tmp16 = c_im(inout[12 * iostride]);
		    tmp13 = c_re(W[11]);
		    tmp15 = c_im(W[11]);
		    tmp17 = FFT_M(tmp13 , tmp14) - FFT_M(tmp15 , tmp16);
		    tmp93 = FFT_M(tmp15 , tmp14) + FFT_M(tmp13 , tmp16);
	       }
	       tmp18 = tmp12 + tmp17;
	       tmp194 = tmp12 - tmp17;
	       tmp94 = tmp92 - tmp93;
	       tmp177 = tmp92 + tmp93;
	  }
	  {
	       fftw_real tmp71;
	       fftw_real tmp124;
	       fftw_real tmp87;
	       fftw_real tmp121;
	       fftw_real tmp76;
	       fftw_real tmp125;
	       fftw_real tmp82;
	       fftw_real tmp120;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp68;
		    fftw_real tmp70;
		    fftw_real tmp67;
		    fftw_real tmp69;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp68 = c_re(inout[15 * iostride]);
		    tmp70 = c_im(inout[15 * iostride]);
		    tmp67 = c_re(W[14]);
		    tmp69 = c_im(W[14]);
		    tmp71 = FFT_M(tmp67 , tmp68) - FFT_M(tmp69 , tmp70);
		    tmp124 = FFT_M(tmp69 , tmp68) + FFT_M(tmp67 , tmp70);
	       }
	       {
		    fftw_real tmp84;
		    fftw_real tmp86;
		    fftw_real tmp83;
		    fftw_real tmp85;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp84 = c_re(inout[11 * iostride]);
		    tmp86 = c_im(inout[11 * iostride]);
		    tmp83 = c_re(W[10]);
		    tmp85 = c_im(W[10]);
		    tmp87 = FFT_M(tmp83 , tmp84) - FFT_M(tmp85 , tmp86);
		    tmp121 = FFT_M(tmp85 , tmp84) + FFT_M(tmp83 , tmp86);
	       }
	       {
		    fftw_real tmp73;
		    fftw_real tmp75;
		    fftw_real tmp72;
		    fftw_real tmp74;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp73 = c_re(inout[7 * iostride]);
		    tmp75 = c_im(inout[7 * iostride]);
		    tmp72 = c_re(W[6]);
		    tmp74 = c_im(W[6]);
		    tmp76 = FFT_M(tmp72 , tmp73) - FFT_M(tmp74 , tmp75);
		    tmp125 = FFT_M(tmp74 , tmp73) + FFT_M(tmp72 , tmp75);
	       }
	       {
		    fftw_real tmp79;
		    fftw_real tmp81;
		    fftw_real tmp78;
		    fftw_real tmp80;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp79 = c_re(inout[3 * iostride]);
		    tmp81 = c_im(inout[3 * iostride]);
		    tmp78 = c_re(W[2]);
		    tmp80 = c_im(W[2]);
		    tmp82 = FFT_M(tmp78 , tmp79) - FFT_M(tmp80 , tmp81);
		    tmp120 = FFT_M(tmp80 , tmp79) + FFT_M(tmp78 , tmp81);
	       }
	       {
		    fftw_real tmp126;
		    fftw_real tmp127;
		    fftw_real tmp119;
		    fftw_real tmp122;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp77 = tmp71 + tmp76;
		    tmp88 = tmp82 + tmp87;
		    tmp161 = tmp77 - tmp88;
		    tmp126 = tmp124 - tmp125;
		    tmp127 = tmp82 - tmp87;
		    tmp128 = tmp126 + tmp127;
		    tmp144 = tmp126 - tmp127;
		    tmp162 = tmp124 + tmp125;
		    tmp163 = tmp120 + tmp121;
		    tmp164 = tmp162 - tmp163;
		    tmp119 = tmp71 - tmp76;
		    tmp122 = tmp120 - tmp121;
		    tmp123 = tmp119 - tmp122;
		    tmp143 = tmp119 + tmp122;
	       }
	  }
	  {
	       fftw_real tmp24;
	       fftw_real tmp96;
	       fftw_real tmp29;
	       fftw_real tmp97;
	       fftw_real tmp98;
	       fftw_real tmp99;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp21;
		    fftw_real tmp23;
		    fftw_real tmp20;
		    fftw_real tmp22;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp21 = c_re(inout[2 * iostride]);
		    tmp23 = c_im(inout[2 * iostride]);
		    tmp20 = c_re(W[1]);
		    tmp22 = c_im(W[1]);
		    tmp24 = FFT_M(tmp20 , tmp21) - FFT_M(tmp22 , tmp23);
		    tmp96 = FFT_M(tmp22 , tmp21) + FFT_M(tmp20 , tmp23);
	       }
	       {
		    fftw_real tmp26;
		    fftw_real tmp28;
		    fftw_real tmp25;
		    fftw_real tmp27;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp26 = c_re(inout[10 * iostride]);
		    tmp28 = c_im(inout[10 * iostride]);
		    tmp25 = c_re(W[9]);
		    tmp27 = c_im(W[9]);
		    tmp29 = FFT_M(tmp25 , tmp26) - FFT_M(tmp27 , tmp28);
		    tmp97 = FFT_M(tmp27 , tmp26) + FFT_M(tmp25 , tmp28);
	       }
	       tmp30 = tmp24 + tmp29;
	       tmp152 = tmp96 + tmp97;
	       tmp98 = tmp96 - tmp97;
	       tmp99 = tmp24 - tmp29;
	       tmp100 = tmp98 - tmp99;
	       tmp136 = tmp99 + tmp98;
	  }
	  {
	       fftw_real tmp35;
	       fftw_real tmp102;
	       fftw_real tmp40;
	       fftw_real tmp103;
	       fftw_real tmp101;
	       fftw_real tmp104;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp32;
		    fftw_real tmp34;
		    fftw_real tmp31;
		    fftw_real tmp33;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp32 = c_re(inout[14 * iostride]);
		    tmp34 = c_im(inout[14 * iostride]);
		    tmp31 = c_re(W[13]);
		    tmp33 = c_im(W[13]);
		    tmp35 = FFT_M(tmp31 , tmp32) - FFT_M(tmp33 , tmp34);
		    tmp102 = FFT_M(tmp33 , tmp32) + FFT_M(tmp31 , tmp34);
	       }
	       {
		    fftw_real tmp37;
		    fftw_real tmp39;
		    fftw_real tmp36;
		    fftw_real tmp38;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp37 = c_re(inout[6 * iostride]);
		    tmp39 = c_im(inout[6 * iostride]);
		    tmp36 = c_re(W[5]);
		    tmp38 = c_im(W[5]);
		    tmp40 = FFT_M(tmp36 , tmp37) - FFT_M(tmp38 , tmp39);
		    tmp103 = FFT_M(tmp38 , tmp37) + FFT_M(tmp36 , tmp39);
	       }
	       tmp41 = tmp35 + tmp40;
	       tmp153 = tmp102 + tmp103;
	       tmp101 = tmp35 - tmp40;
	       tmp104 = tmp102 - tmp103;
	       tmp105 = tmp101 + tmp104;
	       tmp137 = tmp101 - tmp104;
	  }
	  {
	       fftw_real tmp48;
	       fftw_real tmp108;
	       fftw_real tmp64;
	       fftw_real tmp115;
	       fftw_real tmp53;
	       fftw_real tmp109;
	       fftw_real tmp59;
	       fftw_real tmp114;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp45;
		    fftw_real tmp47;
		    fftw_real tmp44;
		    fftw_real tmp46;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp45 = c_re(inout[iostride]);
		    tmp47 = c_im(inout[iostride]);
		    tmp44 = c_re(W[0]);
		    tmp46 = c_im(W[0]);
		    tmp48 = FFT_M(tmp44 , tmp45) - FFT_M(tmp46 , tmp47);
		    tmp108 = FFT_M(tmp46 , tmp45) + FFT_M(tmp44 , tmp47);
	       }
	       {
		    fftw_real tmp61;
		    fftw_real tmp63;
		    fftw_real tmp60;
		    fftw_real tmp62;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp61 = c_re(inout[13 * iostride]);
		    tmp63 = c_im(inout[13 * iostride]);
		    tmp60 = c_re(W[12]);
		    tmp62 = c_im(W[12]);
		    tmp64 = FFT_M(tmp60 , tmp61) - FFT_M(tmp62 , tmp63);
		    tmp115 = FFT_M(tmp62 , tmp61) + FFT_M(tmp60 , tmp63);
	       }
	       {
		    fftw_real tmp50;
		    fftw_real tmp52;
		    fftw_real tmp49;
		    fftw_real tmp51;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp50 = c_re(inout[9 * iostride]);
		    tmp52 = c_im(inout[9 * iostride]);
		    tmp49 = c_re(W[8]);
		    tmp51 = c_im(W[8]);
		    tmp53 = FFT_M(tmp49 , tmp50) - FFT_M(tmp51 , tmp52);
		    tmp109 = FFT_M(tmp51 , tmp50) + FFT_M(tmp49 , tmp52);
	       }
	       {
		    fftw_real tmp56;
		    fftw_real tmp58;
		    fftw_real tmp55;
		    fftw_real tmp57;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp56 = c_re(inout[5 * iostride]);
		    tmp58 = c_im(inout[5 * iostride]);
		    tmp55 = c_re(W[4]);
		    tmp57 = c_im(W[4]);
		    tmp59 = FFT_M(tmp55 , tmp56) - FFT_M(tmp57 , tmp58);
		    tmp114 = FFT_M(tmp57 , tmp56) + FFT_M(tmp55 , tmp58);
	       }
	       {
		    fftw_real tmp113;
		    fftw_real tmp116;
		    fftw_real tmp110;
		    fftw_real tmp111;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp54 = tmp48 + tmp53;
		    tmp65 = tmp59 + tmp64;
		    tmp156 = tmp54 - tmp65;
		    tmp113 = tmp48 - tmp53;
		    tmp116 = tmp114 - tmp115;
		    tmp117 = tmp113 - tmp116;
		    tmp141 = tmp113 + tmp116;
		    tmp157 = tmp108 + tmp109;
		    tmp158 = tmp114 + tmp115;
		    tmp159 = tmp157 - tmp158;
		    tmp110 = tmp108 - tmp109;
		    tmp111 = tmp59 - tmp64;
		    tmp112 = tmp110 + tmp111;
		    tmp140 = tmp110 - tmp111;
	       }
	  }
	  {
	       fftw_real tmp107;
	       fftw_real tmp131;
	       fftw_real tmp202;
	       fftw_real tmp204;
	       fftw_real tmp130;
	       fftw_real tmp203;
	       fftw_real tmp134;
	       fftw_real tmp199;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp95;
		    fftw_real tmp106;
		    fftw_real tmp200;
		    fftw_real tmp201;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp95 = tmp91 - tmp94;
		    tmp106 = FFT_M(K707106781 , (tmp100 - tmp105));
		    tmp107 = tmp95 + tmp106;
		    tmp131 = tmp95 - tmp106;
		    tmp200 = FFT_M(K707106781 , (tmp137 - tmp136));
		    tmp201 = tmp194 + tmp193;
		    tmp202 = tmp200 + tmp201;
		    tmp204 = tmp201 - tmp200;
	       }
	       {
		    fftw_real tmp118;
		    fftw_real tmp129;
		    fftw_real tmp132;
		    fftw_real tmp133;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp118 = FFT_M(K923879532 , tmp112) + FFT_M(K382683432 , tmp117);
		    tmp129 = FFT_M(K382683432 , tmp123) - FFT_M(K923879532 , tmp128);
		    tmp130 = tmp118 + tmp129;
		    tmp203 = tmp129 - tmp118;
		    tmp132 = FFT_M(K382683432 , tmp112) - FFT_M(K923879532 , tmp117);
		    tmp133 = FFT_M(K382683432 , tmp128) + FFT_M(K923879532 , tmp123);
		    tmp134 = tmp132 - tmp133;
		    tmp199 = tmp132 + tmp133;
	       }
	       c_re(inout[11 * iostride]) = tmp107 - tmp130;
	       c_re(inout[3 * iostride]) = tmp107 + tmp130;
	       c_re(inout[15 * iostride]) = tmp131 - tmp134;
	       c_re(inout[7 * iostride]) = tmp131 + tmp134;
	       c_im(inout[3 * iostride]) = tmp199 + tmp202;
	       c_im(inout[11 * iostride]) = tmp202 - tmp199;
	       c_im(inout[7 * iostride]) = tmp203 + tmp204;
	       c_im(inout[15 * iostride]) = tmp204 - tmp203;
	  }
	  {
	       fftw_real tmp139;
	       fftw_real tmp147;
	       fftw_real tmp196;
	       fftw_real tmp198;
	       fftw_real tmp146;
	       fftw_real tmp197;
	       fftw_real tmp150;
	       fftw_real tmp191;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp135;
		    fftw_real tmp138;
		    fftw_real tmp192;
		    fftw_real tmp195;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp135 = tmp91 + tmp94;
		    tmp138 = FFT_M(K707106781 , (tmp136 + tmp137));
		    tmp139 = tmp135 + tmp138;
		    tmp147 = tmp135 - tmp138;
		    tmp192 = FFT_M(K707106781 , (tmp100 + tmp105));
		    tmp195 = tmp193 - tmp194;
		    tmp196 = tmp192 + tmp195;
		    tmp198 = tmp195 - tmp192;
	       }
	       {
		    fftw_real tmp142;
		    fftw_real tmp145;
		    fftw_real tmp148;
		    fftw_real tmp149;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp142 = FFT_M(K382683432 , tmp140) + FFT_M(K923879532 , tmp141);
		    tmp145 = FFT_M(K923879532 , tmp143) - FFT_M(K382683432 , tmp144);
		    tmp146 = tmp142 + tmp145;
		    tmp197 = tmp145 - tmp142;
		    tmp148 = FFT_M(K923879532 , tmp140) - FFT_M(K382683432 , tmp141);
		    tmp149 = FFT_M(K923879532 , tmp144) + FFT_M(K382683432 , tmp143);
		    tmp150 = tmp148 - tmp149;
		    tmp191 = tmp148 + tmp149;
	       }
	       c_re(inout[9 * iostride]) = tmp139 - tmp146;
	       c_re(inout[iostride]) = tmp139 + tmp146;
	       c_re(inout[13 * iostride]) = tmp147 - tmp150;
	       c_re(inout[5 * iostride]) = tmp147 + tmp150;
	       c_im(inout[iostride]) = tmp191 + tmp196;
	       c_im(inout[9 * iostride]) = tmp196 - tmp191;
	       c_im(inout[5 * iostride]) = tmp197 + tmp198;
	       c_im(inout[13 * iostride]) = tmp198 - tmp197;
	  }
	  {
	       fftw_real tmp155;
	       fftw_real tmp167;
	       fftw_real tmp188;
	       fftw_real tmp190;
	       fftw_real tmp166;
	       fftw_real tmp189;
	       fftw_real tmp170;
	       fftw_real tmp185;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp151;
		    fftw_real tmp154;
		    fftw_real tmp186;
		    fftw_real tmp187;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp151 = tmp7 - tmp18;
		    tmp154 = tmp152 - tmp153;
		    tmp155 = tmp151 + tmp154;
		    tmp167 = tmp151 - tmp154;
		    tmp186 = tmp41 - tmp30;
		    tmp187 = tmp180 - tmp177;
		    tmp188 = tmp186 + tmp187;
		    tmp190 = tmp187 - tmp186;
	       }
	       {
		    fftw_real tmp160;
		    fftw_real tmp165;
		    fftw_real tmp168;
		    fftw_real tmp169;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp160 = tmp156 + tmp159;
		    tmp165 = tmp161 - tmp164;
		    tmp166 = FFT_M(K707106781 , (tmp160 + tmp165));
		    tmp189 = FFT_M(K707106781 , (tmp165 - tmp160));
		    tmp168 = tmp159 - tmp156;
		    tmp169 = tmp161 + tmp164;
		    tmp170 = FFT_M(K707106781 , (tmp168 - tmp169));
		    tmp185 = FFT_M(K707106781 , (tmp168 + tmp169));
	       }
	       c_re(inout[10 * iostride]) = tmp155 - tmp166;
	       c_re(inout[2 * iostride]) = tmp155 + tmp166;
	       c_re(inout[14 * iostride]) = tmp167 - tmp170;
	       c_re(inout[6 * iostride]) = tmp167 + tmp170;
	       c_im(inout[2 * iostride]) = tmp185 + tmp188;
	       c_im(inout[10 * iostride]) = tmp188 - tmp185;
	       c_im(inout[6 * iostride]) = tmp189 + tmp190;
	       c_im(inout[14 * iostride]) = tmp190 - tmp189;
	  }
	  {
	       fftw_real tmp43;
	       fftw_real tmp171;
	       fftw_real tmp182;
	       fftw_real tmp184;
	       fftw_real tmp90;
	       fftw_real tmp183;
	       fftw_real tmp174;
	       fftw_real tmp175;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp19;
		    fftw_real tmp42;
		    fftw_real tmp176;
		    fftw_real tmp181;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp19 = tmp7 + tmp18;
		    tmp42 = tmp30 + tmp41;
		    tmp43 = tmp19 + tmp42;
		    tmp171 = tmp19 - tmp42;
		    tmp176 = tmp152 + tmp153;
		    tmp181 = tmp177 + tmp180;
		    tmp182 = tmp176 + tmp181;
		    tmp184 = tmp181 - tmp176;
	       }
	       {
		    fftw_real tmp66;
		    fftw_real tmp89;
		    fftw_real tmp172;
		    fftw_real tmp173;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp66 = tmp54 + tmp65;
		    tmp89 = tmp77 + tmp88;
		    tmp90 = tmp66 + tmp89;
		    tmp183 = tmp89 - tmp66;
		    tmp172 = tmp157 + tmp158;
		    tmp173 = tmp162 + tmp163;
		    tmp174 = tmp172 - tmp173;
		    tmp175 = tmp172 + tmp173;
	       }
	       c_re(inout[8 * iostride]) = tmp43 - tmp90;
	       c_re(inout[0]) = tmp43 + tmp90;
	       c_re(inout[12 * iostride]) = tmp171 - tmp174;
	       c_re(inout[4 * iostride]) = tmp171 + tmp174;
	       c_im(inout[0]) = tmp175 + tmp182;
	       c_im(inout[8 * iostride]) = tmp182 - tmp175;
	       c_im(inout[4 * iostride]) = tmp183 + tmp184;
	       c_im(inout[12 * iostride]) = tmp184 - tmp183;
	  }
     }
}


void CFourier::fftw_twiddle_4(fftw_complex *A, const fftw_complex *W, tNativeAcc iostride, tNativeAcc m, tNativeAcc dist)
{

	 tNativeAcc i;
     fftw_complex *inout;
     inout = A;
     for (i = m; i > 0; i = i - 1, inout = inout + dist, W = W + 3) {
	  fftw_real tmp1;
	  fftw_real tmp25;
	  fftw_real tmp6;
	  fftw_real tmp24;
	  fftw_real tmp12;
	  fftw_real tmp20;
	  fftw_real tmp17;
	  fftw_real tmp21;
	  ASSERT_ALIGNED_DOUBLE;
	  tmp1 = c_re(inout[0]);
	  tmp25 = c_im(inout[0]);
	  {
	       fftw_real tmp3;
	       fftw_real tmp5;
	       fftw_real tmp2;
	       fftw_real tmp4;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp3 = c_re(inout[2 * iostride]);
	       tmp5 = c_im(inout[2 * iostride]);
	       tmp2 = c_re(W[1]);
	       tmp4 = c_im(W[1]);
	       tmp6 = FFT_M(tmp2 , tmp3) - FFT_M(tmp4 , tmp5);
	       tmp24 = FFT_M(tmp4 , tmp3) + FFT_M(tmp2 , tmp5);
	  }
	  {
	       fftw_real tmp9;
	       fftw_real tmp11;
	       fftw_real tmp8;
	       fftw_real tmp10;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp9 = c_re(inout[iostride]);
	       tmp11 = c_im(inout[iostride]);
	       tmp8 = c_re(W[0]);
	       tmp10 = c_im(W[0]);
	       tmp12 = FFT_M(tmp8 , tmp9) - FFT_M(tmp10 , tmp11);
	       tmp20 = FFT_M(tmp10 , tmp9) + FFT_M(tmp8 , tmp11);
	  }
	  {
	       fftw_real tmp14;
	       fftw_real tmp16;
	       fftw_real tmp13;
	       fftw_real tmp15;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp14 = c_re(inout[3 * iostride]);
	       tmp16 = c_im(inout[3 * iostride]);
	       tmp13 = c_re(W[2]);
	       tmp15 = c_im(W[2]);
	       tmp17 = FFT_M(tmp13 , tmp14) - FFT_M(tmp15 , tmp16);
	       tmp21 = FFT_M(tmp15 , tmp14) + FFT_M(tmp13 , tmp16);
	  }
	  {
	       fftw_real tmp7;
	       fftw_real tmp18;
	       fftw_real tmp27;
	       fftw_real tmp28;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp7 = tmp1 + tmp6;
	       tmp18 = tmp12 + tmp17;
	       c_re(inout[2 * iostride]) = tmp7 - tmp18;
	       c_re(inout[0]) = tmp7 + tmp18;
	       tmp27 = tmp25 - tmp24;
	       tmp28 = tmp12 - tmp17;
	       c_im(inout[iostride]) = tmp27 - tmp28;
	       c_im(inout[3 * iostride]) = tmp28 + tmp27;
	  }
	  {
	       fftw_real tmp23;
	       fftw_real tmp26;
	       fftw_real tmp19;
	       fftw_real tmp22;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp23 = tmp20 + tmp21;
	       tmp26 = tmp24 + tmp25;
	       c_im(inout[0]) = tmp23 + tmp26;
	       c_im(inout[2 * iostride]) = tmp26 - tmp23;
	       tmp19 = tmp1 - tmp6;
	       tmp22 = tmp20 - tmp21;
	       c_re(inout[3 * iostride]) = tmp19 - tmp22;
	       c_re(inout[iostride]) = tmp19 + tmp22;
	  }
     }
}

void CFourier::fftwi_no_twiddle_16(const fftw_complex *input, fftw_complex *output, tNativeAcc istride, tNativeAcc ostride)
{
	 fftw_real tmp7;
     fftw_real tmp129;
     fftw_real tmp38;
     fftw_real tmp115;
     fftw_real tmp49;
     fftw_real tmp95;
     fftw_real tmp83;
     fftw_real tmp105;
     fftw_real tmp29;
     fftw_real tmp123;
     fftw_real tmp73;
     fftw_real tmp101;
     fftw_real tmp78;
     fftw_real tmp102;
     fftw_real tmp126;
     fftw_real tmp141;
     fftw_real tmp14;
     fftw_real tmp116;
     fftw_real tmp45;
     fftw_real tmp130;
     fftw_real tmp52;
     fftw_real tmp84;
     fftw_real tmp55;
     fftw_real tmp85;
     fftw_real tmp22;
     fftw_real tmp118;
     fftw_real tmp62;
     fftw_real tmp98;
     fftw_real tmp67;
     fftw_real tmp99;
     fftw_real tmp121;
     fftw_real tmp140;
     ASSERT_ALIGNED_DOUBLE;
     {
	  fftw_real tmp3;
	  fftw_real tmp81;
	  fftw_real tmp34;
	  fftw_real tmp48;
	  fftw_real tmp6;
	  fftw_real tmp47;
	  fftw_real tmp37;
	  fftw_real tmp82;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp1;
	       fftw_real tmp2;
	       fftw_real tmp32;
	       fftw_real tmp33;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp1 = c_re(input[0]);
	       tmp2 = c_re(input[8 * istride]);
	       tmp3 = (tmp1 + tmp2);
	       tmp81 = (tmp1 - tmp2);
	       tmp32 = c_im(input[0]);
	       tmp33 = c_im(input[8 * istride]);
	       tmp34 = (tmp32 + tmp33);
	       tmp48 = (tmp32 - tmp33);
	  }
	  {
	       fftw_real tmp4;
	       fftw_real tmp5;
	       fftw_real tmp35;
	       fftw_real tmp36;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp4 = c_re(input[4 * istride]);
	       tmp5 = c_re(input[12 * istride]);
	       tmp6 = (tmp4 + tmp5);
	       tmp47 = (tmp4 - tmp5);
	       tmp35 = c_im(input[4 * istride]);
	       tmp36 = c_im(input[12 * istride]);
	       tmp37 = (tmp35 + tmp36);
	       tmp82 = (tmp35 - tmp36);
	  }
	  tmp7 = tmp3 + tmp6;
	  tmp129 = tmp3 - tmp6;
	  tmp38 = tmp34 + tmp37;
	  tmp115 = tmp34 - tmp37;
	  tmp49 = tmp47 + tmp48;
	  tmp95 = tmp48 - tmp47;
	  tmp83 = tmp81 - tmp82;
	  tmp105 = tmp81 + tmp82;
     }
     {
	  fftw_real tmp25;
	  fftw_real tmp74;
	  fftw_real tmp72;
	  fftw_real tmp124;
	  fftw_real tmp28;
	  fftw_real tmp69;
	  fftw_real tmp77;
	  fftw_real tmp125;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp23;
	       fftw_real tmp24;
	       fftw_real tmp70;
	       fftw_real tmp71;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp23 = c_re(input[15 * istride]);
	       tmp24 = c_re(input[7 * istride]);
	       tmp25 = (tmp23 + tmp24);
	       tmp74 = (tmp23 - tmp24);
	       tmp70 = c_im(input[15 * istride]);
	       tmp71 = c_im(input[7 * istride]);
	       tmp72 = (tmp70 - tmp71);
	       tmp124 = (tmp70 + tmp71);
	  }
	  {
	       fftw_real tmp26;
	       fftw_real tmp27;
	       fftw_real tmp75;
	       fftw_real tmp76;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp26 = c_re(input[3 * istride]);
	       tmp27 = c_re(input[11 * istride]);
	       tmp28 = (tmp26 + tmp27);
	       tmp69 = (tmp26 - tmp27);
	       tmp75 = c_im(input[3 * istride]);
	       tmp76 = c_im(input[11 * istride]);
	       tmp77 = (tmp75 - tmp76);
	       tmp125 = (tmp75 + tmp76);
	  }
	  tmp29 = tmp25 + tmp28;
	  tmp123 = tmp25 - tmp28;
	  tmp73 = tmp69 + tmp72;
	  tmp101 = tmp72 - tmp69;
	  tmp78 = tmp74 - tmp77;
	  tmp102 = tmp74 + tmp77;
	  tmp126 = tmp124 - tmp125;
	  tmp141 = tmp124 + tmp125;
     }
     {
	  fftw_real tmp10;
	  fftw_real tmp50;
	  fftw_real tmp41;
	  fftw_real tmp51;
	  fftw_real tmp13;
	  fftw_real tmp54;
	  fftw_real tmp44;
	  fftw_real tmp53;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp8;
	       fftw_real tmp9;
	       fftw_real tmp39;
	       fftw_real tmp40;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp8 = c_re(input[2 * istride]);
	       tmp9 = c_re(input[10 * istride]);
	       tmp10 = (tmp8 + tmp9);
	       tmp50 = (tmp8 - tmp9);
	       tmp39 = c_im(input[2 * istride]);
	       tmp40 = c_im(input[10 * istride]);
	       tmp41 = (tmp39 + tmp40);
	       tmp51 = (tmp39 - tmp40);
	  }
	  {
	       fftw_real tmp11;
	       fftw_real tmp12;
	       fftw_real tmp42;
	       fftw_real tmp43;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp11 = c_re(input[14 * istride]);
	       tmp12 = c_re(input[6 * istride]);
	       tmp13 = (tmp11 + tmp12);
	       tmp54 = (tmp11 - tmp12);
	       tmp42 = c_im(input[14 * istride]);
	       tmp43 = c_im(input[6 * istride]);
	       tmp44 = (tmp42 + tmp43);
	       tmp53 = (tmp42 - tmp43);
	  }
	  tmp14 = tmp10 + tmp13;
	  tmp116 = tmp10 - tmp13;
	  tmp45 = tmp41 + tmp44;
	  tmp130 = tmp44 - tmp41;
	  tmp52 = tmp50 + tmp51;
	  tmp84 = tmp50 - tmp51;
	  tmp55 = tmp53 - tmp54;
	  tmp85 = tmp54 + tmp53;
     }
     {
	  fftw_real tmp18;
	  fftw_real tmp63;
	  fftw_real tmp61;
	  fftw_real tmp119;
	  fftw_real tmp21;
	  fftw_real tmp58;
	  fftw_real tmp66;
	  fftw_real tmp120;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp16;
	       fftw_real tmp17;
	       fftw_real tmp59;
	       fftw_real tmp60;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp16 = c_re(input[istride]);
	       tmp17 = c_re(input[9 * istride]);
	       tmp18 = (tmp16 + tmp17);
	       tmp63 = (tmp16 - tmp17);
	       tmp59 = c_im(input[istride]);
	       tmp60 = c_im(input[9 * istride]);
	       tmp61 = (tmp59 - tmp60);
	       tmp119 = (tmp59 + tmp60);
	  }
	  {
	       fftw_real tmp19;
	       fftw_real tmp20;
	       fftw_real tmp64;
	       fftw_real tmp65;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp19 = c_re(input[5 * istride]);
	       tmp20 = c_re(input[13 * istride]);
	       tmp21 = (tmp19 + tmp20);
	       tmp58 = (tmp19 - tmp20);
	       tmp64 = c_im(input[5 * istride]);
	       tmp65 = c_im(input[13 * istride]);
	       tmp66 = (tmp64 - tmp65);
	       tmp120 = (tmp64 + tmp65);
	  }
	  tmp22 = tmp18 + tmp21;
	  tmp118 = tmp18 - tmp21;
	  tmp62 = tmp58 + tmp61;
	  tmp98 = tmp61 - tmp58;
	  tmp67 = tmp63 - tmp66;
	  tmp99 = tmp63 + tmp66;
	  tmp121 = tmp119 - tmp120;
	  tmp140 = tmp119 + tmp120;
     }
     {
	  fftw_real tmp15;
	  fftw_real tmp30;
	  fftw_real tmp31;
	  fftw_real tmp46;
	  ASSERT_ALIGNED_DOUBLE;
	  tmp15 = tmp7 + tmp14;
	  tmp30 = tmp22 + tmp29;
	  c_re(output[8 * ostride]) = tmp15 - tmp30;
	  c_re(output[0]) = tmp15 + tmp30;
	  tmp31 = tmp22 - tmp29;
	  tmp46 = tmp38 - tmp45;
	  c_im(output[4 * ostride]) = tmp31 + tmp46;
	  c_im(output[12 * ostride]) = tmp46 - tmp31;
     }
     {
	  fftw_real tmp139;
	  fftw_real tmp142;
	  fftw_real tmp143;
	  fftw_real tmp144;
	  ASSERT_ALIGNED_DOUBLE;
	  tmp139 = tmp38 + tmp45;
	  tmp142 = tmp140 + tmp141;
	  c_im(output[8 * ostride]) = tmp139 - tmp142;
	  c_im(output[0]) = tmp139 + tmp142;
	  tmp143 = tmp7 - tmp14;
	  tmp144 = tmp141 - tmp140;
	  c_re(output[12 * ostride]) = tmp143 - tmp144;
	  c_re(output[4 * ostride]) = tmp143 + tmp144;
     }
     {
	  fftw_real tmp117;
	  fftw_real tmp131;
	  fftw_real tmp128;
	  fftw_real tmp132;
	  fftw_real tmp122;
	  fftw_real tmp127;
	  ASSERT_ALIGNED_DOUBLE;
	  tmp117 = tmp115 - tmp116;
	  tmp131 = tmp129 + tmp130;
	  tmp122 = tmp118 - tmp121;
	  tmp127 = tmp123 + tmp126;
	  tmp128 = FFT_M(K707106781 , (tmp122 - tmp127));
	  tmp132 = FFT_M(K707106781 , (tmp122 + tmp127));
	  c_im(output[14 * ostride]) = tmp117 - tmp128;
	  c_im(output[6 * ostride]) = tmp117 + tmp128;
	  c_re(output[10 * ostride]) = tmp131 - tmp132;
	  c_re(output[2 * ostride]) = tmp131 + tmp132;
     }
     {
	  fftw_real tmp133;
	  fftw_real tmp137;
	  fftw_real tmp136;
	  fftw_real tmp138;
	  fftw_real tmp134;
	  fftw_real tmp135;
	  ASSERT_ALIGNED_DOUBLE;
	  tmp133 = tmp116 + tmp115;
	  tmp137 = tmp129 - tmp130;
	  tmp134 = tmp118 + tmp121;
	  tmp135 = tmp126 - tmp123;
	  tmp136 = FFT_M(K707106781 , (tmp134 + tmp135));
	  tmp138 = FFT_M(K707106781 , (tmp135 - tmp134));
	  c_im(output[10 * ostride]) = tmp133 - tmp136;
	  c_im(output[2 * ostride]) = tmp133 + tmp136;
	  c_re(output[14 * ostride]) = tmp137 - tmp138;
	  c_re(output[6 * ostride]) = tmp137 + tmp138;
     }
     {
	  fftw_real tmp57;
	  fftw_real tmp89;
	  fftw_real tmp92;
	  fftw_real tmp94;
	  fftw_real tmp87;
	  fftw_real tmp93;
	  fftw_real tmp80;
	  fftw_real tmp88;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp56;
	       fftw_real tmp90;
	       fftw_real tmp91;
	       fftw_real tmp86;
	       fftw_real tmp68;
	       fftw_real tmp79;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp56 = FFT_M(K707106781 , (tmp52 + tmp55));
	       tmp57 = tmp49 + tmp56;
	       tmp89 = tmp49 - tmp56;
	       tmp90 = FFT_M(K923879532 , tmp67) - FFT_M(K382683432 , tmp62);
	       tmp91 = FFT_M(K382683432 , tmp73) + FFT_M(K923879532 , tmp78);
	       tmp92 = tmp90 - tmp91;
	       tmp94 = tmp90 + tmp91;
	       tmp86 = FFT_M(K707106781 , (tmp84 + tmp85));
	       tmp87 = tmp83 - tmp86;
	       tmp93 = tmp83 + tmp86;
	       tmp68 = FFT_M(K923879532 , tmp62) + FFT_M(K382683432 , tmp67);
	       tmp79 = FFT_M(K923879532 , tmp73) - FFT_M(K382683432 , tmp78);
	       tmp80 = tmp68 + tmp79;
	       tmp88 = tmp79 - tmp68;
	  }
	  c_im(output[9 * ostride]) = tmp57 - tmp80;
	  c_im(output[ostride]) = tmp57 + tmp80;
	  c_re(output[13 * ostride]) = tmp87 - tmp88;
	  c_re(output[5 * ostride]) = tmp87 + tmp88;
	  c_im(output[13 * ostride]) = tmp89 - tmp92;
	  c_im(output[5 * ostride]) = tmp89 + tmp92;
	  c_re(output[9 * ostride]) = tmp93 - tmp94;
	  c_re(output[ostride]) = tmp93 + tmp94;
     }
     {
	  fftw_real tmp97;
	  fftw_real tmp109;
	  fftw_real tmp112;
	  fftw_real tmp114;
	  fftw_real tmp107;
	  fftw_real tmp113;
	  fftw_real tmp104;
	  fftw_real tmp108;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp96;
	       fftw_real tmp110;
	       fftw_real tmp111;
	       fftw_real tmp106;
	       fftw_real tmp100;
	       fftw_real tmp103;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp96 = FFT_M(K707106781 , (tmp84 - tmp85));
	       tmp97 = tmp95 + tmp96;
	       tmp109 = tmp95 - tmp96;
	       tmp110 = FFT_M(K382683432 , tmp99) - FFT_M(K923879532 , tmp98);
	       tmp111 = FFT_M(K923879532 , tmp101) + FFT_M(K382683432 , tmp102);
	       tmp112 = tmp110 - tmp111;
	       tmp114 = tmp110 + tmp111;
	       tmp106 = FFT_M(K707106781 , (tmp55 - tmp52));
	       tmp107 = tmp105 - tmp106;
	       tmp113 = tmp105 + tmp106;
	       tmp100 = FFT_M(K382683432 , tmp98) + FFT_M(K923879532 , tmp99);
	       tmp103 = FFT_M(K382683432 , tmp101) - FFT_M(K923879532 , tmp102);
	       tmp104 = tmp100 + tmp103;
	       tmp108 = tmp103 - tmp100;
	  }
	  c_im(output[11 * ostride]) = tmp97 - tmp104;
	  c_im(output[3 * ostride]) = tmp97 + tmp104;
	  c_re(output[15 * ostride]) = tmp107 - tmp108;
	  c_re(output[7 * ostride]) = tmp107 + tmp108;
	  c_im(output[15 * ostride]) = tmp109 - tmp112;
	  c_im(output[7 * ostride]) = tmp109 + tmp112;
	  c_re(output[11 * ostride]) = tmp113 - tmp114;
	  c_re(output[3 * ostride]) = tmp113 + tmp114;
     }
}

void CFourier::fftwi_twiddle_8(fftw_complex *A, const fftw_complex *W, tNativeAcc iostride, tNativeAcc m, tNativeAcc dist)
{
	 tNativeAcc i;
     fftw_complex *inout;
     inout = A;
     for (i = m; i > 0; i = i - 1, inout = inout + dist, W = W + 7) {
	  fftw_real tmp7;
	  fftw_real tmp43;
	  fftw_real tmp71;
	  fftw_real tmp77;
	  fftw_real tmp41;
	  fftw_real tmp53;
	  fftw_real tmp56;
	  fftw_real tmp64;
	  fftw_real tmp18;
	  fftw_real tmp76;
	  fftw_real tmp46;
	  fftw_real tmp68;
	  fftw_real tmp30;
	  fftw_real tmp48;
	  fftw_real tmp51;
	  fftw_real tmp65;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp1;
	       fftw_real tmp70;
	       fftw_real tmp6;
	       fftw_real tmp69;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp1 = c_re(inout[0]);
	       tmp70 = c_im(inout[0]);
	       {
		    fftw_real tmp3;
		    fftw_real tmp5;
		    fftw_real tmp2;
		    fftw_real tmp4;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp3 = c_re(inout[4 * iostride]);
		    tmp5 = c_im(inout[4 * iostride]);
		    tmp2 = c_re(W[3]);
		    tmp4 = c_im(W[3]);
		    tmp6 = FFT_M(tmp2 , tmp3) + FFT_M(tmp4 , tmp5);
		    tmp69 = FFT_M(tmp2 , tmp5) - FFT_M(tmp4 , tmp3);
	       }
	       tmp7 = tmp1 + tmp6;
	       tmp43 = tmp1 - tmp6;
	       tmp71 = tmp69 + tmp70;
	       tmp77 = tmp70 - tmp69;
	  }
	  {
	       fftw_real tmp35;
	       fftw_real tmp54;
	       fftw_real tmp40;
	       fftw_real tmp55;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp32;
		    fftw_real tmp34;
		    fftw_real tmp31;
		    fftw_real tmp33;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp32 = c_re(inout[7 * iostride]);
		    tmp34 = c_im(inout[7 * iostride]);
		    tmp31 = c_re(W[6]);
		    tmp33 = c_im(W[6]);
		    tmp35 = FFT_M(tmp31 , tmp32) + FFT_M(tmp33 , tmp34);
		    tmp54 = FFT_M(tmp31 , tmp34) - FFT_M(tmp33 , tmp32);
	       }
	       {
		    fftw_real tmp37;
		    fftw_real tmp39;
		    fftw_real tmp36;
		    fftw_real tmp38;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp37 = c_re(inout[3 * iostride]);
		    tmp39 = c_im(inout[3 * iostride]);
		    tmp36 = c_re(W[2]);
		    tmp38 = c_im(W[2]);
		    tmp40 = FFT_M(tmp36 , tmp37) + FFT_M(tmp38 , tmp39);
		    tmp55 = FFT_M(tmp36 , tmp39) - FFT_M(tmp38 , tmp37);
	       }
	       tmp41 = tmp35 + tmp40;
	       tmp53 = tmp35 - tmp40;
	       tmp56 = tmp54 - tmp55;
	       tmp64 = tmp54 + tmp55;
	  }
	  {
	       fftw_real tmp12;
	       fftw_real tmp44;
	       fftw_real tmp17;
	       fftw_real tmp45;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp9;
		    fftw_real tmp11;
		    fftw_real tmp8;
		    fftw_real tmp10;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp9 = c_re(inout[2 * iostride]);
		    tmp11 = c_im(inout[2 * iostride]);
		    tmp8 = c_re(W[1]);
		    tmp10 = c_im(W[1]);
		    tmp12 = FFT_M(tmp8 , tmp9) + FFT_M(tmp10 , tmp11);
		    tmp44 = FFT_M(tmp8 , tmp11) - FFT_M(tmp10 , tmp9);
	       }
	       {
		    fftw_real tmp14;
		    fftw_real tmp16;
		    fftw_real tmp13;
		    fftw_real tmp15;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp14 = c_re(inout[6 * iostride]);
		    tmp16 = c_im(inout[6 * iostride]);
		    tmp13 = c_re(W[5]);
		    tmp15 = c_im(W[5]);
		    tmp17 = FFT_M(tmp13 , tmp14) + FFT_M(tmp15 , tmp16);
		    tmp45 = FFT_M(tmp13 , tmp16) - FFT_M(tmp15 , tmp14);
	       }
	       tmp18 = tmp12 + tmp17;
	       tmp76 = tmp12 - tmp17;
	       tmp46 = tmp44 - tmp45;
	       tmp68 = tmp44 + tmp45;
	  }
	  {
	       fftw_real tmp24;
	       fftw_real tmp49;
	       fftw_real tmp29;
	       fftw_real tmp50;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp21;
		    fftw_real tmp23;
		    fftw_real tmp20;
		    fftw_real tmp22;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp21 = c_re(inout[iostride]);
		    tmp23 = c_im(inout[iostride]);
		    tmp20 = c_re(W[0]);
		    tmp22 = c_im(W[0]);
		    tmp24 = FFT_M(tmp20 , tmp21) + FFT_M(tmp22 , tmp23);
		    tmp49 = FFT_M(tmp20 , tmp23) - FFT_M(tmp22 , tmp21);
	       }
	       {
		    fftw_real tmp26;
		    fftw_real tmp28;
		    fftw_real tmp25;
		    fftw_real tmp27;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp26 = c_re(inout[5 * iostride]);
		    tmp28 = c_im(inout[5 * iostride]);
		    tmp25 = c_re(W[4]);
		    tmp27 = c_im(W[4]);
		    tmp29 = FFT_M(tmp25 , tmp26) + FFT_M(tmp27 , tmp28);
		    tmp50 = FFT_M(tmp25 , tmp28) - FFT_M(tmp27 , tmp26);
	       }
	       tmp30 = tmp24 + tmp29;
	       tmp48 = tmp24 - tmp29;
	       tmp51 = tmp49 - tmp50;
	       tmp65 = tmp49 + tmp50;
	  }
	  {
	       fftw_real tmp19;
	       fftw_real tmp42;
	       fftw_real tmp63;
	       fftw_real tmp66;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp19 = tmp7 + tmp18;
	       tmp42 = tmp30 + tmp41;
	       c_re(inout[4 * iostride]) = tmp19 - tmp42;
	       c_re(inout[0]) = tmp19 + tmp42;
	       {
		    fftw_real tmp73;
		    fftw_real tmp74;
		    fftw_real tmp67;
		    fftw_real tmp72;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp73 = tmp30 - tmp41;
		    tmp74 = tmp71 - tmp68;
		    c_im(inout[2 * iostride]) = tmp73 + tmp74;
		    c_im(inout[6 * iostride]) = tmp74 - tmp73;
		    tmp67 = tmp65 + tmp64;
		    tmp72 = tmp68 + tmp71;
		    c_im(inout[0]) = tmp67 + tmp72;
		    c_im(inout[4 * iostride]) = tmp72 - tmp67;
	       }
	       tmp63 = tmp7 - tmp18;
	       tmp66 = tmp64 - tmp65;
	       c_re(inout[6 * iostride]) = tmp63 - tmp66;
	       c_re(inout[2 * iostride]) = tmp63 + tmp66;
	       {
		    fftw_real tmp59;
		    fftw_real tmp78;
		    fftw_real tmp62;
		    fftw_real tmp75;
		    fftw_real tmp60;
		    fftw_real tmp61;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp59 = tmp43 + tmp46;
		    tmp78 = tmp76 + tmp77;
		    tmp60 = tmp56 - tmp53;
		    tmp61 = tmp48 + tmp51;
		    tmp62 = FFT_M(K707106781 , (tmp60 - tmp61));
		    tmp75 = FFT_M(K707106781 , (tmp61 + tmp60));
		    c_re(inout[7 * iostride]) = tmp59 - tmp62;
		    c_re(inout[3 * iostride]) = tmp59 + tmp62;
		    c_im(inout[iostride]) = tmp75 + tmp78;
		    c_im(inout[5 * iostride]) = tmp78 - tmp75;
	       }
	       {
		    fftw_real tmp47;
		    fftw_real tmp80;
		    fftw_real tmp58;
		    fftw_real tmp79;
		    fftw_real tmp52;
		    fftw_real tmp57;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp47 = tmp43 - tmp46;
		    tmp80 = tmp77 - tmp76;
		    tmp52 = tmp48 - tmp51;
		    tmp57 = tmp53 + tmp56;
		    tmp58 = FFT_M(K707106781 , (tmp52 + tmp57));
		    tmp79 = FFT_M(K707106781 , (tmp52 - tmp57));
		    c_re(inout[5 * iostride]) = tmp47 - tmp58;
		    c_re(inout[iostride]) = tmp47 + tmp58;
		    c_im(inout[3 * iostride]) = tmp79 + tmp80;
		    c_im(inout[7 * iostride]) = tmp80 - tmp79;
	       }
	  }
     }
}

void CFourier::fftwi_twiddle_9(fftw_complex *A, const fftw_complex *W, tNativeAcc iostride, tNativeAcc m, tNativeAcc dist)
{
	 tNativeAcc i;
     fftw_complex *inout;
     inout = A;
     for (i = m; i > 0; i = i - 1, inout = inout + dist, W = W + 8) {
	  fftw_real tmp1;
	  fftw_real tmp99;
	  fftw_real tmp64;
	  fftw_real tmp98;
	  fftw_real tmp105;
	  fftw_real tmp104;
	  fftw_real tmp12;
	  fftw_real tmp61;
	  fftw_real tmp47;
	  fftw_real tmp78;
	  fftw_real tmp89;
	  fftw_real tmp54;
	  fftw_real tmp75;
	  fftw_real tmp90;
	  fftw_real tmp30;
	  fftw_real tmp68;
	  fftw_real tmp86;
	  fftw_real tmp59;
	  fftw_real tmp71;
	  fftw_real tmp87;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp6;
	       fftw_real tmp63;
	       fftw_real tmp11;
	       fftw_real tmp62;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp1 = c_re(inout[0]);
	       tmp99 = c_im(inout[0]);
	       {
		    fftw_real tmp3;
		    fftw_real tmp5;
		    fftw_real tmp2;
		    fftw_real tmp4;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp3 = c_re(inout[3 * iostride]);
		    tmp5 = c_im(inout[3 * iostride]);
		    tmp2 = c_re(W[2]);
		    tmp4 = c_im(W[2]);
		    tmp6 = FFT_M(tmp2 , tmp3) + FFT_M(tmp4 , tmp5);
		    tmp63 = FFT_M(tmp2 , tmp5) - FFT_M(tmp4 , tmp3);
	       }
	       {
		    fftw_real tmp8;
		    fftw_real tmp10;
		    fftw_real tmp7;
		    fftw_real tmp9;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp8 = c_re(inout[6 * iostride]);
		    tmp10 = c_im(inout[6 * iostride]);
		    tmp7 = c_re(W[5]);
		    tmp9 = c_im(W[5]);
		    tmp11 = FFT_M(tmp7 , tmp8) + FFT_M(tmp9 , tmp10);
		    tmp62 = FFT_M(tmp7 , tmp10) - FFT_M(tmp9 , tmp8);
	       }
	       tmp64 = FFT_M(K866025403 , (tmp62 - tmp63));
	       tmp98 = tmp63 + tmp62;
	       tmp105 = tmp99 - FFT_M(K500000000 , tmp98);
	       tmp104 = FFT_M(K866025403 , (tmp6 - tmp11));
	       tmp12 = tmp6 + tmp11;
	       tmp61 = tmp1 - FFT_M(K500000000 , tmp12);
	  }
	  {
	       fftw_real tmp35;
	       fftw_real tmp50;
	       fftw_real tmp40;
	       fftw_real tmp51;
	       fftw_real tmp45;
	       fftw_real tmp52;
	       fftw_real tmp46;
	       fftw_real tmp53;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp32;
		    fftw_real tmp34;
		    fftw_real tmp31;
		    fftw_real tmp33;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp32 = c_re(inout[2 * iostride]);
		    tmp34 = c_im(inout[2 * iostride]);
		    tmp31 = c_re(W[1]);
		    tmp33 = c_im(W[1]);
		    tmp35 = FFT_M(tmp31 , tmp32) + FFT_M(tmp33 , tmp34);
		    tmp50 = FFT_M(tmp31 , tmp34) - FFT_M(tmp33 , tmp32);
	       }
	       {
		    fftw_real tmp37;
		    fftw_real tmp39;
		    fftw_real tmp36;
		    fftw_real tmp38;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp37 = c_re(inout[5 * iostride]);
		    tmp39 = c_im(inout[5 * iostride]);
		    tmp36 = c_re(W[4]);
		    tmp38 = c_im(W[4]);
		    tmp40 = FFT_M(tmp36 , tmp37) + FFT_M(tmp38 , tmp39);
		    tmp51 = FFT_M(tmp36 , tmp39) - FFT_M(tmp38 , tmp37);
	       }
	       {
		    fftw_real tmp42;
		    fftw_real tmp44;
		    fftw_real tmp41;
		    fftw_real tmp43;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp42 = c_re(inout[8 * iostride]);
		    tmp44 = c_im(inout[8 * iostride]);
		    tmp41 = c_re(W[7]);
		    tmp43 = c_im(W[7]);
		    tmp45 = FFT_M(tmp41 , tmp42) + FFT_M(tmp43 , tmp44);
		    tmp52 = FFT_M(tmp41 , tmp44) - FFT_M(tmp43 , tmp42);
	       }
	       tmp46 = tmp40 + tmp45;
	       tmp53 = tmp51 + tmp52;
	       {
		    fftw_real tmp76;
		    fftw_real tmp77;
		    fftw_real tmp73;
		    fftw_real tmp74;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp47 = tmp35 + tmp46;
		    tmp76 = tmp35 - FFT_M(K500000000 , tmp46);
		    tmp77 = FFT_M(K866025403 , (tmp52 - tmp51));
		    tmp78 = tmp76 - tmp77;
		    tmp89 = tmp76 + tmp77;
		    tmp54 = tmp50 + tmp53;
		    tmp73 = tmp50 - FFT_M(K500000000 , tmp53);
		    tmp74 = FFT_M(K866025403 , (tmp40 - tmp45));
		    tmp75 = tmp73 - tmp74;
		    tmp90 = tmp74 + tmp73;
	       }
	  }
	  {
	       fftw_real tmp18;
	       fftw_real tmp55;
	       fftw_real tmp23;
	       fftw_real tmp56;
	       fftw_real tmp28;
	       fftw_real tmp57;
	       fftw_real tmp29;
	       fftw_real tmp58;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp15;
		    fftw_real tmp17;
		    fftw_real tmp14;
		    fftw_real tmp16;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp15 = c_re(inout[iostride]);
		    tmp17 = c_im(inout[iostride]);
		    tmp14 = c_re(W[0]);
		    tmp16 = c_im(W[0]);
		    tmp18 = FFT_M(tmp14 , tmp15) + FFT_M(tmp16 , tmp17);
		    tmp55 = FFT_M(tmp14 , tmp17) - FFT_M(tmp16 , tmp15);
	       }
	       {
		    fftw_real tmp20;
		    fftw_real tmp22;
		    fftw_real tmp19;
		    fftw_real tmp21;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp20 = c_re(inout[4 * iostride]);
		    tmp22 = c_im(inout[4 * iostride]);
		    tmp19 = c_re(W[3]);
		    tmp21 = c_im(W[3]);
		    tmp23 = FFT_M(tmp19 , tmp20) + FFT_M(tmp21 , tmp22);
		    tmp56 = FFT_M(tmp19 , tmp22) - FFT_M(tmp21 , tmp20);
	       }
	       {
		    fftw_real tmp25;
		    fftw_real tmp27;
		    fftw_real tmp24;
		    fftw_real tmp26;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp25 = c_re(inout[7 * iostride]);
		    tmp27 = c_im(inout[7 * iostride]);
		    tmp24 = c_re(W[6]);
		    tmp26 = c_im(W[6]);
		    tmp28 = FFT_M(tmp24 , tmp25) + FFT_M(tmp26 , tmp27);
		    tmp57 = FFT_M(tmp24 , tmp27) - FFT_M(tmp26 , tmp25);
	       }
	       tmp29 = tmp23 + tmp28;
	       tmp58 = tmp56 + tmp57;
	       {
		    fftw_real tmp66;
		    fftw_real tmp67;
		    fftw_real tmp69;
		    fftw_real tmp70;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp30 = tmp18 + tmp29;
		    tmp66 = tmp18 - FFT_M(K500000000 , tmp29);
		    tmp67 = FFT_M(K866025403 , (tmp57 - tmp56));
		    tmp68 = tmp66 - tmp67;
		    tmp86 = tmp66 + tmp67;
		    tmp59 = tmp55 + tmp58;
		    tmp69 = tmp55 - FFT_M(K500000000 , tmp58);
		    tmp70 = FFT_M(K866025403 , (tmp23 - tmp28));
		    tmp71 = tmp69 - tmp70;
		    tmp87 = tmp70 + tmp69;
	       }
	  }
	  {
	       fftw_real tmp60;
	       fftw_real tmp13;
	       fftw_real tmp48;
	       fftw_real tmp49;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp60 = FFT_M(K866025403 , (tmp54 - tmp59));
	       tmp13 = tmp1 + tmp12;
	       tmp48 = tmp30 + tmp47;
	       tmp49 = tmp13 - FFT_M(K500000000 , tmp48);
	       c_re(inout[0]) = tmp13 + tmp48;
	       c_re(inout[3 * iostride]) = tmp49 + tmp60;
	       c_re(inout[6 * iostride]) = tmp49 - tmp60;
	  }
	  {
	       fftw_real tmp101;
	       fftw_real tmp97;
	       fftw_real tmp100;
	       fftw_real tmp102;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp101 = FFT_M(K866025403 , (tmp30 - tmp47));
	       tmp97 = tmp59 + tmp54;
	       tmp100 = tmp98 + tmp99;
	       tmp102 = tmp100 - FFT_M(K500000000 , tmp97);
	       c_im(inout[0]) = tmp97 + tmp100;
	       c_im(inout[6 * iostride]) = tmp102 - tmp101;
	       c_im(inout[3 * iostride]) = tmp101 + tmp102;
	  }
	  {
	       fftw_real tmp65;
	       fftw_real tmp110;
	       fftw_real tmp80;
	       fftw_real tmp111;
	       fftw_real tmp84;
	       fftw_real tmp109;
	       fftw_real tmp81;
	       fftw_real tmp112;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp65 = tmp61 - tmp64;
	       tmp110 = tmp105 - tmp104;
	       {
		    fftw_real tmp72;
		    fftw_real tmp79;
		    fftw_real tmp82;
		    fftw_real tmp83;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp72 = FFT_M(K173648177 , tmp68) - FFT_M(K984807753 , tmp71);
		    tmp79 = FFT_M(K342020143 , tmp75) + FFT_M(K939692620 , tmp78);
		    tmp80 = tmp72 - tmp79;
		    tmp111 = FFT_M(K866025403 , (tmp72 + tmp79));
		    tmp82 = FFT_M(K342020143 , tmp78) - FFT_M(K939692620 , tmp75);
		    tmp83 = FFT_M(K173648177 , tmp71) + FFT_M(K984807753 , tmp68);
		    tmp84 = FFT_M(K866025403 , (tmp82 - tmp83));
		    tmp109 = tmp83 + tmp82;
	       }
	       c_re(inout[2 * iostride]) = tmp65 + tmp80;
	       tmp81 = tmp65 - FFT_M(K500000000 , tmp80);
	       c_re(inout[8 * iostride]) = tmp81 - tmp84;
	       c_re(inout[5 * iostride]) = tmp81 + tmp84;
	       c_im(inout[2 * iostride]) = tmp109 + tmp110;
	       tmp112 = tmp110 - FFT_M(K500000000 , tmp109);
	       c_im(inout[5 * iostride]) = tmp111 + tmp112;
	       c_im(inout[8 * iostride]) = tmp112 - tmp111;
	  }
	  {
	       fftw_real tmp85;
	       fftw_real tmp106;
	       fftw_real tmp92;
	       fftw_real tmp107;
	       fftw_real tmp96;
	       fftw_real tmp103;
	       fftw_real tmp93;
	       fftw_real tmp108;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp85 = tmp61 + tmp64;
	       tmp106 = tmp104 + tmp105;
	       {
		    fftw_real tmp88;
		    fftw_real tmp91;
		    fftw_real tmp94;
		    fftw_real tmp95;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp88 = FFT_M(K766044443 , tmp86) - FFT_M(K642787609 , tmp87);
		    tmp91 = FFT_M(K173648177 , tmp89) - FFT_M(K984807753 , tmp90);
		    tmp92 = tmp88 + tmp91;
		    tmp107 = FFT_M(K866025403 , (tmp88 - tmp91));
		    tmp94 = FFT_M(K173648177 , tmp90) + FFT_M(K984807753 , tmp89);
		    tmp95 = FFT_M(K766044443 , tmp87) + FFT_M(K642787609 , tmp86);
		    tmp96 = FFT_M(K866025403 , (tmp94 - tmp95));
		    tmp103 = tmp95 + tmp94;
	       }
	       c_re(inout[iostride]) = tmp85 + tmp92;
	       tmp93 = tmp85 - FFT_M(K500000000 , tmp92);
	       c_re(inout[7 * iostride]) = tmp93 - tmp96;
	       c_re(inout[4 * iostride]) = tmp93 + tmp96;
	       c_im(inout[iostride]) = tmp103 + tmp106;
	       tmp108 = tmp106 - FFT_M(K500000000 , tmp103);
	       c_im(inout[4 * iostride]) = tmp107 + tmp108;
	       c_im(inout[7 * iostride]) = tmp108 - tmp107;
	  }
     }
}

void CFourier::fftwi_twiddle_4(fftw_complex *A, const fftw_complex *W, tNativeAcc iostride, tNativeAcc m, tNativeAcc dist)
{
	 tNativeAcc i;
     fftw_complex *inout;
     inout = A;
     for (i = m; i > 0; i = i - 1, inout = inout + dist, W = W + 3) {
	  fftw_real tmp1;
	  fftw_real tmp25;
	  fftw_real tmp6;
	  fftw_real tmp24;
	  fftw_real tmp12;
	  fftw_real tmp20;
	  fftw_real tmp17;
	  fftw_real tmp21;
	  ASSERT_ALIGNED_DOUBLE;
	  tmp1 = c_re(inout[0]);
	  tmp25 = c_im(inout[0]);
	  {
	       fftw_real tmp3;
	       fftw_real tmp5;
	       fftw_real tmp2;
	       fftw_real tmp4;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp3 = c_re(inout[2 * iostride]);
	       tmp5 = c_im(inout[2 * iostride]);
	       tmp2 = c_re(W[1]);
	       tmp4 = c_im(W[1]);
	       tmp6 = FFT_M(tmp2 , tmp3) + FFT_M(tmp4 , tmp5);
	       tmp24 = FFT_M(tmp2 , tmp5) - FFT_M(tmp4 , tmp3);
	  }
	  {
	       fftw_real tmp9;
	       fftw_real tmp11;
	       fftw_real tmp8;
	       fftw_real tmp10;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp9 = c_re(inout[iostride]);
	       tmp11 = c_im(inout[iostride]);
	       tmp8 = c_re(W[0]);
	       tmp10 = c_im(W[0]);
	       tmp12 = FFT_M(tmp8 , tmp9) + FFT_M(tmp10 , tmp11);
	       tmp20 = FFT_M(tmp8 , tmp11) - FFT_M(tmp10 , tmp9);
	  }
	  {
	       fftw_real tmp14;
	       fftw_real tmp16;
	       fftw_real tmp13;
	       fftw_real tmp15;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp14 = c_re(inout[3 * iostride]);
	       tmp16 = c_im(inout[3 * iostride]);
	       tmp13 = c_re(W[2]);
	       tmp15 = c_im(W[2]);
	       tmp17 = FFT_M(tmp13 , tmp14) + FFT_M(tmp15 , tmp16);
	       tmp21 = FFT_M(tmp13 , tmp16) - FFT_M(tmp15 , tmp14);
	  }
	  {
	       fftw_real tmp7;
	       fftw_real tmp18;
	       fftw_real tmp27;
	       fftw_real tmp28;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp7 = tmp1 + tmp6;
	       tmp18 = tmp12 + tmp17;
	       c_re(inout[2 * iostride]) = tmp7 - tmp18;
	       c_re(inout[0]) = tmp7 + tmp18;
	       tmp27 = tmp12 - tmp17;
	       tmp28 = tmp25 - tmp24;
	       c_im(inout[iostride]) = tmp27 + tmp28;
	       c_im(inout[3 * iostride]) = tmp28 - tmp27;
	  }
	  {
	       fftw_real tmp23;
	       fftw_real tmp26;
	       fftw_real tmp19;
	       fftw_real tmp22;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp23 = tmp20 + tmp21;
	       tmp26 = tmp24 + tmp25;
	       c_im(inout[0]) = tmp23 + tmp26;
	       c_im(inout[2 * iostride]) = tmp26 - tmp23;
	       tmp19 = tmp1 - tmp6;
	       tmp22 = tmp20 - tmp21;
	       c_re(inout[iostride]) = tmp19 - tmp22;
	       c_re(inout[3 * iostride]) = tmp19 + tmp22;
	  }
     }
}

void CFourier::fftw_twiddle_7(fftw_complex *A, const fftw_complex *W, tNativeAcc iostride, tNativeAcc m, tNativeAcc dist)
{
	 tNativeAcc i;
     fftw_complex *inout;
     inout = A;
     for (i = m; i > 0; i = i - 1, inout = inout + dist, W = W + 6) {
	  fftw_real tmp1;
	  fftw_real tmp53;
	  fftw_real tmp12;
	  fftw_real tmp54;
	  fftw_real tmp38;
	  fftw_real tmp50;
	  fftw_real tmp23;
	  fftw_real tmp55;
	  fftw_real tmp44;
	  fftw_real tmp51;
	  fftw_real tmp34;
	  fftw_real tmp56;
	  fftw_real tmp41;
	  fftw_real tmp52;
	  ASSERT_ALIGNED_DOUBLE;
	  tmp1 = c_re(inout[0]);
	  tmp53 = c_im(inout[0]);
	  {
	       fftw_real tmp6;
	       fftw_real tmp36;
	       fftw_real tmp11;
	       fftw_real tmp37;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp3;
		    fftw_real tmp5;
		    fftw_real tmp2;
		    fftw_real tmp4;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp3 = c_re(inout[iostride]);
		    tmp5 = c_im(inout[iostride]);
		    tmp2 = c_re(W[0]);
		    tmp4 = c_im(W[0]);
		    tmp6 = FFT_M(tmp2 , tmp3) - FFT_M(tmp4 , tmp5);
		    tmp36 = FFT_M(tmp4 , tmp3) + FFT_M(tmp2 , tmp5);
	       }
	       {
		    fftw_real tmp8;
		    fftw_real tmp10;
		    fftw_real tmp7;
		    fftw_real tmp9;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp8 = c_re(inout[6 * iostride]);
		    tmp10 = c_im(inout[6 * iostride]);
		    tmp7 = c_re(W[5]);
		    tmp9 = c_im(W[5]);
		    tmp11 = FFT_M(tmp7 , tmp8) - FFT_M(tmp9 , tmp10);
		    tmp37 = FFT_M(tmp9 , tmp8) + FFT_M(tmp7 , tmp10);
	       }
	       tmp12 = tmp6 + tmp11;
	       tmp54 = tmp11 - tmp6;
	       tmp38 = tmp36 - tmp37;
	       tmp50 = tmp36 + tmp37;
	  }
	  {
	       fftw_real tmp17;
	       fftw_real tmp42;
	       fftw_real tmp22;
	       fftw_real tmp43;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp14;
		    fftw_real tmp16;
		    fftw_real tmp13;
		    fftw_real tmp15;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp14 = c_re(inout[2 * iostride]);
		    tmp16 = c_im(inout[2 * iostride]);
		    tmp13 = c_re(W[1]);
		    tmp15 = c_im(W[1]);
		    tmp17 = FFT_M(tmp13 , tmp14) - FFT_M(tmp15 , tmp16);
		    tmp42 = FFT_M(tmp15 , tmp14) + FFT_M(tmp13 , tmp16);
	       }
	       {
		    fftw_real tmp19;
		    fftw_real tmp21;
		    fftw_real tmp18;
		    fftw_real tmp20;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp19 = c_re(inout[5 * iostride]);
		    tmp21 = c_im(inout[5 * iostride]);
		    tmp18 = c_re(W[4]);
		    tmp20 = c_im(W[4]);
		    tmp22 = FFT_M(tmp18 , tmp19) - FFT_M(tmp20 , tmp21);
		    tmp43 = FFT_M(tmp20 , tmp19) + FFT_M(tmp18 , tmp21);
	       }
	       tmp23 = tmp17 + tmp22;
	       tmp55 = tmp22 - tmp17;
	       tmp44 = tmp42 - tmp43;
	       tmp51 = tmp42 + tmp43;
	  }
	  {
	       fftw_real tmp28;
	       fftw_real tmp39;
	       fftw_real tmp33;
	       fftw_real tmp40;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp25;
		    fftw_real tmp27;
		    fftw_real tmp24;
		    fftw_real tmp26;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp25 = c_re(inout[3 * iostride]);
		    tmp27 = c_im(inout[3 * iostride]);
		    tmp24 = c_re(W[2]);
		    tmp26 = c_im(W[2]);
		    tmp28 = FFT_M(tmp24 , tmp25) - FFT_M(tmp26 , tmp27);
		    tmp39 = FFT_M(tmp26 , tmp25) + FFT_M(tmp24 , tmp27);
	       }
	       {
		    fftw_real tmp30;
		    fftw_real tmp32;
		    fftw_real tmp29;
		    fftw_real tmp31;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp30 = c_re(inout[4 * iostride]);
		    tmp32 = c_im(inout[4 * iostride]);
		    tmp29 = c_re(W[3]);
		    tmp31 = c_im(W[3]);
		    tmp33 = FFT_M(tmp29 , tmp30) - FFT_M(tmp31 , tmp32);
		    tmp40 = FFT_M(tmp31 , tmp30) + FFT_M(tmp29 , tmp32);
	       }
	       tmp34 = tmp28 + tmp33;
	       tmp56 = tmp33 - tmp28;
	       tmp41 = tmp39 - tmp40;
	       tmp52 = tmp39 + tmp40;
	  }
	  {
	       fftw_real tmp47;
	       fftw_real tmp46;
	       fftw_real tmp59;
	       fftw_real tmp60;
	       ASSERT_ALIGNED_DOUBLE;
	       c_re(inout[0]) = tmp1 + tmp12 + tmp23 + tmp34;
	       tmp47 = FFT_M(K781831482 , tmp38) + FFT_M(K974927912 , tmp44) + FFT_M(K433883739 , tmp41);
	       tmp46 = tmp1 + FFT_M(K623489801 , tmp12) - FFT_M(K900968867 , tmp34) - FFT_M(K222520933 , tmp23);
	       c_re(inout[6 * iostride]) = tmp46 - tmp47;
	       c_re(inout[iostride]) = tmp46 + tmp47;
	       {
		    fftw_real tmp49;
		    fftw_real tmp48;
		    fftw_real tmp45;
		    fftw_real tmp35;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp49 = FFT_M(K433883739 , tmp38) + FFT_M(K974927912 , tmp41) - FFT_M(K781831482 , tmp44);
		    tmp48 = tmp1 + FFT_M(K623489801 , tmp23) - FFT_M(K222520933 , tmp34) - FFT_M(K900968867 , tmp12);
		    c_re(inout[4 * iostride]) = tmp48 - tmp49;
		    c_re(inout[3 * iostride]) = tmp48 + tmp49;
		    tmp45 = FFT_M(K974927912 , tmp38) - FFT_M(K781831482 , tmp41) - FFT_M(K433883739 , tmp44);
		    tmp35 = tmp1 + FFT_M(K623489801 , tmp34) - FFT_M(K900968867 , tmp23) - FFT_M(K222520933 , tmp12);
		    c_re(inout[5 * iostride]) = tmp35 - tmp45;
		    c_re(inout[2 * iostride]) = tmp35 + tmp45;
	       }
	       c_im(inout[0]) = tmp50 + tmp51 + tmp52 + tmp53;
	       tmp59 = FFT_M(K974927912 , tmp54) - FFT_M(K781831482 , tmp56) - FFT_M(K433883739 , tmp55);
	       tmp60 = FFT_M(K623489801 , tmp52) + tmp53 - FFT_M(K900968867 , tmp51) - FFT_M(K222520933 , tmp50);
	       c_im(inout[2 * iostride]) = tmp59 + tmp60;
	       c_im(inout[5 * iostride]) = tmp60 - tmp59;
	       {
		    fftw_real tmp61;
		    fftw_real tmp62;
		    fftw_real tmp57;
		    fftw_real tmp58;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp61 = FFT_M(K433883739 , tmp54) + FFT_M(K974927912 , tmp56) - FFT_M(K781831482 , tmp55);
		    tmp62 = FFT_M(K623489801 , tmp51) + tmp53 - FFT_M(K222520933 , tmp52) - FFT_M(K900968867 , tmp50);
		    c_im(inout[3 * iostride]) = tmp61 + tmp62;
		    c_im(inout[4 * iostride]) = tmp62 - tmp61;
		    tmp57 = FFT_M(K781831482 , tmp54) + FFT_M(K974927912 , tmp55) + FFT_M(K433883739 , tmp56);
		    tmp58 = FFT_M(K623489801 , tmp50) + tmp53 - FFT_M(K900968867 , tmp52) - FFT_M(K222520933 , tmp51);
		    c_im(inout[iostride]) = tmp57 + tmp58;
		    c_im(inout[6 * iostride]) = tmp58 - tmp57;
	       }
	  }
     }
}


void CFourier::fftwi_twiddle_7(fftw_complex *A, const fftw_complex *W, tNativeAcc iostride, tNativeAcc m, tNativeAcc dist)
{
	 tNativeAcc i;
     fftw_complex *inout;
     inout = A;
     for (i = m; i > 0; i = i - 1, inout = inout + dist, W = W + 6) {
	  fftw_real tmp1;
	  fftw_real tmp53;
	  fftw_real tmp12;
	  fftw_real tmp54;
	  fftw_real tmp38;
	  fftw_real tmp50;
	  fftw_real tmp23;
	  fftw_real tmp55;
	  fftw_real tmp44;
	  fftw_real tmp51;
	  fftw_real tmp34;
	  fftw_real tmp56;
	  fftw_real tmp41;
	  fftw_real tmp52;
	  ASSERT_ALIGNED_DOUBLE;
	  tmp1 = c_re(inout[0]);
	  tmp53 = c_im(inout[0]);
	  {
	       fftw_real tmp6;
	       fftw_real tmp37;
	       fftw_real tmp11;
	       fftw_real tmp36;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp3;
		    fftw_real tmp5;
		    fftw_real tmp2;
		    fftw_real tmp4;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp3 = c_re(inout[iostride]);
		    tmp5 = c_im(inout[iostride]);
		    tmp2 = c_re(W[0]);
		    tmp4 = c_im(W[0]);
		    tmp6 = FFT_M(tmp2 , tmp3) + FFT_M(tmp4 , tmp5);
		    tmp37 = FFT_M(tmp2 , tmp5) - FFT_M(tmp4 , tmp3);
	       }
	       {
		    fftw_real tmp8;
		    fftw_real tmp10;
		    fftw_real tmp7;
		    fftw_real tmp9;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp8 = c_re(inout[6 * iostride]);
		    tmp10 = c_im(inout[6 * iostride]);
		    tmp7 = c_re(W[5]);
		    tmp9 = c_im(W[5]);
		    tmp11 = FFT_M(tmp7 , tmp8) + FFT_M(tmp9 , tmp10);
		    tmp36 = FFT_M(tmp7 , tmp10) - FFT_M(tmp9 , tmp8);
	       }
	       tmp12 = tmp6 + tmp11;
	       tmp54 = tmp6 - tmp11;
	       tmp38 = tmp36 - tmp37;
	       tmp50 = tmp37 + tmp36;
	  }
	  {
	       fftw_real tmp17;
	       fftw_real tmp43;
	       fftw_real tmp22;
	       fftw_real tmp42;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp14;
		    fftw_real tmp16;
		    fftw_real tmp13;
		    fftw_real tmp15;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp14 = c_re(inout[2 * iostride]);
		    tmp16 = c_im(inout[2 * iostride]);
		    tmp13 = c_re(W[1]);
		    tmp15 = c_im(W[1]);
		    tmp17 = FFT_M(tmp13 , tmp14) + FFT_M(tmp15 , tmp16);
		    tmp43 = FFT_M(tmp13 , tmp16) - FFT_M(tmp15 , tmp14);
	       }
	       {
		    fftw_real tmp19;
		    fftw_real tmp21;
		    fftw_real tmp18;
		    fftw_real tmp20;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp19 = c_re(inout[5 * iostride]);
		    tmp21 = c_im(inout[5 * iostride]);
		    tmp18 = c_re(W[4]);
		    tmp20 = c_im(W[4]);
		    tmp22 = FFT_M(tmp18 , tmp19) + FFT_M(tmp20 , tmp21);
		    tmp42 = FFT_M(tmp18 , tmp21) - FFT_M(tmp20 , tmp19);
	       }
	       tmp23 = tmp17 + tmp22;
	       tmp55 = tmp17 - tmp22;
	       tmp44 = tmp42 - tmp43;
	       tmp51 = tmp43 + tmp42;
	  }
	  {
	       fftw_real tmp28;
	       fftw_real tmp40;
	       fftw_real tmp33;
	       fftw_real tmp39;
	       ASSERT_ALIGNED_DOUBLE;
	       {
		    fftw_real tmp25;
		    fftw_real tmp27;
		    fftw_real tmp24;
		    fftw_real tmp26;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp25 = c_re(inout[3 * iostride]);
		    tmp27 = c_im(inout[3 * iostride]);
		    tmp24 = c_re(W[2]);
		    tmp26 = c_im(W[2]);
		    tmp28 = FFT_M(tmp24 , tmp25) + FFT_M(tmp26 , tmp27);
		    tmp40 = FFT_M(tmp24 , tmp27) - FFT_M(tmp26 , tmp25);
	       }
	       {
		    fftw_real tmp30;
		    fftw_real tmp32;
		    fftw_real tmp29;
		    fftw_real tmp31;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp30 = c_re(inout[4 * iostride]);
		    tmp32 = c_im(inout[4 * iostride]);
		    tmp29 = c_re(W[3]);
		    tmp31 = c_im(W[3]);
		    tmp33 = FFT_M(tmp29 , tmp30) + FFT_M(tmp31 , tmp32);
		    tmp39 = FFT_M(tmp29 , tmp32) - FFT_M(tmp31 , tmp30);
	       }
	       tmp34 = tmp28 + tmp33;
	       tmp56 = tmp28 - tmp33;
	       tmp41 = tmp39 - tmp40;
	       tmp52 = tmp40 + tmp39;
	  }
	  {
	       fftw_real tmp47;
	       fftw_real tmp46;
	       fftw_real tmp59;
	       fftw_real tmp60;
	       ASSERT_ALIGNED_DOUBLE;
	       c_re(inout[0]) = tmp1 + tmp12 + tmp23 + tmp34;
	       tmp47 = FFT_M(K781831482 , tmp38) + FFT_M(K974927912 , tmp44) + FFT_M(K433883739 , tmp41);
	       tmp46 = tmp1 + FFT_M(K623489801 , tmp12) - FFT_M(K900968867 , tmp34) - FFT_M(K222520933 , tmp23);
	       c_re(inout[6 * iostride]) = tmp46 - tmp47;
	       c_re(inout[iostride]) = tmp46 + tmp47;
	       {
		    fftw_real tmp49;
		    fftw_real tmp48;
		    fftw_real tmp45;
		    fftw_real tmp35;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp49 = FFT_M(K433883739 , tmp38) + FFT_M(K974927912 , tmp41) - FFT_M(K781831482 , tmp44);
		    tmp48 = tmp1 + FFT_M(K623489801 , tmp23) - FFT_M(K222520933 , tmp34) - FFT_M(K900968867 , tmp12);
		    c_re(inout[4 * iostride]) = tmp48 - tmp49;
		    c_re(inout[3 * iostride]) = tmp48 + tmp49;
		    tmp45 = FFT_M(K974927912 , tmp38) - FFT_M(K781831482 , tmp41) - FFT_M(K433883739 , tmp44);
		    tmp35 = tmp1 + FFT_M(K623489801 , tmp34) - FFT_M(K900968867 , tmp23) - FFT_M(K222520933 , tmp12);
		    c_re(inout[5 * iostride]) = tmp35 - tmp45;
		    c_re(inout[2 * iostride]) = tmp35 + tmp45;
	       }
	       c_im(inout[0]) = tmp50 + tmp51 + tmp52 + tmp53;
	       tmp59 = FFT_M(K974927912 , tmp54) - FFT_M(K781831482 , tmp56) - FFT_M(K433883739 , tmp55);
	       tmp60 = FFT_M(K623489801 , tmp52) + tmp53 - FFT_M(K900968867 , tmp51) - FFT_M(K222520933 , tmp50);
	       c_im(inout[2 * iostride]) = tmp59 + tmp60;
	       c_im(inout[5 * iostride]) = tmp60 - tmp59;
	       {
		    fftw_real tmp61;
		    fftw_real tmp62;
		    fftw_real tmp57;
		    fftw_real tmp58;
		    ASSERT_ALIGNED_DOUBLE;
		    tmp61 = FFT_M(K433883739 , tmp54) + FFT_M(K974927912 , tmp56) - FFT_M(K781831482 , tmp55);
		    tmp62 = FFT_M(K623489801 , tmp51) + tmp53 - FFT_M(K222520933 , tmp52) - FFT_M(K900968867 , tmp50);
		    c_im(inout[3 * iostride]) = tmp61 + tmp62;
		    c_im(inout[4 * iostride]) = tmp62 - tmp61;
		    tmp57 = FFT_M(K781831482 , tmp54) + FFT_M(K974927912 , tmp55) + FFT_M(K433883739 , tmp56);
		    tmp58 = FFT_M(K623489801 , tmp50) + tmp53 - FFT_M(K900968867 , tmp52) - FFT_M(K222520933 , tmp51);
		    c_im(inout[iostride]) = tmp57 + tmp58;
		    c_im(inout[6 * iostride]) = tmp58 - tmp57;
	       }
	  }
     }
}


void CFourier::fftw_no_twiddle_11(const fftw_complex *input,fftw_complex *output,tNativeAcc istride,tNativeAcc ostride)
{
	 //Version where I take the value down within the function
	 fftw_real tmp1;
     fftw_real tmp48;
     fftw_real tmp4;
     fftw_real tmp42;
     fftw_real tmp20;
     fftw_real tmp53;
     fftw_real tmp32;
     fftw_real tmp49;
     fftw_real tmp7;
     fftw_real tmp46;
     fftw_real tmp10;
     fftw_real tmp43;
     fftw_real tmp23;
     fftw_real tmp52;
     fftw_real tmp13;
     fftw_real tmp45;
     fftw_real tmp26;
     fftw_real tmp50;
     fftw_real tmp29;
     fftw_real tmp51;
     fftw_real tmp16;
     fftw_real tmp44;
     ASSERT_ALIGNED_DOUBLE;
     {
	  fftw_real tmp2;
	  fftw_real tmp3;
	  fftw_real tmp18;
	  fftw_real tmp19;
	  ASSERT_ALIGNED_DOUBLE;
	  tmp1 = (c_re(input[0]))>>2;
	  tmp48 = (c_im(input[0]))>>2;
	  tmp2 = c_re(input[istride]);
	  tmp3 = c_re(input[10 * istride]);
	  tmp4 = (tmp2 + tmp3)>>2;
	  tmp42 = (tmp3 - tmp2)>>2;
	  tmp18 = c_im(input[istride]);
	  tmp19 = c_im(input[10 * istride]);
	  tmp20 = (tmp18 - tmp19)>>2;
	  tmp53 = (tmp18 + tmp19)>>2;
	  {
	       fftw_real tmp30;
	       fftw_real tmp31;
	       fftw_real tmp5;
	       fftw_real tmp6;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp30 = c_im(input[2 * istride]);
	       tmp31 = c_im(input[9 * istride]);
	       tmp32 = (tmp30 - tmp31)>>2;
	       tmp49 = (tmp30 + tmp31)>>2;
	       tmp5 = c_re(input[2 * istride]);
	       tmp6 = c_re(input[9 * istride]);
	       tmp7 = (tmp5 + tmp6)>>2;
	       tmp46 = (tmp6 - tmp5)>>2;
	  }
     }
     {
	  fftw_real tmp8;
	  fftw_real tmp9;
	  fftw_real tmp24;
	  fftw_real tmp25;
	  ASSERT_ALIGNED_DOUBLE;
	  tmp8 = c_re(input[3 * istride]);
	  tmp9 = c_re(input[8 * istride]);
	  tmp10 = (tmp8 + tmp9)>>2;
	  tmp43 = (tmp9 - tmp8)>>2;
	  {
	       fftw_real tmp21;
	       fftw_real tmp22;
	       fftw_real tmp11;
	       fftw_real tmp12;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp21 = c_im(input[3 * istride]);
	       tmp22 = c_im(input[8 * istride]);
	       tmp23 = (tmp21 - tmp22)>>2;
	       tmp52 = (tmp21 + tmp22)>>2;
	       tmp11 = c_re(input[4 * istride]);
	       tmp12 = c_re(input[7 * istride]);
	       tmp13 = (tmp11 + tmp12)>>2;
	       tmp45 = (tmp12 - tmp11)>>2;
	  }
	  tmp24 = c_im(input[4 * istride]);
	  tmp25 = c_im(input[7 * istride]);
	  tmp26 = (tmp24 - tmp25)>>2;
	  tmp50 = (tmp24 + tmp25)>>2;
	  {
	       fftw_real tmp27;
	       fftw_real tmp28;
	       fftw_real tmp14;
	       fftw_real tmp15;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp27 = c_im(input[5 * istride]);
	       tmp28 = c_im(input[6 * istride]);
	       tmp29 = (tmp27 - tmp28)>>2;
	       tmp51 = (tmp27 + tmp28)>>2;
	       tmp14 = c_re(input[5 * istride]);
	       tmp15 = c_re(input[6 * istride]);
	       tmp16 = (tmp14 + tmp15)>>2;
	       tmp44 = (tmp15 - tmp14)>>2;
	  }
     }
     {
	  fftw_real tmp35;
	  fftw_real tmp34;
	  fftw_real tmp59;
	  fftw_real tmp60;
	  ASSERT_ALIGNED_DOUBLE;
	  c_re(output[0]) = tmp1 + tmp4 + tmp7 + tmp10 + tmp13 + tmp16;
	  {
	       fftw_real tmp41;
	       fftw_real tmp40;
	       fftw_real tmp37;
	       fftw_real tmp36;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp41 = FFT_M(K281732556 , tmp20) + FFT_M(K755749574 , tmp23) + FFT_M(K989821441 , tmp29) - FFT_M(K909631995 , tmp26) - FFT_M(K540640817 , tmp32);
	       tmp40 = tmp1 + FFT_M(K841253532 , tmp7) + FFT_M(K415415013 , tmp13) - FFT_M(K142314838 , tmp16) - FFT_M(K654860733 , tmp10) - FFT_M(K959492973 , tmp4);
	       c_re(output[6 * ostride]) = tmp40 - tmp41;
	       c_re(output[5 * ostride]) = tmp40 + tmp41;
	       tmp37 = FFT_M(K540640817 , tmp20) + FFT_M(K909631995 , tmp32) + FFT_M(K989821441 , tmp23) + FFT_M(K755749574 , tmp26) + FFT_M(K281732556 , tmp29);
	       tmp36 = tmp1 + FFT_M(K841253532 , tmp4) + FFT_M(K415415013 , tmp7) - FFT_M(K959492973 , tmp16) - FFT_M(K654860733 , tmp13) - FFT_M(K142314838 , tmp10);
	       c_re(output[10 * ostride]) = tmp36 - tmp37;
	       c_re(output[ostride]) = tmp36 + tmp37;
	  }
	  tmp35 = FFT_M(K909631995 , tmp20) + FFT_M(K755749574 , tmp32) - FFT_M(K540640817 , tmp29) - FFT_M(K989821441 , tmp26) - FFT_M(K281732556 , tmp23);
	  tmp34 = tmp1 + FFT_M(K415415013 , tmp4) + FFT_M(K841253532 , tmp16) - FFT_M(K142314838 , tmp13) - FFT_M(K959492973 , tmp10) - FFT_M(K654860733 , tmp7);
	  c_re(output[9 * ostride]) = tmp34 - tmp35;
	  c_re(output[2 * ostride]) = tmp34 + tmp35;
	  {
	       fftw_real tmp39;
	       fftw_real tmp38;
	       fftw_real tmp33;
	       fftw_real tmp17;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp39 = FFT_M(K989821441 , tmp20) + FFT_M(K540640817 , tmp26) + FFT_M(K755749574 , tmp29) - FFT_M(K909631995 , tmp23) - FFT_M(K281732556 , tmp32);
	       tmp38 = tmp1 + FFT_M(K415415013 , tmp10) + FFT_M(K841253532 , tmp13) - FFT_M(K654860733 , tmp16) - FFT_M(K959492973 , tmp7) - FFT_M(K142314838 , tmp4);
	       c_re(output[8 * ostride]) = tmp38 - tmp39;
	       c_re(output[3 * ostride]) = tmp38 + tmp39;
	       tmp33 = FFT_M(K755749574 , tmp20) + FFT_M(K540640817 , tmp23) + FFT_M(K281732556 , tmp26) - FFT_M(K909631995 , tmp29) - FFT_M(K989821441 , tmp32);
	       tmp17 = tmp1 + FFT_M(K841253532 , tmp10) + FFT_M(K415415013 , tmp16) - FFT_M(K959492973 , tmp13) - FFT_M(K142314838 , tmp7) - FFT_M(K654860733 , tmp4);
	       c_re(output[7 * ostride]) = tmp17 - tmp33;
	       c_re(output[4 * ostride]) = tmp17 + tmp33;
	  }
	  c_im(output[0]) = tmp48 + tmp53 + tmp49 + tmp52 + tmp50 + tmp51;
	  {
	       fftw_real tmp47;
	       fftw_real tmp54;
	       fftw_real tmp57;
	       fftw_real tmp58;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp47 = FFT_M(K281732556 , tmp42) + FFT_M(K755749574 , tmp43) + FFT_M(K989821441 , tmp44) - FFT_M(K909631995 , tmp45) - FFT_M(K540640817 , tmp46);
	       tmp54 = tmp48 + FFT_M(K841253532 , tmp49) + FFT_M(K415415013 , tmp50) - FFT_M(K142314838 , tmp51) - FFT_M(K654860733 , tmp52) - FFT_M(K959492973 , tmp53);
	       c_im(output[5 * ostride]) = tmp47 + tmp54;
	       c_im(output[6 * ostride]) = tmp54 - tmp47;
	       tmp57 = FFT_M(K540640817 , tmp42) + FFT_M(K909631995 , tmp46) + FFT_M(K989821441 , tmp43) + FFT_M(K755749574 , tmp45) + FFT_M(K281732556 , tmp44);
	       tmp58 = tmp48 + FFT_M(K841253532 , tmp53) + FFT_M(K415415013 , tmp49) - FFT_M(K959492973 , tmp51) - FFT_M(K654860733 , tmp50) - FFT_M(K142314838 , tmp52);
	       c_im(output[ostride]) = tmp57 + tmp58;
	       c_im(output[10 * ostride]) = tmp58 - tmp57;
	  }
	  tmp59 = FFT_M(K909631995 , tmp42) + FFT_M(K755749574 , tmp46) - FFT_M(K540640817 , tmp44) - FFT_M(K989821441 , tmp45) - FFT_M(K281732556 , tmp43);
	  tmp60 = tmp48 + FFT_M(K415415013 , tmp53) + FFT_M(K841253532 , tmp51) - FFT_M(K142314838 , tmp50) - FFT_M(K959492973 , tmp52) - FFT_M(K654860733 , tmp49);
	  c_im(output[2 * ostride]) = tmp59 + tmp60;
	  c_im(output[9 * ostride]) = tmp60 - tmp59;
	  {
	       fftw_real tmp55;
	       fftw_real tmp56;
	       fftw_real tmp61;
	       fftw_real tmp62;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp55 = FFT_M(K989821441 , tmp42) + FFT_M(K540640817 , tmp45) + FFT_M(K755749574 , tmp44) - FFT_M(K909631995 , tmp43) - FFT_M(K281732556 , tmp46);
	       tmp56 = tmp48 + FFT_M(K415415013 , tmp52) + FFT_M(K841253532 , tmp50) - FFT_M(K654860733 , tmp51) - FFT_M(K959492973 , tmp49) - FFT_M(K142314838 , tmp53);
	       c_im(output[3 * ostride]) = tmp55 + tmp56;
	       c_im(output[8 * ostride]) = tmp56 - tmp55;
	       tmp61 = FFT_M(K755749574 , tmp42) + FFT_M(K540640817 , tmp43) + FFT_M(K281732556 , tmp45) - FFT_M(K909631995 , tmp44) - FFT_M(K989821441 , tmp46);
	       tmp62 = tmp48 + FFT_M(K841253532 , tmp52) + FFT_M(K415415013 , tmp51) - FFT_M(K959492973 , tmp50) - FFT_M(K142314838 , tmp49) - FFT_M(K654860733 , tmp53);
	       c_im(output[4 * ostride]) = tmp61 + tmp62;
	       c_im(output[7 * ostride]) = tmp62 - tmp61;
	  }
     }
}

void CFourier::fftwi_no_twiddle_11(const fftw_complex *input, fftw_complex *output, tNativeAcc istride, tNativeAcc ostride)
{
	 fftw_real tmp1;
     fftw_real tmp23;
     fftw_real tmp4;
     fftw_real tmp17;
     fftw_real tmp38;
     fftw_real tmp49;
     fftw_real tmp26;
     fftw_real tmp53;
     fftw_real tmp7;
     fftw_real tmp21;
     fftw_real tmp10;
     fftw_real tmp18;
     fftw_real tmp35;
     fftw_real tmp50;
     fftw_real tmp13;
     fftw_real tmp20;
     fftw_real tmp29;
     fftw_real tmp51;
     fftw_real tmp32;
     fftw_real tmp52;
     fftw_real tmp16;
     fftw_real tmp19;
     ASSERT_ALIGNED_DOUBLE;
     {
	  fftw_real tmp2;
	  fftw_real tmp3;
	  fftw_real tmp36;
	  fftw_real tmp37;
	  ASSERT_ALIGNED_DOUBLE;
	  tmp1 = c_re(input[0]);
	  tmp23 = c_im(input[0]);
	  tmp2 = c_re(input[istride]);
	  tmp3 = c_re(input[10 * istride]);
	  tmp4 = tmp2 + tmp3;
	  tmp17 = tmp2 - tmp3;
	  tmp36 = c_im(input[istride]);
	  tmp37 = c_im(input[10 * istride]);
	  tmp38 = tmp36 + tmp37;
	  tmp49 = tmp37 - tmp36;
	  {
	       fftw_real tmp24;
	       fftw_real tmp25;
	       fftw_real tmp5;
	       fftw_real tmp6;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp24 = c_im(input[2 * istride]);
	       tmp25 = c_im(input[9 * istride]);
	       tmp26 = tmp24 + tmp25;
	       tmp53 = tmp25 - tmp24;
	       tmp5 = c_re(input[2 * istride]);
	       tmp6 = c_re(input[9 * istride]);
	       tmp7 = tmp5 + tmp6;
	       tmp21 = tmp5 - tmp6;
	  }
     }
     {
	  fftw_real tmp8;
	  fftw_real tmp9;
	  fftw_real tmp27;
	  fftw_real tmp28;
	  ASSERT_ALIGNED_DOUBLE;
	  tmp8 = c_re(input[3 * istride]);
	  tmp9 = c_re(input[8 * istride]);
	  tmp10 = tmp8 + tmp9;
	  tmp18 = tmp8 - tmp9;
	  {
	       fftw_real tmp33;
	       fftw_real tmp34;
	       fftw_real tmp11;
	       fftw_real tmp12;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp33 = c_im(input[3 * istride]);
	       tmp34 = c_im(input[8 * istride]);
	       tmp35 = tmp33 + tmp34;
	       tmp50 = tmp34 - tmp33;
	       tmp11 = c_re(input[4 * istride]);
	       tmp12 = c_re(input[7 * istride]);
	       tmp13 = tmp11 + tmp12;
	       tmp20 = tmp11 - tmp12;
	  }
	  tmp27 = c_im(input[4 * istride]);
	  tmp28 = c_im(input[7 * istride]);
	  tmp29 = tmp27 + tmp28;
	  tmp51 = tmp28 - tmp27;
	  {
	       fftw_real tmp30;
	       fftw_real tmp31;
	       fftw_real tmp14;
	       fftw_real tmp15;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp30 = c_im(input[5 * istride]);
	       tmp31 = c_im(input[6 * istride]);
	       tmp32 = tmp30 + tmp31;
	       tmp52 = tmp31 - tmp30;
	       tmp14 = c_re(input[5 * istride]);
	       tmp15 = c_re(input[6 * istride]);
	       tmp16 = tmp14 + tmp15;
	       tmp19 = tmp14 - tmp15;
	  }
     }
     {
	  fftw_real tmp56;
	  fftw_real tmp55;
	  fftw_real tmp44;
	  fftw_real tmp45;
	  ASSERT_ALIGNED_DOUBLE;
	  c_re(output[0]) = tmp1 + tmp4 + tmp7 + tmp10 + tmp13 + tmp16;
	  {
	       fftw_real tmp62;
	       fftw_real tmp61;
	       fftw_real tmp58;
	       fftw_real tmp57;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp62 = FFT_M(K281732556 , tmp49) + FFT_M(K755749574 , tmp50) + FFT_M(K989821441 , tmp52) - FFT_M(K909631995 , tmp51) - FFT_M(K540640817 , tmp53);
	       tmp61 = tmp1 + FFT_M(K841253532 , tmp7) + FFT_M(K415415013 , tmp13) - FFT_M(K142314838 , tmp16) - FFT_M(K654860733 , tmp10) - FFT_M(K959492973 , tmp4);
	       c_re(output[6 * ostride]) = tmp61 - tmp62;
	       c_re(output[5 * ostride]) = tmp61 + tmp62;
	       tmp58 = FFT_M(K540640817 , tmp49) + FFT_M(K909631995 , tmp53) + FFT_M(K989821441 , tmp50) + FFT_M(K755749574 , tmp51) + FFT_M(K281732556 , tmp52);
	       tmp57 = tmp1 + FFT_M(K841253532 , tmp4) + FFT_M(K415415013 , tmp7) - FFT_M(K959492973 , tmp16) - FFT_M(K654860733 , tmp13) - FFT_M(K142314838 , tmp10);
	       c_re(output[10 * ostride]) = tmp57 - tmp58;
	       c_re(output[ostride]) = tmp57 + tmp58;
	  }
	  tmp56 = FFT_M(K909631995 , tmp49) + FFT_M(K755749574 , tmp53) - FFT_M(K540640817 , tmp52) - FFT_M(K989821441 , tmp51) - FFT_M(K281732556 , tmp50);
	  tmp55 = tmp1 + FFT_M(K415415013 , tmp4) + FFT_M(K841253532 , tmp16) - FFT_M(K142314838 , tmp13) - FFT_M(K959492973 , tmp10) - FFT_M(K654860733 , tmp7);
	  c_re(output[9 * ostride]) = tmp55 - tmp56;
	  c_re(output[2 * ostride]) = tmp55 + tmp56;
	  {
	       fftw_real tmp60;
	       fftw_real tmp59;
	       fftw_real tmp54;
	       fftw_real tmp48;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp60 = FFT_M(K989821441 , tmp49) + FFT_M(K540640817 , tmp51) + FFT_M(K755749574 , tmp52) - FFT_M(K909631995 , tmp50) - FFT_M(K281732556 , tmp53);
	       tmp59 = tmp1 + FFT_M(K415415013 , tmp10) + FFT_M(K841253532 , tmp13) - FFT_M(K654860733 , tmp16) - FFT_M(K959492973 , tmp7) - FFT_M(K142314838 , tmp4);
	       c_re(output[8 * ostride]) = tmp59 - tmp60;
	       c_re(output[3 * ostride]) = tmp59 + tmp60;
	       tmp54 = FFT_M(K755749574 , tmp49) + FFT_M(K540640817 , tmp50) + FFT_M(K281732556 , tmp51) - FFT_M(K909631995 , tmp52) - FFT_M(K989821441 , tmp53);
	       tmp48 = tmp1 + FFT_M(K841253532 , tmp10) + FFT_M(K415415013 , tmp16) - FFT_M(K959492973 , tmp13) - FFT_M(K142314838 , tmp7) - FFT_M(K654860733 , tmp4);
	       c_re(output[7 * ostride]) = tmp48 - tmp54;
	       c_re(output[4 * ostride]) = tmp48 + tmp54;
	  }
	  c_im(output[0]) = tmp23 + tmp38 + tmp26 + tmp35 + tmp29 + tmp32;
	  {
	       fftw_real tmp22;
	       fftw_real tmp39;
	       fftw_real tmp42;
	       fftw_real tmp43;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp22 = FFT_M(K281732556 , tmp17) + FFT_M(K755749574 , tmp18) + FFT_M(K989821441 , tmp19) - FFT_M(K909631995 , tmp20) - FFT_M(K540640817 , tmp21);
	       tmp39 = tmp23 + FFT_M(K841253532 , tmp26) + FFT_M(K415415013 , tmp29) - FFT_M(K142314838 , tmp32) - FFT_M(K654860733 , tmp35) - FFT_M(K959492973 , tmp38);
	       c_im(output[5 * ostride]) = tmp22 + tmp39;
	       c_im(output[6 * ostride]) = tmp39 - tmp22;
	       tmp42 = FFT_M(K540640817 , tmp17) + FFT_M(K909631995 , tmp21) + FFT_M(K989821441 , tmp18) + FFT_M(K755749574 , tmp20) + FFT_M(K281732556 , tmp19);
	       tmp43 = tmp23 + FFT_M(K841253532 , tmp38) + FFT_M(K415415013 , tmp26) - FFT_M(K959492973 , tmp32) - FFT_M(K654860733 , tmp29) - FFT_M(K142314838 , tmp35);
	       c_im(output[ostride]) = tmp42 + tmp43;
	       c_im(output[10 * ostride]) = tmp43 - tmp42;
	  }
	  tmp44 = FFT_M(K909631995 , tmp17) + FFT_M(K755749574 , tmp21) - FFT_M(K540640817 , tmp19) - FFT_M(K989821441 , tmp20) - FFT_M(K281732556 , tmp18);
	  tmp45 = tmp23 + FFT_M(K415415013 , tmp38) + FFT_M(K841253532 , tmp32) - FFT_M(K142314838 , tmp29) - FFT_M(K959492973 , tmp35) - FFT_M(K654860733 , tmp26);
	  c_im(output[2 * ostride]) = tmp44 + tmp45;
	  c_im(output[9 * ostride]) = tmp45 - tmp44;
	  {
	       fftw_real tmp40;
	       fftw_real tmp41;
	       fftw_real tmp46;
	       fftw_real tmp47;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp40 = FFT_M(K989821441 , tmp17) + FFT_M(K540640817 , tmp20) + FFT_M(K755749574 , tmp19) - FFT_M(K909631995 , tmp18) - FFT_M(K281732556 , tmp21);
	       tmp41 = tmp23 + FFT_M(K415415013 , tmp35) + FFT_M(K841253532 , tmp29) - FFT_M(K654860733 , tmp32) - FFT_M(K959492973 , tmp26) - FFT_M(K142314838 , tmp38);
	       c_im(output[3 * ostride]) = tmp40 + tmp41;
	       c_im(output[8 * ostride]) = tmp41 - tmp40;
	       tmp46 = FFT_M(K755749574 , tmp17) + FFT_M(K540640817 , tmp18) + FFT_M(K281732556 , tmp20) - FFT_M(K909631995 , tmp19) - FFT_M(K989821441 , tmp21);
	       tmp47 = tmp23 + FFT_M(K841253532 , tmp35) + FFT_M(K415415013 , tmp32) - FFT_M(K959492973 , tmp29) - FFT_M(K142314838 , tmp26) - FFT_M(K654860733 , tmp38);
	       c_im(output[4 * ostride]) = tmp46 + tmp47;
	       c_im(output[7 * ostride]) = tmp47 - tmp46;
	  }
     }
}

void CFourier::fftw_no_twiddle_4(const fftw_complex *input, fftw_complex *output, tNativeAcc istride, tNativeAcc ostride)
{
     fftw_real tmp3;
     fftw_real tmp11;
     fftw_real tmp9;
     fftw_real tmp15;
     fftw_real tmp6;
     fftw_real tmp10;
     fftw_real tmp14;
     fftw_real tmp16;
     ASSERT_ALIGNED_DOUBLE;
     {
	  fftw_real tmp1;
	  fftw_real tmp2;
	  fftw_real tmp7;
	  fftw_real tmp8;
	  ASSERT_ALIGNED_DOUBLE;
	  tmp1 = c_re(input[0]);
	  tmp2 = c_re(input[2 * istride]);
	  tmp3 = (tmp1 + tmp2)>>1;
	  tmp11 = (tmp1 - tmp2)>>1;
	  tmp7 = c_im(input[0]);
	  tmp8 = c_im(input[2 * istride]);
	  tmp9 = (tmp7 - tmp8)>>1;
	  tmp15 = (tmp7 + tmp8)>>1;
     }
     {
	  fftw_real tmp4;
	  fftw_real tmp5;
	  fftw_real tmp12;
	  fftw_real tmp13;
	  ASSERT_ALIGNED_DOUBLE;
	  tmp4 = c_re(input[istride]);
	  tmp5 = c_re(input[3 * istride]);
	  tmp6 = (tmp4 + tmp5)>>1;
	  tmp10 = (tmp4 - tmp5)>>1;
	  tmp12 = c_im(input[istride]);
	  tmp13 = c_im(input[3 * istride]);
	  tmp14 = (tmp12 - tmp13)>>1;
	  tmp16 = (tmp12 + tmp13)>>1;
     }
     c_re(output[2 * ostride]) = (tmp3 - tmp6);
     c_re(output[0]) = (tmp3 + tmp6);
     c_im(output[ostride]) = (tmp9 - tmp10);
     c_im(output[3 * ostride]) = (tmp10 + tmp9);
     c_re(output[3 * ostride]) = (tmp11 - tmp14);
     c_re(output[ostride]) = (tmp11 + tmp14);
     c_im(output[2 * ostride]) = (tmp15 - tmp16);
     c_im(output[0]) = (tmp15 + tmp16);
}


void CFourier::fftw_no_twiddle_2(const fftw_complex *input, fftw_complex *output,tNativeAcc istride, tNativeAcc ostride)
{
     fftw_real tmp1;
     fftw_real tmp2;
     fftw_real tmp3;
     fftw_real tmp4;
     ASSERT_ALIGNED_DOUBLE;
     tmp1 = c_re(input[0]);
     tmp2 = c_re(input[istride]);
     c_re(output[ostride]) = (tmp1 - tmp2)>>1;
     c_re(output[0]) = (tmp1 + tmp2)>>1;
     tmp3 = c_im(input[0]);
     tmp4 = c_im(input[istride]);
     c_im(output[ostride]) = (tmp3 - tmp4)>>1;
     c_im(output[0]) = (tmp3 + tmp4)>>1;
}


//***************************************************************************
//*
//*			Function name : *** GenerateTwiddles ***
//*
//*			Description : Generates the twiddle values for the FFTs and IFFTs
//*
//*
//***************************************************************************

fftw_complex* CFourier::GenerateTwiddles(tNativeAcc n, tNativeAcc length)
{
//***************************************************************************
//*			*** GenerateTwiddles ***     Fixed Point Version 
//***************************************************************************
//#else
//TODO convert this one properly	
	tNative temp_re,temp_im;
	tNativeAcc r1=n/length-1;
	tNativeAcc m_alloc=length;
	tNativeAcc m=m_alloc;
	tNative i,j;
	tNativeAcc twiddle_order[]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

	FFTW_TRIG_REAL twoPiOverN = FFTW_K2PI / (FFTW_TRIG_REAL) n;
		
	fftw_complex* W = (fftw_complex *) _cache_malloc(r1 * m_alloc * sizeof(fftw_complex), -1);
	
	tNative istart=0;
	 

	  for (i = istart; i < m; ++i)
	       for (j = 0; j < r1; ++j) 
		   {
				tNativeAcc k = (i - istart) * r1 + j;
				FFTW_TRIG_REAL
				ij = (FFTW_TRIG_REAL) (i * twiddle_order[j]);
				temp_re = (short)(FFT_ONE*FFTW_TRIG_COS(twoPiOverN * ij));
				c_re(W[k]) = temp_re;
				temp_im = (short)(FFT_ONE*FFTW_FORWARD * FFTW_TRIG_SIN(twoPiOverN * ij));
				c_im(W[k]) = temp_im;
	       }
			
	return W;
}


//***************************************************************************
//*
//*			Function name : *** fft576 ***
//*
//*			Description : Performs a 576 taps Fast Fourier Transform
//*
//*
//***************************************************************************

void CFourier::fft576(const fftw_complex *input, fftw_complex *output)
{
//***************************************************************************
//*			*** fft576 ***     Fixed Point Version 
//***************************************************************************


	tNative tran_len=576,tran_len1=4,tran_len2=9,tran_len3=16,i=0;
	tNative pass1=tran_len1 * tran_len2;



	for(tNative s=0;s<tran_len3;++s)
	{
		for(tNative t=0;t<tran_len2;++t)
		{
				fftw_no_twiddle_4(input+s+t*tran_len3,
					output+s*tran_len/tran_len3+t*tran_len1,tran_len2*tran_len3,1);
			
		}
		for(i=s*pass1;i<s*pass1+pass1;i++) // * Scale Down *
		{
			output[i].re = (output[i].re>>F576_PASS1);
			output[i].im = (output[i].im>>F576_PASS1);
		}
		fftw_twiddle_9(output+s*tran_len/tran_len3,W36_9,tran_len/(tran_len2*tran_len3),tran_len/(tran_len2*tran_len3),1);
	}
	for(i=0;i<tran_len;i++) // * Scale Down *
	{
		output[i].re = (output[i].re>>F576_PASS2);
		output[i].im = (output[i].im>>F576_PASS2);		
	}
	fftw_twiddle_16(output,W576_16,tran_len/tran_len3,tran_len/tran_len3,1);

}


//***************************************************************************
//*
//*			Function name : *** fft512 ***
//*
//*			Description : Performs a 512 taps Fast Fourier Transform
//*
//*
//***************************************************************************

void CFourier::fft512(const fftw_complex *input, fftw_complex *output)
{
//***************************************************************************
//*			*** fft512 ***     Fixed Point Version 
//***************************************************************************

	tNative tran_len=512,tran_len1=4,tran_len2=8,tran_len3=16,i=0;
	tNative pass1=tran_len1*tran_len2;

	for(tNative s=0;s<tran_len3;++s)
	{
		for(tNative t=0;t<tran_len2;++t)
		{	
				fftw_no_twiddle_4(input+s+t*tran_len3,
					output+s*tran_len/tran_len3+t*tran_len1,tran_len2*tran_len3,1);	
		}
		for(i=s*pass1;i<s*pass1+pass1;i++) // * Scale Down *
		{
			output[i].re = (output[i].re>>F512_PASS1);
			output[i].im = (output[i].im>>F512_PASS1);
		}
		fftw_twiddle_8(output+s*tran_len/tran_len3,W32_8,tran_len/(tran_len2*tran_len3),tran_len/(tran_len2*tran_len3),1);
	}
	for(i=0;i<tran_len;i++) // * Scale Down *
	{
		output[i].re = (output[i].re>>F512_PASS2);
		output[i].im = (output[i].im>>F512_PASS2);		
	}
	fftw_twiddle_16(output,W512_16,tran_len/tran_len3,tran_len/tran_len3,1);
}


//***************************************************************************
//*
//*			Function name : *** fft352 ***
//*
//*			Description : Performs a 352 taps Fast Fourier Transform
//*
//*
//***************************************************************************

void CFourier::fft352(const fftw_complex *input, fftw_complex *output)
{
	tNative tran_len=352,tran_len1=11,tran_len2=4,tran_len3=8,i=0;
	tNative pass1=tran_len1 * tran_len2;

	for(tNative s=0;s<tran_len3;++s)
	{
		for(tNative t=0;t<tran_len2;++t)
		{
				fftw_no_twiddle_11(input+s+t*tran_len3,
					output+s*tran_len/tran_len3+t*tran_len1,tran_len2*tran_len3,1);	
		}
		for(i=s*pass1;i<s*pass1+pass1;i++) // * Scale Down *
		{
			output[i].re = (output[i].re>>F352_PASS1);
			output[i].im = (output[i].im>>F352_PASS1);
		}
		fftw_twiddle_4(output+s*tran_len/tran_len3,W44_4,tran_len/(tran_len2*tran_len3),tran_len/(tran_len2*tran_len3),1);
	}
	for(i=0;i<tran_len;i++) // * Scale Down *
	{
		output[i].re = (output[i].re>>F352_PASS2);
		output[i].im = (output[i].im>>F352_PASS2);		
	}
	fftw_twiddle_8(output,W352_8,tran_len/tran_len3,tran_len/tran_len3,1);
}




//***************************************************************************
//*
//*			Function name : *** fft224 ***
//*
//*			Description : Performs a 224 taps Fast Fourier Transform
//*
//*
//***************************************************************************

void CFourier::fft224(const fftw_complex *input, fftw_complex *output)
{
//***************************************************************************
//*			*** fft224 ***     Fixed Point Version 
//***************************************************************************

	tNative tran_len=224,tran_len1=2,tran_len2=7,tran_len3=16,i=0;
	tNative pass1=tran_len1 * tran_len2;
	
	for(tNative s=0;s<tran_len3;++s)
	{
		for(tNative t=0;t<tran_len2;++t)
		{
				fftw_no_twiddle_2(input+s+t*tran_len3,
					output+s*tran_len/tran_len3+t*tran_len1,tran_len2*tran_len3,1);
		}
		for(i=s*pass1;i<s*pass1+pass1;i++) // * Scale Down *
		{
			output[i].re = (output[i].re>>F224_PASS1);
			output[i].im = (output[i].im>>F224_PASS1);
		}
		fftw_twiddle_7(output+s*tran_len/tran_len3,W14_7,tran_len/(tran_len2*tran_len3),tran_len/(tran_len2*tran_len3),1);
	}
	for(i=0;i<tran_len;i++) // * Scale Down *
	{
		output[i].re = (output[i].re>>F224_PASS2);
		output[i].im = (output[i].im>>F224_PASS2);		
	}
	fftw_twiddle_16(output,W224_16,tran_len/tran_len3,tran_len/tran_len3,1);
}


//***************************************************************************
//*
//*			Function name : *** ifft1024 ***
//*
//*			Description : Performs a 1024 taps Inverse Fast Fourier Transform
//*
//*
//***************************************************************************

void CFourier::ifft1024(const fftw_complex *input, fftw_complex *out)
{
	fftw_complex* restrict output=out;

	tNative tran_len=1024,tran_len1=16,tran_len2=8,tran_len3=8,i=0;
	tNative pass1=tran_len1 * tran_len2;

	for(tNative s=0;s<tran_len3;++s)
	{
		for(tNative t=0;t<tran_len2;++t)
		{
				fftwi_no_twiddle_16(input+s+t*tran_len3,
					output+s*tran_len/tran_len3+t*tran_len1,tran_len2*tran_len3,1);	
		}
#ifndef SCALE_DOWN_IFFT_OFF // *** Scale Down. Only Fixed Point Version ***
		for(i=s*pass1;i<s*pass1+pass1;i++)
		{
			output[i].re = (output[i].re>>IFFT_PASS1);
			output[i].im = (output[i].im>>IFFT_PASS1);
		}
#endif
		fftwi_twiddle_8(output+s*tran_len/tran_len3,W128_8,tran_len/(tran_len2*tran_len3),tran_len/(tran_len2*tran_len3),1);
	}
	fftwi_twiddle_8(output,W1024_8,tran_len/tran_len3,tran_len/tran_len3,1);
}


//***************************************************************************
//*
//*			Function name : *** ifft1152 ***
//*
//*			Description : Performs a 1152 taps Inverse Fast Fourier Transform
//*
//*
//***************************************************************************

void CFourier::ifft1152(const fftw_complex *input, fftw_complex *output)
{
	tNative tran_len=1152,tran_len1=16,tran_len2=8,tran_len3=9,i=0;
	tNative pass1=tran_len1 * tran_len2;

	for(tNative s=0;s<tran_len3;++s)
	{
		for(tNative t=0;t<tran_len2;++t)
		{
				fftwi_no_twiddle_16(input+s+t*tran_len3,
					output+s*tran_len/tran_len3+t*tran_len1,tran_len2*tran_len3,1);	
		}
#ifndef SCALE_DOWN_IFFT_OFF // *** Scale Down. Only Fixed Point Version ***
		for(i=s*pass1;i<s*pass1+pass1;i++) 
		{
			output[i].re = (output[i].re>>IFFT_PASS1);
			output[i].im = (output[i].im>>IFFT_PASS1);
		}
#endif
		fftwi_twiddle_8(output+s*tran_len/tran_len3,W128_8,tran_len/(tran_len2*tran_len3),tran_len/(tran_len2*tran_len3),1);
	}
	fftwi_twiddle_9(output,W1152_9,tran_len/tran_len3,tran_len/tran_len3,1);

}

//***************************************************************************
//*
//*			Function name : *** ifft576 ***
//*
//*			Description : Performs a 576 taps Inverse Fast Fourier Transform
//*
//*
//***************************************************************************

void CFourier::ifft576(const fftw_complex *input, fftw_complex *output)
{
	tNative tran_len=576,tran_len1=16,tran_len2=4,tran_len3=9,i=0;
	tNative pass1=tran_len1 * tran_len2;

	for(tNative s=0;s<tran_len3;++s)
	{
		for(tNative t=0;t<tran_len2;++t)
		{
				fftwi_no_twiddle_16(input+s+t*tran_len3,
					output+s*tran_len/tran_len3+t*tran_len1,tran_len2*tran_len3,1);
		}
#ifndef SCALE_DOWN_IFFT_OFF // *** Scale Down. Only Fixed Point Version ***
		for(i=s*pass1;i<s*pass1+pass1;i++)
		{
			output[i].re = (output[i].re>>IFFT_PASS1);
			output[i].im = (output[i].im>>IFFT_PASS1);
		}
#endif
		fftwi_twiddle_4(output+s*tran_len/tran_len3,W64_4,tran_len/(tran_len2*tran_len3),tran_len/(tran_len2*tran_len3),1);
	}
	fftwi_twiddle_9(output,W576_9,tran_len/tran_len3,tran_len/tran_len3,1);

}




//***************************************************************************
//*
//*			Function name : *** ifft704 ***
//*
//*			Description : Performs a 704 taps Inverse Fast Fourier Transform
//*
//*
//***************************************************************************

void CFourier::ifft704(const fftw_complex *input, fftw_complex *output)
{
	tNative tran_len=704,tran_len1=11,tran_len2=8,tran_len3=8,i=0;
	tNative pass1=tran_len1 * tran_len2;

	for(tNative s=0;s<tran_len3;++s)
	{
		for(tNative t=0;t<tran_len2;++t)
		{	
				fftwi_no_twiddle_11(input+s+t*tran_len3,
					output+s*tran_len/tran_len3+t*tran_len1,tran_len2*tran_len3,1);	
		}
#ifndef SCALE_DOWN_IFFT_OFF // *** Scale Down. Only Fixed Point Version ***
		for(i=s*pass1;i<s*pass1+pass1;i++)
		{
			output[i].re = (output[i].re>>IFFT_PASS1);
			output[i].im = (output[i].im>>IFFT_PASS1);
		}
#endif
		fftwi_twiddle_8(output+s*tran_len/tran_len3,W88_8,tran_len/(tran_len2*tran_len3),tran_len/(tran_len2*tran_len3),1);
	}
	fftwi_twiddle_8(output,W704_8,tran_len/tran_len3,tran_len/tran_len3,1);
}
	


//***************************************************************************
//*
//*			Function name : *** ifft448 ***
//*
//*			Description : Performs a 448 taps Inverse Fast Fourier Transform
//*
//*
//***************************************************************************

void CFourier::ifft448(const fftw_complex *input, fftw_complex *output)
{
	tNative tran_len=448,tran_len1=16,tran_len2=7,tran_len3=4,i=0;
        tNative pass1=tran_len1 * tran_len2;


	for(tNative s=0;s<tran_len3;++s)
	{
		for(tNative t=0;t<tran_len2;++t)
		{	
				fftwi_no_twiddle_16(input+s+t*tran_len3,
					output+s*tran_len/tran_len3+t*tran_len1,tran_len2*tran_len3,1);	
		}
#ifndef SCALE_DOWN_IFFT_OFF // *** Scale Down. Only Fixed Point Version ***
                
		for(i=s*pass1;i<s*pass1+pass1;i++)
		{
			output[i].re = (output[i].re>>IFFT_PASS1);
			output[i].im = (output[i].im>>IFFT_PASS1);
		}
#endif
		fftwi_twiddle_7(output+s*tran_len/tran_len3,W112_7,tran_len/(tran_len2*tran_len3),tran_len/(tran_len2*tran_len3),1);
	}
	fftwi_twiddle_4(output,W448_4,tran_len/tran_len3,tran_len/tran_len3,1);
}



