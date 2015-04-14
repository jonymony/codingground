
#if !defined(CWINTER_H)
#define CWINTER_H

#include "../bbc_types.h"


#define CWDETECT_ON

#define MAX_NO_INTERS 2

class CCWInter
{
public:

	CCWInter();
	virtual ~CCWInter();

	void Detect(tNative* data_in, tNative* data_out, tNative size, tNativeAcc* freq,
		tNative* bin_1, tNative* bin_2);

private:

	void CalculateFreqs(tNative* mag_store, tNative bin, tNative num, tNative size, tNativeAcc* freq);

	tNativeAcc m_old_freq[MAX_NO_INTERS];
};



#endif // !defined(CWINTER_H)
