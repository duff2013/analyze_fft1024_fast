/* ----------------------------------------------------------------------
 Copyright (C) 2010 ARM Limited. All rights reserved.
 
 $Date:        15. February 2012
 $Revision:   V1.1.0
 
 Project:      CMSIS DSP Library
 Title:      arm_cfft_radix4_q15.c
 
 Description:  This file has function definition of Radix-4 FFT & IFFT function and
 In-place bit reversal using bit reversal table
 
 Target Processor: Cortex-M4/Cortex-M3/Cortex-M0
 
 Version 1.1.0 2012/02/15
 Updated with more optimizations, bug fixes and minor API changes.
 
 Version 1.0.10 2011/7/15
 Big Endian support added and Merged M0 and M3/M4 Source code.
 
 Version 1.0.3 2010/11/29
 Re-organized the CMSIS folders and updated documentation.
 
 Version 1.0.2 2010/11/11
 Documentation updated.
 
 Version 1.0.1 2010/10/05
 Production release and review comments incorporated.
 
 Version 1.0.0 2010/09/20
 Production release and review comments incorporated.
 
 Version 0.0.5  2010/04/26
 incorporated review comments and updated with latest CMSIS layer
 
 Version 0.0.3  2010/03/10
 Initial version
 -------------------------------------------------------------------- */

// Teensy Audio Library adapted version. Colin Duffy

#include "arm_math.h"
/**
 @ingroup groupTransforms
 */

/**
 @addtogroup Radix4_CFFT_CIFFT
 @{
 */

/**
 @details
 @brief Processing function for the Q15 CFFT/CIFFT.
 @param[in]      *S    points to an instance of the Q15 CFFT/CIFFT structure.
 @param[in, out] *pSrc points to the complex data buffer. Processing occurs in-place.
 @return none.
 
 \par Input and output formats:
 \par
 Internally input is downscaled by 2 for every stage to avoid saturations inside CFFT/CIFFT process.
 Hence the output format is different for different FFT sizes.
 The input and output formats for different FFT sizes and number of bits to upscale are mentioned in the tables below for CFFT and CIFFT:
 \par
 \image html CFFTQ15.gif "Input and Output Formats for Q15 CFFT"
 \image html CIFFTQ15.gif "Input and Output Formats for Q15 CIFFT"
 */

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
inline void arm_radix4_butterfly_q15_stage1(q15_t * pSrc16, uint32_t fftLen, q15_t * pCoef16, uint32_t twidCoefModifier) __attribute__((always_inline, unused));

inline void arm_radix4_butterfly_q15_stage2(q15_t * pSrc16, uint32_t fftLen, q15_t * pCoef16, uint32_t twidCoefModifier) __attribute__((always_inline, unused));

inline void arm_radix4_butterfly_q15_stage3(q15_t * pSrc16, uint32_t fftLen, q15_t * pCoef16, uint32_t twidCoefModifier) __attribute__((always_inline, unused));

// ifft
inline void arm_radix4_butterfly_inverse_q15_stage1(q15_t * pSrc16, uint32_t fftLen, q15_t * pCoef16, uint32_t twidCoefModifier) __attribute__((always_inline, unused));

inline void arm_radix4_butterfly_inverse_q15_stage2(q15_t * pSrc16, uint32_t fftLen, q15_t * pCoef16, uint32_t twidCoefModifier) __attribute__((always_inline, unused));

inline void arm_radix4_butterfly_inverse_q15_stage3(q15_t * pSrc16, uint32_t fftLen, q15_t * pCoef16, uint32_t twidCoefModifier) __attribute__((always_inline, unused));

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
void arm_cfft_radix4_q15_stage1(const arm_cfft_radix4_instance_q15 * S, q15_t * pSrc) {
    if (S->ifftFlag == 1u) {
        /*  Complex IFFT radix-4  */
        arm_radix4_butterfly_inverse_q15_stage1(pSrc, S->fftLen, S->pTwiddle, S->twidCoefModifier);
    }
    else {
        /*  Complex FFT radix-4  */
        arm_radix4_butterfly_q15_stage1(pSrc, S->fftLen, S->pTwiddle, S->twidCoefModifier);
    }
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
void arm_cfft_radix4_q15_stage2(const arm_cfft_radix4_instance_q15 * S, q15_t * pSrc) {
    if (S->ifftFlag == 1u) {
        /*  Complex IFFT radix-4  */
        arm_radix4_butterfly_inverse_q15_stage2(pSrc, S->fftLen, S->pTwiddle, S->twidCoefModifier);
    }
    else {
        /*  Complex FFT radix-4  */
        arm_radix4_butterfly_q15_stage2(pSrc, S->fftLen, S->pTwiddle, S->twidCoefModifier);
    }
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
void arm_cfft_radix4_q15_stage3(const arm_cfft_radix4_instance_q15 * S, q15_t * pSrc) {
    if (S->ifftFlag == 1u) {
        /*  Complex IFFT radix-4  */
        arm_radix4_butterfly_inverse_q15_stage3(pSrc, S->fftLen, S->pTwiddle, S->twidCoefModifier);
    }
    else {
        /*  Complex FFT radix-4  */
        arm_radix4_butterfly_q15_stage3(pSrc, S->fftLen, S->pTwiddle, S->twidCoefModifier);
    }
    
    if (S->bitReverseFlag == 1u) {
        /*  Bit Reversal */
        arm_bitreversal_q15(pSrc, S->fftLen, S->bitRevFactor, S->pBitRevTable);
    }
}
/**
 @} end of Radix4_CFFT_CIFFT group
 */

/*
 Radix-4 FFT algorithm used is :
 
 Input real and imaginary data:
 x(n) = xa + j * ya
 x(n+N/4 ) = xb + j * yb
 x(n+N/2 ) = xc + j * yc
 x(n+3N 4) = xd + j * yd
 
 
 Output real and imaginary data:
 x(4r) = xa'+ j * ya'
 x(4r+1) = xb'+ j * yb'
 x(4r+2) = xc'+ j * yc'
 x(4r+3) = xd'+ j * yd'
 
 
 Twiddle factors for radix-4 FFT:
 Wn = co1 + j * (- si1)
 W2n = co2 + j * (- si2)
 W3n = co3 + j * (- si3)
 
 The real and imaginary output values for the radix-4 butterfly are
 xa' = xa + xb + xc + xd
 ya' = ya + yb + yc + yd
 xb' = (xa+yb-xc-yd)* co1 + (ya-xb-yc+xd)* (si1)
 yb' = (ya-xb-yc+xd)* co1 - (xa+yb-xc-yd)* (si1)
 xc' = (xa-xb+xc-xd)* co2 + (ya-yb+yc-yd)* (si2)
 yc' = (ya-yb+yc-yd)* co2 - (xa-xb+xc-xd)* (si2)
 xd' = (xa-yb-xc+yd)* co3 + (ya+xb-yc-xd)* (si3)
 yd' = (ya+xb-yc-xd)* co3 - (xa-yb-xc+yd)* (si3)
 
 */

/**
 @brief  Core function for the Q15 CFFT butterfly process.
 @param[in, out] *pSrc16          points to the in-place buffer of Q15 data type.
 @param[in]      fftLen           length of the FFT.
 @param[in]      *pCoef16         points to twiddle coefficient buffer.
 @param[in]      twidCoefModifier twiddle coefficient modifier that supports different size FFTs with the same twiddle factor table.
 @return none.
 */

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
void arm_radix4_butterfly_q15_all_stages(q15_t * pSrc16, uint32_t fftLen, q15_t * pCoef16, uint32_t twidCoefModifier) {
    /* Run the below code for Cortex-M4 and Cortex-M3   */
    /* Total process is divided into three stages       */
    /*****************************************************************************************/
    /* process first stage, middle stages, & last stage */
    /* Input is in 1.15(q15) format */
    arm_radix4_butterfly_q15_stage1(pSrc16, fftLen, pCoef16, twidCoefModifier);
    /* data is in 4.11(q11) format */
    /* end of first stage process  */
    
    /*****************************************************************************************/
    /* start of middle stage process */
    arm_radix4_butterfly_q15_stage2(pSrc16, fftLen, pCoef16, twidCoefModifier);
    /* end of middle stage process */
    /* data is in 10.6(q6) format for the 1024 point */
    /* data is in 8.8(q8) format for the 256 point   */
    /* data is in 6.10(q10) format for the 64 point  */
    /* data is in 4.12(q12) format for the 16 point  */
    
    /*****************************************************************************************/
    /*  Initializations for the last stage */
    arm_radix4_butterfly_q15_stage3(pSrc16, fftLen, pCoef16, twidCoefModifier);
    /* end of last stage process */
    /* output is in 11.5(q5) format for the 1024 point */
    /* output is in 9.7(q7) format for the 256 point   */
    /* output is in 7.9(q9) format for the 64 point    */
    /* output is in 5.11(q11) format for the 16 point  */
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
inline void arm_radix4_butterfly_q15_stage1(q15_t * pSrc16, uint32_t fftLen, q15_t * pCoef16, uint32_t twidCoefModifier) {
    /*  start of first stage process */
#ifndef ARM_MATH_CM0
    /*  Initializations for the first stage */
    q31_t R, S, T, U;
    q31_t C1, C2, C3, out1, out2;
    q15_t in;
    
    uint32_t i3, i2, i1, i0, ic, n2, n1, j;
    
    n2 = fftLen;
    n1 = n2;
    
    /* n2 = fftLen/4 */
    n2 >>= 2u;
    
    /* Index for twiddle coefficient */
    ic = 0u;
    
    /* Index for input read and output write */
    i0 = 0u;
    j = n2;
    
    do
    {
        /*  Butterfly implementation */
        
        /*  index calculation for the input as, */
        /*  pSrc16[i0 + 0], pSrc16[i0 + fftLen/4], pSrc16[i0 + fftLen/2], pSrc16[i0 + 3fftLen/4] */
        i1 = i0 + n2;
        i2 = i1 + n2;
        i3 = i2 + n2;
        
        /*  Reading i0, i0+fftLen/2 inputs */
        /* Read ya (real), xa(imag) input */
        T = _SIMD32_OFFSET(pSrc16 + (2u * i0));
        in = ((int16_t) (T & 0xFFFF)) >> 2;
        T = ((T >> 2) & 0xFFFF0000) | (in & 0xFFFF);
        
        /* Read yc (real), xc(imag) input */
        S = _SIMD32_OFFSET(pSrc16 + (2u * i2));
        in = ((int16_t) (S & 0xFFFF)) >> 2;
        S = ((S >> 2) & 0xFFFF0000) | (in & 0xFFFF);
        
        /* R = packed((ya + yc), (xa + xc) ) */
        R = __QADD16(T, S);
        
        /* S = packed((ya - yc), (xa - xc) ) */
        S = __QSUB16(T, S);
        
        /*  Reading i0+fftLen/4 , i0+3fftLen/4 inputs */
        /* Read yb (real), xb(imag) input */
        T = _SIMD32_OFFSET(pSrc16 + (2u * i1));
        in = ((int16_t) (T & 0xFFFF)) >> 2;
        T = ((T >> 2) & 0xFFFF0000) | (in & 0xFFFF);
        
        /* Read yd (real), xd(imag) input */
        U = _SIMD32_OFFSET(pSrc16 + (2u * i3));
        in = ((int16_t) (U & 0xFFFF)) >> 2;
        U = ((U >> 2) & 0xFFFF0000) | (in & 0xFFFF);
        
        /* T = packed((yb + yd), (xb + xd) ) */
        T = __QADD16(T, U);
        
        /*  writing the butterfly processed i0 sample */
        /* xa' = xa + xb + xc + xd */
        /* ya' = ya + yb + yc + yd */
        _SIMD32_OFFSET(pSrc16 + (2u * i0)) = __SHADD16(R, T);
        
        /* R = packed((ya + yc) - (yb + yd), (xa + xc)- (xb + xd)) */
        R = __QSUB16(R, T);
        
        /* co2 & si2 are read from SIMD Coefficient pointer */
        C2 = _SIMD32_OFFSET(pCoef16 + (4u * ic));
        
#ifndef ARM_MATH_BIG_ENDIAN
        
        /* xc' = (xa-xb+xc-xd)* co2 + (ya-yb+yc-yd)* (si2) */
        out1 = __SMUAD(C2, R) >> 16u;
        /* yc' = (ya-yb+yc-yd)* co2 - (xa-xb+xc-xd)* (si2) */
        out2 = __SMUSDX(C2, R);
        
#else
        
        /* xc' = (ya-yb+yc-yd)* co2 - (xa-xb+xc-xd)* (si2) */
        out1 = __SMUSDX(R, C2) >> 16u;
        /* yc' = (xa-xb+xc-xd)* co2 + (ya-yb+yc-yd)* (si2) */
        out2 = __SMUAD(C2, R);
        
#endif /*      #ifndef ARM_MATH_BIG_ENDIAN     */
        
        /*  Reading i0+fftLen/4 */
        /* T = packed(yb, xb) */
        T = _SIMD32_OFFSET(pSrc16 + (2u * i1));
        in = ((int16_t) (T & 0xFFFF)) >> 2;
        T = ((T >> 2) & 0xFFFF0000) | (in & 0xFFFF);
        
        /* writing the butterfly processed i0 + fftLen/4 sample */
        /* writing output(xc', yc') in little endian format */
        _SIMD32_OFFSET(pSrc16 + (2u * i1)) =
        (q31_t) ((out2) & 0xFFFF0000) | (out1 & 0x0000FFFF);
        
        /*  Butterfly calculations */
        /* U = packed(yd, xd) */
        U = _SIMD32_OFFSET(pSrc16 + (2u * i3));
        in = ((int16_t) (U & 0xFFFF)) >> 2;
        U = ((U >> 2) & 0xFFFF0000) | (in & 0xFFFF);
        
        /* T = packed(yb-yd, xb-xd) */
        T = __QSUB16(T, U);
        
#ifndef ARM_MATH_BIG_ENDIAN
        
        /* R = packed((ya-yc) + (xb- xd) , (xa-xc) - (yb-yd)) */
        R = __QASX(S, T);
        /* S = packed((ya-yc) - (xb- xd),  (xa-xc) + (yb-yd)) */
        S = __QSAX(S, T);
        
#else
        
        /* R = packed((ya-yc) + (xb- xd) , (xa-xc) - (yb-yd)) */
        R = __QSAX(S, T);
        /* S = packed((ya-yc) - (xb- xd),  (xa-xc) + (yb-yd)) */
        S = __QASX(S, T);
        
#endif /*      #ifndef ARM_MATH_BIG_ENDIAN     */
        
        /* co1 & si1 are read from SIMD Coefficient pointer */
        C1 = _SIMD32_OFFSET(pCoef16 + (2u * ic));
        /*  Butterfly process for the i0+fftLen/2 sample */
        
#ifndef ARM_MATH_BIG_ENDIAN
        
        /* xb' = (xa+yb-xc-yd)* co1 + (ya-xb-yc+xd)* (si1) */
        out1 = __SMUAD(C1, S) >> 16u;
        /* yb' = (ya-xb-yc+xd)* co1 - (xa+yb-xc-yd)* (si1) */
        out2 = __SMUSDX(C1, S);
        
#else
        
        /* xb' = (ya-xb-yc+xd)* co1 - (xa+yb-xc-yd)* (si1) */
        out1 = __SMUSDX(S, C1) >> 16u;
        /* yb' = (xa+yb-xc-yd)* co1 + (ya-xb-yc+xd)* (si1) */
        out2 = __SMUAD(C1, S);
        
#endif /*      #ifndef ARM_MATH_BIG_ENDIAN     */
        
        /* writing output(xb', yb') in little endian format */
        _SIMD32_OFFSET(pSrc16 + (2u * i2)) =
        ((out2) & 0xFFFF0000) | ((out1) & 0x0000FFFF);
        
        
        /* co3 & si3 are read from SIMD Coefficient pointer */
        C3 = _SIMD32_OFFSET(pCoef16 + (6u * ic));
        /*  Butterfly process for the i0+3fftLen/4 sample */
        
#ifndef ARM_MATH_BIG_ENDIAN
        
        /* xd' = (xa-yb-xc+yd)* co3 + (ya+xb-yc-xd)* (si3) */
        out1 = __SMUAD(C3, R) >> 16u;
        /* yd' = (ya+xb-yc-xd)* co3 - (xa-yb-xc+yd)* (si3) */
        out2 = __SMUSDX(C3, R);
        
#else
        
        /* xd' = (ya+xb-yc-xd)* co3 - (xa-yb-xc+yd)* (si3) */
        out1 = __SMUSDX(R, C3) >> 16u;
        /* yd' = (xa-yb-xc+yd)* co3 + (ya+xb-yc-xd)* (si3) */
        out2 = __SMUAD(C3, R);
        
#endif /*      #ifndef ARM_MATH_BIG_ENDIAN     */
        
        /* writing output(xd', yd') in little endian format */
        _SIMD32_OFFSET(pSrc16 + (2u * i3)) =
        ((out2) & 0xFFFF0000) | (out1 & 0x0000FFFF);
        
        /*  Twiddle coefficients index modifier */
        ic = ic + twidCoefModifier;
        
        /*  Updating input index */
        i0 = i0 + 1u;
        
    } while (--j);
#else
    
#endif /* #ifndef ARM_MATH_CM0 */
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
inline void arm_radix4_butterfly_q15_stage2(q15_t * pSrc16, uint32_t fftLen, q15_t * pCoef16, uint32_t twidCoefModifier) {
#ifndef ARM_MATH_CM0
    /*  Initializations for the middle stage */
    q31_t R, S, T, U;
    q31_t C1, C2, C3, out1, out2;
    q15_t in;
    uint32_t i3, i2, i1, i0, ic, n2, n1, k, j;
    
    n2 = fftLen;
    /* n2 = fftLen/4 */
    n2 >>= 2u;
    
    /*  Twiddle coefficients index modifier */
    twidCoefModifier <<= 2u;
    /*  Calculation of Middle stage */
    for (k = fftLen / 4u; k > 4u; k >>= 2u)
    {
        /*  Initializations for the middle stage */
        n1 = n2;
        n2 >>= 2u;
        ic = 0u;
        
        for (j = 0u; j <= (n2 - 1u); j++)
        {
            /*  index calculation for the coefficients */
            C1 = _SIMD32_OFFSET(pCoef16 + (2u * ic));
            C2 = _SIMD32_OFFSET(pCoef16 + (4u * ic));
            C3 = _SIMD32_OFFSET(pCoef16 + (6u * ic));
            
            /*  Twiddle coefficients index modifier */
            ic = ic + twidCoefModifier;
            
            /*  Butterfly implementation */
            for (i0 = j; i0 < fftLen; i0 += n1)
            {
                /*  index calculation for the input as, */
                /*  pSrc16[i0 + 0], pSrc16[i0 + fftLen/4], pSrc16[i0 + fftLen/2], pSrc16[i0 + 3fftLen/4] */
                i1 = i0 + n2;
                i2 = i1 + n2;
                i3 = i2 + n2;
                
                /*  Reading i0, i0+fftLen/2 inputs */
                /* Read ya (real), xa(imag) input */
                T = _SIMD32_OFFSET(pSrc16 + (2u * i0));
                
                /* Read yc (real), xc(imag) input */
                S = _SIMD32_OFFSET(pSrc16 + (2u * i2));
                
                /* R = packed( (ya + yc), (xa + xc)) */
                R = __QADD16(T, S);
                
                /* S = packed((ya - yc), (xa - xc)) */
                S = __QSUB16(T, S);
                
                /*  Reading i0+fftLen/4 , i0+3fftLen/4 inputs */
                /* Read yb (real), xb(imag) input */
                T = _SIMD32_OFFSET(pSrc16 + (2u * i1));
                
                /* Read yd (real), xd(imag) input */
                U = _SIMD32_OFFSET(pSrc16 + (2u * i3));
                
                /* T = packed( (yb + yd), (xb + xd)) */
                T = __QADD16(T, U);
                
                /*  writing the butterfly processed i0 sample */
                
                /* xa' = xa + xb + xc + xd */
                /* ya' = ya + yb + yc + yd */
                out1 = __SHADD16(R, T);
                in = ((int16_t) (out1 & 0xFFFF)) >> 1;
                out1 = ((out1 >> 1) & 0xFFFF0000) | (in & 0xFFFF);
                _SIMD32_OFFSET(pSrc16 + (2u * i0)) = out1;
                
                /* R = packed( (ya + yc) - (yb + yd), (xa + xc) - (xb + xd)) */
                R = __SHSUB16(R, T);
                
#ifndef ARM_MATH_BIG_ENDIAN
                
                /* (ya-yb+yc-yd)* (si2) + (xa-xb+xc-xd)* co2 */
                out1 = __SMUAD(C2, R) >> 16u;
                
                /* (ya-yb+yc-yd)* co2 - (xa-xb+xc-xd)* (si2) */
                out2 = __SMUSDX(C2, R);
                
#else
                
                /* (ya-yb+yc-yd)* co2 - (xa-xb+xc-xd)* (si2) */
                out1 = __SMUSDX(R, C2) >> 16u;
                
                /* (ya-yb+yc-yd)* (si2) + (xa-xb+xc-xd)* co2 */
                out2 = __SMUAD(C2, R);
                
#endif /*      #ifndef ARM_MATH_BIG_ENDIAN     */
                
                /*  Reading i0+3fftLen/4 */
                /* Read yb (real), xb(imag) input */
                T = _SIMD32_OFFSET(pSrc16 + (2u * i1));
                
                /*  writing the butterfly processed i0 + fftLen/4 sample */
                /* xc' = (xa-xb+xc-xd)* co2 + (ya-yb+yc-yd)* (si2) */
                /* yc' = (ya-yb+yc-yd)* co2 - (xa-xb+xc-xd)* (si2) */
                _SIMD32_OFFSET(pSrc16 + (2u * i1)) =
                ((out2) & 0xFFFF0000) | (out1 & 0x0000FFFF);
                
                /*  Butterfly calculations */
                
                /* Read yd (real), xd(imag) input */
                U = _SIMD32_OFFSET(pSrc16 + (2u * i3));
                
                /* T = packed(yb-yd, xb-xd) */
                T = __QSUB16(T, U);
                
#ifndef ARM_MATH_BIG_ENDIAN
                
                /* R = packed((ya-yc) + (xb- xd) , (xa-xc) - (yb-yd)) */
                R = __SHASX(S, T);
                
                /* S = packed((ya-yc) - (xb- xd),  (xa-xc) + (yb-yd)) */
                S = __SHSAX(S, T);
                
                
                /*  Butterfly process for the i0+fftLen/2 sample */
                out1 = __SMUAD(C1, S) >> 16u;
                out2 = __SMUSDX(C1, S);
                
#else
                
                /* R = packed((ya-yc) + (xb- xd) , (xa-xc) - (yb-yd)) */
                R = __SHSAX(S, T);
                
                /* S = packed((ya-yc) - (xb- xd),  (xa-xc) + (yb-yd)) */
                S = __SHASX(S, T);
                
                
                /*  Butterfly process for the i0+fftLen/2 sample */
                out1 = __SMUSDX(S, C1) >> 16u;
                out2 = __SMUAD(C1, S);
                
#endif /*      #ifndef ARM_MATH_BIG_ENDIAN     */
                
                /* xb' = (xa+yb-xc-yd)* co1 + (ya-xb-yc+xd)* (si1) */
                /* yb' = (ya-xb-yc+xd)* co1 - (xa+yb-xc-yd)* (si1) */
                _SIMD32_OFFSET(pSrc16 + (2u * i2)) =
                ((out2) & 0xFFFF0000) | (out1 & 0x0000FFFF);
                
                /*  Butterfly process for the i0+3fftLen/4 sample */
                
#ifndef ARM_MATH_BIG_ENDIAN
                
                out1 = __SMUAD(C3, R) >> 16u;
                out2 = __SMUSDX(C3, R);
                
#else
                
                out1 = __SMUSDX(R, C3) >> 16u;
                out2 = __SMUAD(C3, R);
                
#endif /*      #ifndef ARM_MATH_BIG_ENDIAN     */
                
                /* xd' = (xa-yb-xc+yd)* co3 + (ya+xb-yc-xd)* (si3) */
                /* yd' = (ya+xb-yc-xd)* co3 - (xa-yb-xc+yd)* (si3) */
                _SIMD32_OFFSET(pSrc16 + (2u * i3)) =
                ((out2) & 0xFFFF0000) | (out1 & 0x0000FFFF);
            }
        }
        /*  Twiddle coefficients index modifier */
        twidCoefModifier <<= 2u;
    }
#else
    
#endif /* #ifndef ARM_MATH_CM0 */
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
inline void arm_radix4_butterfly_q15_stage3(q15_t * pSrc16, uint32_t fftLen, q15_t * pCoef16, uint32_t twidCoefModifier) {
    /* start of last stage process */
#ifndef ARM_MATH_CM0
    /*  Initializations for the last stage */
    q31_t R, S, T, U;
    q31_t C1, C2, C3, out1, out2;
    q15_t *ptr1;
    q31_t xaya, xbyb, xcyc, xdyd;
    
    uint32_t j;
    
    j = fftLen >> 2;
    
    ptr1 = &pSrc16[0];
    
    /*  Butterfly implementation */
    do
    {
        /* Read xa (real), ya(imag) input */
        xaya = *__SIMD32(ptr1)++;
        
        /* Read xb (real), yb(imag) input */
        xbyb = *__SIMD32(ptr1)++;
        
        /* Read xc (real), yc(imag) input */
        xcyc = *__SIMD32(ptr1)++;
        
        /* Read xd (real), yd(imag) input */
        xdyd = *__SIMD32(ptr1)++;
        
        /* R = packed((ya + yc), (xa + xc)) */
        R = __QADD16(xaya, xcyc);
        
        /* T = packed((yb + yd), (xb + xd)) */
        T = __QADD16(xbyb, xdyd);
        
        /* pointer updation for writing */
        ptr1 = ptr1 - 8u;
        
        
        /* xa' = xa + xb + xc + xd */
        /* ya' = ya + yb + yc + yd */
        *__SIMD32(ptr1)++ = __SHADD16(R, T);
        
        /* T = packed((yb + yd), (xb + xd)) */
        T = __QADD16(xbyb, xdyd);
        
        /* xc' = (xa-xb+xc-xd) */
        /* yc' = (ya-yb+yc-yd) */
        *__SIMD32(ptr1)++ = __SHSUB16(R, T);
        
        /* S = packed((ya - yc), (xa - xc)) */
        S = __QSUB16(xaya, xcyc);
        
        /* Read yd (real), xd(imag) input */
        /* T = packed( (yb - yd), (xb - xd))  */
        U = __QSUB16(xbyb, xdyd);
        
#ifndef ARM_MATH_BIG_ENDIAN
        
        /* xb' = (xa+yb-xc-yd) */
        /* yb' = (ya-xb-yc+xd) */
        *__SIMD32(ptr1)++ = __SHSAX(S, U);
        
        
        /* xd' = (xa-yb-xc+yd) */
        /* yd' = (ya+xb-yc-xd) */
        *__SIMD32(ptr1)++ = __SHASX(S, U);
        
#else
        
        /* xb' = (xa+yb-xc-yd) */
        /* yb' = (ya-xb-yc+xd) */
        *__SIMD32(ptr1)++ = __SHASX(S, U);
        
        
        /* xd' = (xa-yb-xc+yd) */
        /* yd' = (ya+xb-yc-xd) */
        *__SIMD32(ptr1)++ = __SHSAX(S, U);
        
#endif /*      #ifndef ARM_MATH_BIG_ENDIAN     */
        
    } while (--j);
#else
    
#endif /* #ifndef ARM_MATH_CM0 */
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
inline void arm_radix4_butterfly_inverse_q15_stage1(q15_t * pSrc16, uint32_t fftLen, q15_t * pCoef16, uint32_t twidCoefModifier) {
#ifndef ARM_MATH_CM0
    
    q31_t R, S, T, U;
    q31_t C1, C2, C3, out1, out2;
    uint32_t n1, n2, ic, i0, i1, i2, i3, j, k;
    q15_t in;
    
    q15_t *ptr1;
    
    
    
    q31_t xaya, xbyb, xcyc, xdyd;
    
    /* Total process is divided into three stages */
    
    /* process first stage, middle stages, & last stage */
    
    /*  Initializations for the first stage */
    n2 = fftLen;
    n1 = n2;
    
    /* n2 = fftLen/4 */
    n2 >>= 2u;
    
    /* Index for twiddle coefficient */
    ic = 0u;
    
    /* Index for input read and output write */
    i0 = 0u;
    j = n2;
    
    /* Input is in 1.15(q15) format */
    
    /*  start of first stage process */
    do
    {
        /*  Butterfly implementation */
        
        /*  index calculation for the input as, */
        /*  pSrc16[i0 + 0], pSrc16[i0 + fftLen/4], pSrc16[i0 + fftLen/2], pSrc16[i0 + 3fftLen/4] */
        i1 = i0 + n2;
        i2 = i1 + n2;
        i3 = i2 + n2;
        
        /*  Reading i0, i0+fftLen/2 inputs */
        /* Read ya (real), xa(imag) input */
        T = _SIMD32_OFFSET(pSrc16 + (2u * i0));
        in = ((int16_t) (T & 0xFFFF)) >> 2;
        T = ((T >> 2) & 0xFFFF0000) | (in & 0xFFFF);
        
        /* Read yc (real), xc(imag) input */
        S = _SIMD32_OFFSET(pSrc16 + (2u * i2));
        in = ((int16_t) (S & 0xFFFF)) >> 2;
        S = ((S >> 2) & 0xFFFF0000) | (in & 0xFFFF);
        
        /* R = packed((ya + yc), (xa + xc) ) */
        R = __QADD16(T, S);
        
        /* S = packed((ya - yc), (xa - xc) ) */
        S = __QSUB16(T, S);
        
        /*  Reading i0+fftLen/4 , i0+3fftLen/4 inputs */
        /* Read yb (real), xb(imag) input */
        T = _SIMD32_OFFSET(pSrc16 + (2u * i1));
        in = ((int16_t) (T & 0xFFFF)) >> 2;
        T = ((T >> 2) & 0xFFFF0000) | (in & 0xFFFF);
        
        /* Read yd (real), xd(imag) input */
        U = _SIMD32_OFFSET(pSrc16 + (2u * i3));
        in = ((int16_t) (U & 0xFFFF)) >> 2;
        U = ((U >> 2) & 0xFFFF0000) | (in & 0xFFFF);
        
        /* T = packed((yb + yd), (xb + xd) ) */
        T = __QADD16(T, U);
        
        /*  writing the butterfly processed i0 sample */
        /* xa' = xa + xb + xc + xd */
        /* ya' = ya + yb + yc + yd */
        _SIMD32_OFFSET(pSrc16 + (2u * i0)) = __SHADD16(R, T);
        
        /* R = packed((ya + yc) - (yb + yd), (xa + xc)- (xb + xd)) */
        R = __QSUB16(R, T);
        
        /* co2 & si2 are read from SIMD Coefficient pointer */
        C2 = _SIMD32_OFFSET(pCoef16 + (4u * ic));
        
#ifndef ARM_MATH_BIG_ENDIAN
        
        /* xc' = (xa-xb+xc-xd)* co2 + (ya-yb+yc-yd)* (si2) */
        out1 = __SMUSD(C2, R) >> 16u;
        /* yc' = (ya-yb+yc-yd)* co2 - (xa-xb+xc-xd)* (si2) */
        out2 = __SMUADX(C2, R);
        
#else
        
        /* xc' = (ya-yb+yc-yd)* co2 - (xa-xb+xc-xd)* (si2) */
        out1 = __SMUADX(C2, R) >> 16u;
        /* yc' = (xa-xb+xc-xd)* co2 + (ya-yb+yc-yd)* (si2) */
        out2 = __SMUSD(__QSUB16(0, C2), R);
        
#endif /*      #ifndef ARM_MATH_BIG_ENDIAN     */
        
        /*  Reading i0+fftLen/4 */
        /* T = packed(yb, xb) */
        T = _SIMD32_OFFSET(pSrc16 + (2u * i1));
        in = ((int16_t) (T & 0xFFFF)) >> 2;
        T = ((T >> 2) & 0xFFFF0000) | (in & 0xFFFF);
        
        /* writing the butterfly processed i0 + fftLen/4 sample */
        /* writing output(xc', yc') in little endian format */
        _SIMD32_OFFSET(pSrc16 + (2u * i1)) =
        (q31_t) ((out2) & 0xFFFF0000) | (out1 & 0x0000FFFF);
        
        /*  Butterfly calculations */
        /* U = packed(yd, xd) */
        U = _SIMD32_OFFSET(pSrc16 + (2u * i3));
        in = ((int16_t) (U & 0xFFFF)) >> 2;
        U = ((U >> 2) & 0xFFFF0000) | (in & 0xFFFF);
        
        /* T = packed(yb-yd, xb-xd) */
        T = __QSUB16(T, U);
        
#ifndef ARM_MATH_BIG_ENDIAN
        
        /* R = packed((ya-yc) + (xb- xd) , (xa-xc) - (yb-yd)) */
        R = __QSAX(S, T);
        /* S = packed((ya-yc) + (xb- xd),  (xa-xc) - (yb-yd)) */
        S = __QASX(S, T);
        
#else
        
        /* R = packed((ya-yc) + (xb- xd) , (xa-xc) - (yb-yd)) */
        R = __QASX(S, T);
        /* S = packed((ya-yc) - (xb- xd),  (xa-xc) + (yb-yd)) */
        S = __QSAX(S, T);
        
#endif /*      #ifndef ARM_MATH_BIG_ENDIAN     */
        
        /* co1 & si1 are read from SIMD Coefficient pointer */
        C1 = _SIMD32_OFFSET(pCoef16 + (2u * ic));
        /*  Butterfly process for the i0+fftLen/2 sample */
        
#ifndef ARM_MATH_BIG_ENDIAN
        
        /* xb' = (xa+yb-xc-yd)* co1 + (ya-xb-yc+xd)* (si1) */
        out1 = __SMUSD(C1, S) >> 16u;
        /* yb' = (ya-xb-yc+xd)* co1 - (xa+yb-xc-yd)* (si1) */
        out2 = __SMUADX(C1, S);
        
#else
        
        /* xb' = (ya-xb-yc+xd)* co1 - (xa+yb-xc-yd)* (si1) */
        out1 = __SMUADX(C1, S) >> 16u;
        /* yb' = (xa+yb-xc-yd)* co1 + (ya-xb-yc+xd)* (si1) */
        out2 = __SMUSD(__QSUB16(0, C1), S);
        
#endif /*      #ifndef ARM_MATH_BIG_ENDIAN     */
        
        /* writing output(xb', yb') in little endian format */
        _SIMD32_OFFSET(pSrc16 + (2u * i2)) =
        ((out2) & 0xFFFF0000) | ((out1) & 0x0000FFFF);
        
        
        /* co3 & si3 are read from SIMD Coefficient pointer */
        C3 = _SIMD32_OFFSET(pCoef16 + (6u * ic));
        /*  Butterfly process for the i0+3fftLen/4 sample */
        
#ifndef ARM_MATH_BIG_ENDIAN
        
        /* xd' = (xa-yb-xc+yd)* co3 + (ya+xb-yc-xd)* (si3) */
        out1 = __SMUSD(C3, R) >> 16u;
        /* yd' = (ya+xb-yc-xd)* co3 - (xa-yb-xc+yd)* (si3) */
        out2 = __SMUADX(C3, R);
        
#else
        
        /* xd' = (ya+xb-yc-xd)* co3 - (xa-yb-xc+yd)* (si3) */
        out1 = __SMUADX(C3, R) >> 16u;
        /* yd' = (xa-yb-xc+yd)* co3 + (ya+xb-yc-xd)* (si3) */
        out2 = __SMUSD(__QSUB16(0, C3), R);
        
#endif /*      #ifndef ARM_MATH_BIG_ENDIAN     */
        
        /* writing output(xd', yd') in little endian format */
        _SIMD32_OFFSET(pSrc16 + (2u * i3)) =
        ((out2) & 0xFFFF0000) | (out1 & 0x0000FFFF);
        
        /*  Twiddle coefficients index modifier */
        ic = ic + twidCoefModifier;
        
        /*  Updating input index */
        i0 = i0 + 1u;
        
    } while(--j);
    /* data is in 4.11(q11) format */
    
    /* end of first stage process */
#else
    
#endif /* #ifndef ARM_MATH_CM0 */
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
inline void arm_radix4_butterfly_inverse_q15_stage2(q15_t * pSrc16, uint32_t fftLen, q15_t * pCoef16, uint32_t twidCoefModifier) {
#ifndef ARM_MATH_CM0
    q31_t R, S, T, U;
    q31_t C1, C2, C3, out1, out2;
    uint32_t n1, n2, ic, i0, i1, i2, i3, j, k;
    q15_t in;
    
    q15_t *ptr1;
    
    
    
    q31_t xaya, xbyb, xcyc, xdyd;
    
    /* start of middle stage process */
    
    n2 = fftLen;
    n2 >>= 2u;
    
    /*  Twiddle coefficients index modifier */
    twidCoefModifier <<= 2u;
    
    /*  Calculation of Middle stage */
    for (k = fftLen / 4u; k > 4u; k >>= 2u)
    {
        /*  Initializations for the middle stage */
        n1 = n2;
        n2 >>= 2u;
        ic = 0u;
        
        for (j = 0u; j <= (n2 - 1u); j++)
        {
            /*  index calculation for the coefficients */
            C1 = _SIMD32_OFFSET(pCoef16 + (2u * ic));
            C2 = _SIMD32_OFFSET(pCoef16 + (4u * ic));
            C3 = _SIMD32_OFFSET(pCoef16 + (6u * ic));
            
            /*  Twiddle coefficients index modifier */
            ic = ic + twidCoefModifier;
            
            /*  Butterfly implementation */
            for (i0 = j; i0 < fftLen; i0 += n1)
            {
                /*  index calculation for the input as, */
                /*  pSrc16[i0 + 0], pSrc16[i0 + fftLen/4], pSrc16[i0 + fftLen/2], pSrc16[i0 + 3fftLen/4] */
                i1 = i0 + n2;
                i2 = i1 + n2;
                i3 = i2 + n2;
                
                /*  Reading i0, i0+fftLen/2 inputs */
                /* Read ya (real), xa(imag) input */
                T = _SIMD32_OFFSET(pSrc16 + (2u * i0));
                
                /* Read yc (real), xc(imag) input */
                S = _SIMD32_OFFSET(pSrc16 + (2u * i2));
                
                /* R = packed( (ya + yc), (xa + xc)) */
                R = __QADD16(T, S);
                
                /* S = packed((ya - yc), (xa - xc)) */
                S = __QSUB16(T, S);
                
                /*  Reading i0+fftLen/4 , i0+3fftLen/4 inputs */
                /* Read yb (real), xb(imag) input */
                T = _SIMD32_OFFSET(pSrc16 + (2u * i1));
                
                /* Read yd (real), xd(imag) input */
                U = _SIMD32_OFFSET(pSrc16 + (2u * i3));
                
                /* T = packed( (yb + yd), (xb + xd)) */
                T = __QADD16(T, U);
                
                /*  writing the butterfly processed i0 sample */
                
                /* xa' = xa + xb + xc + xd */
                /* ya' = ya + yb + yc + yd */
                out1 = __SHADD16(R, T);
                in = ((int16_t) (out1 & 0xFFFF)) >> 1;
                out1 = ((out1 >> 1) & 0xFFFF0000) | (in & 0xFFFF);
                _SIMD32_OFFSET(pSrc16 + (2u * i0)) = out1;
                
                /* R = packed( (ya + yc) - (yb + yd), (xa + xc) - (xb + xd)) */
                R = __SHSUB16(R, T);
                
#ifndef ARM_MATH_BIG_ENDIAN
                
                /* (ya-yb+yc-yd)* (si2) + (xa-xb+xc-xd)* co2 */
                out1 = __SMUSD(C2, R) >> 16u;
                
                /* (ya-yb+yc-yd)* co2 - (xa-xb+xc-xd)* (si2) */
                out2 = __SMUADX(C2, R);
                
#else
                
                /* (ya-yb+yc-yd)* co2 - (xa-xb+xc-xd)* (si2) */
                out1 = __SMUADX(R, C2) >> 16u;
                
                /* (ya-yb+yc-yd)* (si2) + (xa-xb+xc-xd)* co2 */
                out2 = __SMUSD(__QSUB16(0, C2), R);
                
#endif /*      #ifndef ARM_MATH_BIG_ENDIAN     */
                
                /*  Reading i0+3fftLen/4 */
                /* Read yb (real), xb(imag) input */
                T = _SIMD32_OFFSET(pSrc16 + (2u * i1));
                
                /*  writing the butterfly processed i0 + fftLen/4 sample */
                /* xc' = (xa-xb+xc-xd)* co2 + (ya-yb+yc-yd)* (si2) */
                /* yc' = (ya-yb+yc-yd)* co2 - (xa-xb+xc-xd)* (si2) */
                _SIMD32_OFFSET(pSrc16 + (2u * i1)) =
                ((out2) & 0xFFFF0000) | (out1 & 0x0000FFFF);
                
                /*  Butterfly calculations */
                
                /* Read yd (real), xd(imag) input */
                U = _SIMD32_OFFSET(pSrc16 + (2u * i3));
                
                /* T = packed(yb-yd, xb-xd) */
                T = __QSUB16(T, U);
                
#ifndef ARM_MATH_BIG_ENDIAN
                
                /* R = packed((ya-yc) + (xb- xd) , (xa-xc) - (yb-yd)) */
                R = __SHSAX(S, T);
                
                /* S = packed((ya-yc) - (xb- xd),  (xa-xc) + (yb-yd)) */
                S = __SHASX(S, T);
                
                
                /*  Butterfly process for the i0+fftLen/2 sample */
                out1 = __SMUSD(C1, S) >> 16u;
                out2 = __SMUADX(C1, S);
                
#else
                
                /* R = packed((ya-yc) + (xb- xd) , (xa-xc) - (yb-yd)) */
                R = __SHASX(S, T);
                
                /* S = packed((ya-yc) - (xb- xd),  (xa-xc) + (yb-yd)) */
                S = __SHSAX(S, T);
                
                
                /*  Butterfly process for the i0+fftLen/2 sample */
                out1 = __SMUADX(S, C1) >> 16u;
                out2 = __SMUSD(__QSUB16(0, C1), S);
                
#endif /*      #ifndef ARM_MATH_BIG_ENDIAN     */
                
                /* xb' = (xa+yb-xc-yd)* co1 + (ya-xb-yc+xd)* (si1) */
                /* yb' = (ya-xb-yc+xd)* co1 - (xa+yb-xc-yd)* (si1) */
                _SIMD32_OFFSET(pSrc16 + (2u * i2)) =
                ((out2) & 0xFFFF0000) | (out1 & 0x0000FFFF);
                
                /*  Butterfly process for the i0+3fftLen/4 sample */
                
#ifndef ARM_MATH_BIG_ENDIAN
                
                out1 = __SMUSD(C3, R) >> 16u;
                out2 = __SMUADX(C3, R);
                
#else
                
                out1 = __SMUADX(C3, R) >> 16u;
                out2 = __SMUSD(__QSUB16(0, C3), R);
                
#endif /*      #ifndef ARM_MATH_BIG_ENDIAN     */
                
                /* xd' = (xa-yb-xc+yd)* co3 + (ya+xb-yc-xd)* (si3) */
                /* yd' = (ya+xb-yc-xd)* co3 - (xa-yb-xc+yd)* (si3) */
                _SIMD32_OFFSET(pSrc16 + (2u * i3)) =
                ((out2) & 0xFFFF0000) | (out1 & 0x0000FFFF);
            }
        }
        /*  Twiddle coefficients index modifier */
        twidCoefModifier <<= 2u;
    }
    /* end of middle stage process */
    
    /* data is in 10.6(q6) format for the 1024 point */
    /* data is in 8.8(q8) format for the 256 point */
    /* data is in 6.10(q10) format for the 64 point */
    /* data is in 4.12(q12) format for the 16 point */
#else
    
#endif /* #ifndef ARM_MATH_CM0 */
}
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
inline void arm_radix4_butterfly_inverse_q15_stage3(q15_t * pSrc16, uint32_t fftLen, q15_t * pCoef16, uint32_t twidCoefModifier) {
#ifndef ARM_MATH_CM0
    q31_t R, S, T, U;
    q31_t C1, C2, C3, out1, out2;
    uint32_t n1, n2, ic, i0, i1, i2, i3, j, k;
    q15_t in;
    
    q15_t *ptr1;
    
    
    
    q31_t xaya, xbyb, xcyc, xdyd;
    
    /*  Initializations for the last stage */
    j = fftLen >> 2;
    
    ptr1 = &pSrc16[0];
    
    /* start of last stage process */
    
    /*  Butterfly implementation */
    do
    {
        /* Read xa (real), ya(imag) input */
        xaya = *__SIMD32(ptr1)++;
        
        /* Read xb (real), yb(imag) input */
        xbyb = *__SIMD32(ptr1)++;
        
        /* Read xc (real), yc(imag) input */
        xcyc = *__SIMD32(ptr1)++;
        
        /* Read xd (real), yd(imag) input */
        xdyd = *__SIMD32(ptr1)++;
        
        /* R = packed((ya + yc), (xa + xc)) */
        R = __QADD16(xaya, xcyc);
        
        /* T = packed((yb + yd), (xb + xd)) */
        T = __QADD16(xbyb, xdyd);
        
        /* pointer updation for writing */
        ptr1 = ptr1 - 8u;
        
        
        /* xa' = xa + xb + xc + xd */
        /* ya' = ya + yb + yc + yd */
        *__SIMD32(ptr1)++ = __SHADD16(R, T);
        
        /* T = packed((yb + yd), (xb + xd)) */
        T = __QADD16(xbyb, xdyd);
        
        /* xc' = (xa-xb+xc-xd) */
        /* yc' = (ya-yb+yc-yd) */
        *__SIMD32(ptr1)++ = __SHSUB16(R, T);
        
        /* S = packed((ya - yc), (xa - xc)) */
        S = __QSUB16(xaya, xcyc);
        
        /* Read yd (real), xd(imag) input */
        /* T = packed( (yb - yd), (xb - xd))  */
        U = __QSUB16(xbyb, xdyd);
        
#ifndef ARM_MATH_BIG_ENDIAN
        
        /* xb' = (xa+yb-xc-yd) */
        /* yb' = (ya-xb-yc+xd) */
        *__SIMD32(ptr1)++ = __SHASX(S, U);
        
        
        /* xd' = (xa-yb-xc+yd) */
        /* yd' = (ya+xb-yc-xd) */
        *__SIMD32(ptr1)++ = __SHSAX(S, U);
        
#else
        
        /* xb' = (xa+yb-xc-yd) */
        /* yb' = (ya-xb-yc+xd) */
        *__SIMD32(ptr1)++ = __SHSAX(S, U);
        
        
        /* xd' = (xa-yb-xc+yd) */
        /* yd' = (ya+xb-yc-xd) */
        *__SIMD32(ptr1)++ = __SHASX(S, U);
        
        
#endif /*      #ifndef ARM_MATH_BIG_ENDIAN     */
        
    } while(--j);
    
    /* end of last stage  process */
    
    /* output is in 11.5(q5) format for the 1024 point */
    /* output is in 9.7(q7) format for the 256 point   */
    /* output is in 7.9(q9) format for the 64 point  */
    /* output is in 5.11(q11) format for the 16 point  */
#else
    
#endif /* #ifndef ARM_MATH_CM0 */
}