/////////////////////////////////////////////////////////////////////////////
/*

NOTES:
	Copyright(C) Integrated Biometrics, 2013

VERSION HISTORY:
	16 December 2013 - Created
*/
/////////////////////////////////////////////////////////////////////////////

#ifndef _WinCEMath_30B53B42_BCC2_47aa_A6C6_1E216FD38845_
#define _WinCEMath_30B53B42_BCC2_47aa_A6C6_1E216FD38845_

// CE doesn't have single precision floating point, replace single precision 
// with double precision and cast back to single precision.
#define cosf(x) ((float)cos((double)(x)))
#define atan2f(x,y) ((float)atan2((double)(x),(double)(y)))
#define sinf(x) ((float)sin((double)(x)))
#define tanf(x) ((float)tan((double)(x)))
#define powf(x,y) ((float)pow((double)(x),(double)(y)))


#endif 