// Copyright 2022 ETH Zurich and University of Bologna.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// Author: Marco Bertuletti, ETH Zurich

void mempool_cfft_q16s( uint16_t fftLen,
                        int16_t *pSrc,
                        int16_t *pCoef,
                        uint16_t *pBitRevTable,
                        uint16_t bitReverseLen,
                        uint8_t bitReverseFlag);


#ifndef XPULP

static inline void radix4_butterfly_first(  int16_t* pIn,
                                            int16_t Co1,
                                            int16_t Si1,
                                            int16_t Co2,
                                            int16_t Si2,
                                            int16_t Co3,
                                            int16_t Si3,
                                            uint32_t i0,
                                            uint32_t n2);

static inline void radix4_butterfly_middle( int16_t* pIn,
                                            int16_t Co1,
                                            int16_t Si1,
                                            int16_t Co2,
                                            int16_t Si2,
                                            int16_t Co3,
                                            int16_t Si3,
                                            uint32_t i0,
                                            uint32_t n2);

static inline void radix4_butterfly_last( int16_t* pIn,
                                          uint32_t i0,
                                          uint32_t n2);
#else

static inline void radix4_butterfly_first(  int16_t* pIn,
                                            uint32_t i0,
                                            uint32_t n2,
                                            v2s CoSi1,
                                            v2s CoSi2,
                                            v2s CoSi3);

static inline void radix4_butterfly_middle( int16_t* pIn,
                                            uint32_t i0,
                                            uint32_t n2,
                                            v2s CoSi1,
                                            v2s CoSi2,
                                            v2s CoSi3);

static inline void radix4_butterfly_last( int16_t* pIn,
                                          uint32_t i0,
                                          uint32_t n2);

#endif

void mempool_cfft_q16s( uint16_t fftLen,
                        int16_t *pSrc,
                        int16_t *pCoef,
                        uint16_t *pBitRevTable,
                        uint16_t bitReverseLen,
                        uint8_t bitReverseFlag) {

    /* Initializations for the first stage */
    uint32_t n1, n2, i0, ic, j, k, twidCoefModifier;
    #ifndef XPULP
    int16_t Co1, Si1, Co2, Si2, Co3, Si3;
    #else
    v2s CoSi1, CoSi2, CoSi3;
    #endif

    /* FIRST STAGE */
    n1 = fftLen;
    n2 = n1 >> 2U;
    for(i0 = 0; i0 < n2; i0++) {

        #ifndef XPULP
        Co1 = pCoef[i0 * 2U];
        Si1 = pCoef[(i0 * 2U) + 1];
        Co2 = pCoef[2U * i0 * 2U];
        Si2 = pCoef[(2U * i0 * 2U) + 1];
        Co3 = pCoef[3U * (i0 * 2U)];
        Si3 = pCoef[(3U * (i0 * 2U)) + 1];
        for(uint32_t idx_fft = 0; idx_fft < N_FFTs; idx_fft++) {
            radix4_butterfly_first(  pSrc + idx_fft * 2 * N_BANKS_SINGLE,
                                      Co1, Si1, Co2, Si2, Co3, Si3, i0, n2);
        }
        #else
        CoSi1 = *(v2s *)&pCoef[2U * i0];
        CoSi2 = *(v2s *)&pCoef[2U * i0 * 2U];
        CoSi3 = *(v2s *)&pCoef[2U * i0 * 3U];
        for(uint32_t idx_fft = 0; idx_fft < N_FFTs; idx_fft++) {
            radix4_butterfly_first( pSrc + idx_fft * 2 * N_BANKS_SINGLE,
                                    i0, n2,
                                    CoSi1, CoSi2, CoSi3);
        }
        #endif

    }

    /* MIDDLE STAGE */
    twidCoefModifier = 4;
    for (k = fftLen / 4U; k > 4U; k >>= 2U) {
        n1 = n2;
        n2 >>= 2U;
        ic = 0U;
        for (j = 0U; j <= (n2 - 1U); j++) {

            #ifndef XPULP
            Co1 = pCoef[ic * 2U];
            Si1 = pCoef[(ic * 2U) + 1U];
            Co2 = pCoef[2U * (ic * 2U)];
            Si2 = pCoef[(2U * (ic * 2U)) + 1U];
            Co3 = pCoef[3U * (ic * 2U)];
            Si3 = pCoef[(3U * (ic * 2U)) + 1U];
            ic = ic + twidCoefModifier;
            for (i0 = j; i0 < fftLen; i0 += n1) {
                for(uint32_t idx_fft = 0; idx_fft < N_FFTs; idx_fft++) {
                    radix4_butterfly_middle(  pSrc + idx_fft * 2 * N_BANKS_SINGLE,
                                              Co1, Si1, Co2, Si2, Co3, Si3, i0, n2);
                }
            }
            #else
            CoSi1 = *(v2s *)&pCoef[2U * ic];
            CoSi2 = *(v2s *)&pCoef[2U * ic * 2U];
            CoSi3 = *(v2s *)&pCoef[2U * ic * 3U];
            ic = ic + twidCoefModifier;
            for (i0 = j; i0 < fftLen; i0 += n1) {
                for(uint32_t idx_fft = 0; idx_fft < N_FFTs; idx_fft++) {
                    radix4_butterfly_middle(  pSrc + idx_fft * 2 * N_BANKS_SINGLE,
                                              i0, n2,
                                              CoSi1, CoSi2, CoSi3);
                }
            }
            #endif

        }
        twidCoefModifier <<= 2U;
    }

    /* LAST STAGE */
    n1 = n2;
    n2 >>= 2U;
    for (i0 = 0U; i0 <= (fftLen - n1); i0 += n1) {
        for(uint32_t idx_fft = 0; idx_fft < N_FFTs; idx_fft++) {
            radix4_butterfly_last(pSrc + idx_fft * 2 * N_BANKS_SINGLE, i0, n2);
        }
    }

    /* BITREVERSAL */
    if(bitReverseFlag) {
        v2s addr1, addr2, addr3, addr4;
        v2s s2 = (v2s){ 2, 2 };
        v2s tmpa1, tmpa2, tmpa3, tmpa4;
        v2s tmpb1, tmpb2, tmpb3, tmpb4;
        int32_t a1, a2, a3, a4;
        int32_t b1, b2, b3, b4;
        uint16_t *ptr;
        for (uint32_t i = 0; i < bitReverseLen; i += 8){
            addr1 = *(v2s *)&pBitRevTable[i];
            addr2 = *(v2s *)&pBitRevTable[i + 2];
            addr3 = *(v2s *)&pBitRevTable[i + 4];
            addr4 = *(v2s *)&pBitRevTable[i + 6];
            asm volatile (
                "pv.sra.h  %[addr1],%[addr1],%[s2];"
                "pv.sra.h  %[addr2],%[addr2],%[s2];"
                "pv.sra.h  %[addr3],%[addr3],%[s2];"
                "pv.sra.h  %[addr4],%[addr4],%[s2];"
                "pv.extract.h  %[a1],%[addr1],0;"
                "pv.extract.h  %[a2],%[addr2],0;"
                "pv.extract.h  %[a3],%[addr3],0;"
                "pv.extract.h  %[a4],%[addr4],0;"
                "pv.extract.h  %[b1],%[addr1],1;"
                "pv.extract.h  %[b2],%[addr2],1;"
                "pv.extract.h  %[b3],%[addr3],1;"
                "pv.extract.h  %[b4],%[addr4],1;"
                : [a1] "=r" (a1), [a2] "=r" (a2), [a3] "=r" (a3), [a4] "=r" (a4),
                  [b1] "=r" (b1), [b2] "=r" (b2), [b3] "=r" (b3), [b4] "=r" (b4),
                  [addr1] "+r" (addr1), [addr2] "+r" (addr2), [addr3] "+r" (addr3), [addr4] "+r" (addr4)
                : [s2] "r" (s2)
                : );
            ptr = (uint16_t*) pSrc;
            for(uint32_t idx_fft = 0; idx_fft < N_FFTs; idx_fft++) {
                tmpa1 = *(v2s *)&ptr[a1];
                tmpa2 = *(v2s *)&ptr[a2];
                tmpa3 = *(v2s *)&ptr[a3];
                tmpa4 = *(v2s *)&ptr[a4];
                tmpb1 = *(v2s *)&ptr[b1];
                tmpb2 = *(v2s *)&ptr[b2];
                tmpb3 = *(v2s *)&ptr[b3];
                tmpb4 = *(v2s *)&ptr[b4];
                *((v2s *)&ptr[a1]) = tmpb1;
                *((v2s *)&ptr[a2]) = tmpb2;
                *((v2s *)&ptr[a3]) = tmpb3;
                *((v2s *)&ptr[a4]) = tmpb4;
                *((v2s *)&ptr[b1]) = tmpa1;
                *((v2s *)&ptr[b2]) = tmpa2;
                *((v2s *)&ptr[b3]) = tmpa3;
                *((v2s *)&ptr[b4]) = tmpa4;
                ptr += N_BANKS_SINGLE;
            }
        }
    }

}

#ifndef XPULP

static inline void radix4_butterfly_first(   int16_t* pSrc,
                                              int16_t Co1,
                                              int16_t Si1,
                                              int16_t Co2,
                                              int16_t Si2,
                                              int16_t Co3,
                                              int16_t Si3,
                                              uint32_t i0,
                                              uint32_t n2) {

    int16_t R0, R1, S0, S1, T0, T1, U0, U1;
    uint32_t i1, i2, i3;
    int16_t out1, out2;
    /*  index calculation for the input as, */
    /*  pIn[i0 + 0], pIn[i0 + fftLen/4], pIn[i0 + fftLen/2], pIn[i0 +
    * 3fftLen/4] */
    i1 = i0 + n2;
    i2 = i1 + n2;
    i3 = i2 + n2;
    /* Reading i0, i0+fftLen/2 inputs */
    /* input is down scale by 4 to avoid overflow */
    /* Read ya (real), xa (imag) input */
    T0 = pIn[i0 * 2U] >> 2U;
    T1 = pIn[(i0 * 2U) + 1U] >> 2U;
    /* input is down scale by 4 to avoid overflow */
    /* Read yc (real), xc(imag) input */
    S0 = pIn[i2 * 2U] >> 2U;
    S1 = pIn[(i2 * 2U) + 1U] >> 2U;
    /* R0 = (ya + yc) */
    R0 = (int16_t) __CLIP(T0 + S0, 15);
    /* R1 = (xa + xc) */
    R1 = (int16_t) __CLIP(T1 + S1, 15);
    /* S0 = (ya - yc) */
    S0 = (int16_t) __CLIP(T0 - S0, 15);
    /* S1 = (xa - xc) */
    S1 = (int16_t) __CLIP(T1 - S1, 15);
    /*  Reading i0+fftLen/4 , i0+3fftLen/4 inputs */
    /* input is down scale by 4 to avoid overflow */
    /* Read yb (real), xb(imag) input */
    T0 = pIn[i1 * 2U] >> 2U;
    T1 = pIn[(i1 * 2U) + 1U] >> 2U;
    /* input is down scale by 4 to avoid overflow */
    /* Read yd (real), xd(imag) input */
    U0 = pIn[i3 * 2U] >> 2U;
    U1 = pIn[(i3 * 2U) + 1U] >> 2U;
    /* T0 = (yb + yd) */
    T0 = (int16_t) __CLIP(T0 + U0, 15);
    /* T1 = (xb + xd) */
    T1 = (int16_t) __CLIP(T1 + U1, 15);
    /*  writing the butterfly processed i0 sample */
    /* ya' = ya + yb + yc + yd */
    /* xa' = xa + xb + xc + xd */
    pIn[i0 * 2] = (int16_t)((R0 >> 1U) + (T0 >> 1U));
    pIn[(i0 * 2) + 1] = (int16_t)((R1 >> 1U) + (T1 >> 1U));
    /* R0 = (ya + yc) - (yb + yd) */
    /* R1 = (xa + xc) - (xb + xd) */
    R0 = (int16_t) __CLIP(R0 - T0, 15);
    R1 = (int16_t) __CLIP(R1 - T1, 15);
    /* xc' = (xa-xb+xc-xd)* co2 + (ya-yb+yc-yd)* (si2) */
    out1 = (int16_t)((Co2 * R0 + Si2 * R1) >> 16U);
    /* yc' = (ya-yb+yc-yd)* co2 - (xa-xb+xc-xd)* (si2) */
    out2 = (int16_t)((-Si2 * R0 + Co2 * R1) >> 16U);
    /*  Reading i0+fftLen/4 */
    /* input is down scale by 4 to avoid overflow */
    /* T0 = yb, T1 =  xb */
    T0 = pIn[i1 * 2U] >> 2;
    T1 = pIn[(i1 * 2U) + 1] >> 2;
    /* writing the butterfly processed i0 + fftLen/4 sample */
    /* writing output(xc', yc') in little endian format */
    pIn[i1 * 2U] = out1;
    pIn[(i1 * 2U) + 1] = out2;
    /*  Butterfly calculations */
    /* input is down scale by 4 to avoid overflow */
    /* U0 = yd, U1 = xd */
    U0 = pIn[i3 * 2U] >> 2;
    U1 = pIn[(i3 * 2U) + 1] >> 2;
    /* T0 = yb-yd */
    T0 = (int16_t) __CLIP(T0 - U0, 15);
    /* T1 = xb-xd */
    T1 = (int16_t) __CLIP(T1 - U1, 15);
    /* R1 = (ya-yc) + (xb- xd),  R0 = (xa-xc) - (yb-yd)) */
    R0 = (int16_t)__CLIP((int32_t)(S0 - T1), 15);
    R1 = (int16_t)__CLIP((int32_t)(S1 + T0), 15);
    /* S1 = (ya-yc) - (xb- xd), S0 = (xa-xc) + (yb-yd)) */
    S0 = (int16_t)__CLIP(((int32_t)S0 + T1), 15);
    S1 = (int16_t)__CLIP(((int32_t)S1 - T0), 15);
    /*  Butterfly process for the i0+fftLen/2 sample */
    /* xb' = (xa+yb-xc-yd)* co1 + (ya-xb-yc+xd)* (si1) */
    out1 = (int16_t)((Si1 * S1 + Co1 * S0) >> 16);
    /* yb' = (ya-xb-yc+xd)* co1 - (xa+yb-xc-yd)* (si1) */
    out2 = (int16_t)((-Si1 * S0 + Co1 * S1) >> 16);
    /* writing output(xb', yb') in little endian format */
    pIn[i2 * 2U] = out1;
    pIn[(i2 * 2U) + 1] = out2;
    /*  Butterfly process for the i0+3fftLen/4 sample */
    /* xd' = (xa-yb-xc+yd)* Co3 + (ya+xb-yc-xd)* (si3) */
    out1 = (int16_t)((Si3 * R1 + Co3 * R0) >> 16U);
    /* yd' = (ya+xb-yc-xd)* Co3 - (xa-yb-xc+yd)* (si3) */
    out2 = (int16_t)((-Si3 * R0 + Co3 * R1) >> 16U);
    /* writing output(xd', yd') in little endian format */
    pIn[i3 * 2U] = out1;
    pIn[(i3 * 2U) + 1] = out2;
}

static inline void radix4_butterfly_middle( int16_t* pIn,
                                            int16_t Co1,
                                            int16_t Si1,
                                            int16_t Co2,
                                            int16_t Si2,
                                            int16_t Co3,
                                            int16_t Si3,
                                            uint32_t i0,
                                            uint32_t n2) {

    int16_t R0, R1, S0, S1, T0, T1, U0, U1;
    uint32_t i1, i2, i3;
    int16_t out1, out2;
    /*  index calculation for the input as, */
    /*  pIn[i0 + 0], pIn[i0 + fftLen/4], pIn[i0 + fftLen/2], pIn[i0 +
    * 3fftLen/4] */
    i1 = i0 + n2;
    i2 = i1 + n2;
    i3 = i2 + n2;
    /*  Reading i0, i0+fftLen/2 inputs */
    /* Read ya (real), xa(imag) input */
    T0 = pIn[i0 * 2U];
    T1 = pIn[(i0 * 2U) + 1U];
    /* Read yc (real), xc(imag) input */
    S0 = pIn[i2 * 2U];
    S1 = pIn[(i2 * 2U) + 1U];
    /* R0 = (ya + yc), R1 = (xa + xc) */
    R0 = (int16_t) __CLIP(T0 + S0, 15);
    R1 = (int16_t) __CLIP(T1 + S1, 15);
    /* S0 = (ya - yc), S1 =(xa - xc) */
    S0 = (int16_t) __CLIP(T0 - S0, 15);
    S1 = (int16_t) __CLIP(T1 - S1, 15);
    /*  Reading i0+fftLen/4 , i0+3fftLen/4 inputs */
    /* Read yb (real), xb(imag) input */
    T0 = pIn[i1 * 2U];
    T1 = pIn[(i1 * 2U) + 1U];
    /* Read yd (real), xd(imag) input */
    U0 = pIn[i3 * 2U];
    U1 = pIn[(i3 * 2U) + 1U];
    /* T0 = (yb + yd), T1 = (xb + xd) */
    T0 = (int16_t) __CLIP(T0 + U0, 15);
    T1 = (int16_t) __CLIP(T1 + U1, 15);
    /*  writing the butterfly processed i0 sample */
    /* xa' = xa + xb + xc + xd */
    /* ya' = ya + yb + yc + yd */
    out1 = (int16_t)(((R0 >> 1U) + (T0 >> 1U)) >> 1U);
    out2 = (int16_t)(((R1 >> 1U) + (T1 >> 1U)) >> 1U);
    pIn[i0 * 2U] = out1;
    pIn[(2U * i0) + 1U] = out2;
    /* R0 = (ya + yc) - (yb + yd), R1 = (xa + xc) - (xb + xd) */
    R0 = (int16_t)((R0 >> 1U) - (T0 >> 1U));
    R1 = (int16_t)((R1 >> 1U) - (T1 >> 1U));
    /* (ya-yb+yc-yd)* (si2) + (xa-xb+xc-xd)* co2 */
    out1 = (int16_t)((Co2 * R0 + Si2 * R1) >> 16U);
    /* (ya-yb+yc-yd)* co2 - (xa-xb+xc-xd)* (si2) */
    out2 = (int16_t)((-Si2 * R0 + Co2 * R1) >> 16U);
    /*  Reading i0+3fftLen/4 */
    /* Read yb (real), xb(imag) input */
    T0 = pIn[i1 * 2U];
    T1 = pIn[(i1 * 2U) + 1U];
    /*  writing the butterfly processed i0 + fftLen/4 sample */
    /* xc' = (xa-xb+xc-xd)* co2 + (ya-yb+yc-yd)* (si2) */
    /* yc' = (ya-yb+yc-yd)* co2 - (xa-xb+xc-xd)* (si2) */
    pIn[i1 * 2U] = out1;
    pIn[(i1 * 2U) + 1U] = out2;
    /*  Butterfly calculations */
    /* Read yd (real), xd(imag) input */
    U0 = pIn[i3 * 2U];
    U1 = pIn[(i3 * 2U) + 1U];
    /* T0 = yb-yd, T1 = xb-xd */
    T0 = (int16_t) __CLIP(T0 - U0, 15);
    T1 = (int16_t) __CLIP(T1 - U1, 15);
    /* R0 = (ya-yc) + (xb- xd), R1 = (xa-xc) - (yb-yd)) */
    R0 = (int16_t)((S0 >> 1U) - (T1 >> 1U));
    R1 = (int16_t)((S1 >> 1U) + (T0 >> 1U));
    /* S0 = (ya-yc) - (xb- xd), S1 = (xa-xc) + (yb-yd)) */
    S0 = (int16_t)((S0 >> 1U) + (T1 >> 1U));
    S1 = (int16_t)((S1 >> 1U) - (T0 >> 1U));
    /*  Butterfly process for the i0+fftLen/2 sample */
    out1 = (int16_t)((Co1 * S0 + Si1 * S1) >> 16U);
    out2 = (int16_t)((-Si1 * S0 + Co1 * S1) >> 16U);
    /* xb' = (xa+yb-xc-yd)* co1 + (ya-xb-yc+xd)* (si1) */
    /* yb' = (ya-xb-yc+xd)* co1 - (xa+yb-xc-yd)* (si1) */
    pIn[i2 * 2U] = out1;
    pIn[(i2 * 2U) + 1U] = out2;
    /*  Butterfly process for the i0+3fftLen/4 sample */
    out1 = (int16_t)((Si3 * R1 + Co3 * R0) >> 16U);
    out2 = (int16_t)((-Si3 * R0 + Co3 * R1) >> 16U);
    /* xd' = (xa-yb-xc+yd)* Co3 + (ya+xb-yc-xd)* (si3) */
    /* yd' = (ya+xb-yc-xd)* Co3 - (xa-yb-xc+yd)* (si3) */
    pIn[i3 * 2U] = out1;
    pIn[(i3 * 2U) + 1U] = out2;
}

static inline void radix4_butterfly_last( int16_t* pIn,
                                          uint32_t i0,
                                          uint32_t n2) {

    int16_t R0, R1, S0, S1, T0, T1, U0, U1;
    uint32_t i1, i2, i3;
    /*  index calculation for the input as, */
    /*  pIn[i0 + 0], pIn[i0 + fftLen/4], pIn[i0 + fftLen/2], pIn[i0 + 3fftLen/4] */
    i1 = i0 + n2;
    i2 = i1 + n2;
    i3 = i2 + n2;
    /*  Reading i0, i0+fftLen/2 inputs */
    /* Read ya (real), xa(imag) input */
    T0 = pIn[i0 * 2U];
    T1 = pIn[(i0 * 2U) + 1U];
    /* Read yc (real), xc(imag) input */
    S0 = pIn[i2 * 2U];
    S1 = pIn[(i2 * 2U) + 1U];
    /* R0 = (ya + yc), R1 = (xa + xc) */
    R0 = (int16_t) __CLIP(T0 + S0, 15);
    R1 = (int16_t) __CLIP(T1 + S1, 15);
    /* S0 = (ya - yc), S1 = (xa - xc) */
    S0 = (int16_t) __CLIP(T0 - S0, 15);
    S1 = (int16_t) __CLIP(T1 - S1, 15);
    /*  Reading i0+fftLen/4 , i0+3fftLen/4 inputs */
    /* Read yb (real), xb(imag) input */
    T0 = pIn[i1 * 2U];
    T1 = pIn[(i1 * 2U) + 1U];
    /* Read yd (real), xd(imag) input */
    U0 = pIn[i3 * 2U];
    U1 = pIn[(i3 * 2U) + 1U];
    /* T0 = (yb + yd), T1 = (xb + xd)) */
    T0 = (int16_t) __CLIP(T0 + U0, 15);
    T1 = (int16_t) __CLIP(T1 + U1, 15);
    /*  writing the butterfly processed i0 sample */
    /* xa' = xa + xb + xc + xd */
    /* ya' = ya + yb + yc + yd */
    pIn[i0 * 2U] = (int16_t)((R0 >> 1U) + (T0 >> 1U));
    pIn[(i0 * 2U) + 1U] = (int16_t)((R1 >> 1U) + (T1 >> 1U));
    /* R0 = (ya + yc) - (yb + yd), R1 = (xa + xc) - (xb + xd) */
    R0 = (int16_t)((R0 >> 1U) - (T0 >> 1U));
    R1 = (int16_t)((R1 >> 1U) - (T1 >> 1U));
    /* Read yb (real), xb(imag) input */
    T0 = pIn[i1 * 2U];
    T1 = pIn[(i1 * 2U) + 1U];
    /*  writing the butterfly processed i0 + fftLen/4 sample */
    /* xc' = (xa-xb+xc-xd) */
    /* yc' = (ya-yb+yc-yd) */
    pIn[i1 * 2U] = R0;
    pIn[(i1 * 2U) + 1U] = R1;
    /* Read yd (real), xd(imag) input */
    U0 = pIn[i3 * 2U];
    U1 = pIn[(i3 * 2U) + 1U];
    /* T0 = (yb - yd), T1 = (xb - xd)  */
    T0 = (int16_t) __CLIP(T0 - U0, 15);
    T1 = (int16_t) __CLIP(T1 - U1, 15);
    /*  writing the butterfly processed i0 + fftLen/2 sample */
    /* xb' = (xa+yb-xc-yd) */
    /* yb' = (ya-xb-yc+xd) */
    pIn[i2 * 2U] = (int16_t)((S0 >> 1U) + (T1 >> 1U));
    pIn[(i2 * 2U) + 1U] = (int16_t)((S1 >> 1U) - (T0 >> 1U));
    /*  writing the butterfly processed i0 + 3fftLen/4 sample */
    /* xd' = (xa-yb-xc+yd) */
    /* yd' = (ya+xb-yc-xd) */
    pIn[i3 * 2U] = (int16_t)((S0 >> 1U) - (T1 >> 1U));
    pIn[(i3 * 2U) + 1U] = (int16_t)((S1 >> 1U) + (T0 >> 1U));
}

#else

static inline void radix4_butterfly_first( int16_t* pIn,
                                            uint32_t i0,
                                            uint32_t n2,
                                            v2s CoSi1,
                                            v2s CoSi2,
                                            v2s CoSi3) {
    int16_t t0, t1, t2, t3, t4, t5;
    uint32_t i1, i2, i3;
    v2s A, B, C, D, E, F, G, H;
    v2s C1, C2, C3;

    /* index calculation for the input as, */
    /* pIn[i0 + 0], pIn[i0 + fftLen/4], pIn[i0 + fftLen/2], pIn[i0 + 3fftLen/4] */
    i1 = i0 + n2;
    i2 = i1 + n2;
    i3 = i2 + n2;

    asm volatile(
    "pv.extract.h  %[t1],%[CoSi1],1;"
    "pv.extract.h  %[t3],%[CoSi2],1;"
    "pv.extract.h  %[t5],%[CoSi3],1;"
    "pv.extract.h  %[t0],%[CoSi1],0;"
    "pv.extract.h  %[t2],%[CoSi2],0;"
    "pv.extract.h  %[t4],%[CoSi3],0;"
    "sub           %[t1],zero,%[t1];"
    "sub           %[t3],zero,%[t3];"
    "sub           %[t5],zero,%[t5];"
    "pv.pack.h %[C1],%[t1],%[t0];"
    "pv.pack.h %[C2],%[t3],%[t2];"
    "pv.pack.h %[C3],%[t5],%[t4];"
    : [C1] "=r" (C1), [C2] "=r" (C2), [C3] "=r" (C3),
      [t0] "=&r" (t0), [t1] "=&r" (t1), [t2] "=&r" (t2), [t3] "=&r" (t3),
      [t4] "=&r" (t4), [t5] "=&r" (t5)
    : [CoSi1] "r" (CoSi1), [CoSi2] "r" (CoSi2), [CoSi3] "r" (CoSi3)
    : );

    #ifndef ASM
    v2s s1 = {1, 1};
    v2s s2 = {2, 2};
    /* Read yb (real), xb(imag) input */
    B = __SRA2(*(v2s *)&pIn[i1 * 2U], s2);
    /* Read yd (real), xd(imag) input */
    D = __SRA2(*(v2s *)&pIn[i3 * 2U], s2);
    /* Read ya (real), xa (imag) input */
    A = __SRA2(*(v2s *)&pIn[i0 * 2U], s2);
    /* Read yc (real), xc(imag) input */
    C = __SRA2(*(v2s *)&pIn[i2 * 2U], s2);
    /* G0 = (yb + yd), G1 = (xb + xd) */
    G = __ADD2(B, D);
    /* H0 = (yb - yd), H1 = (xb - xd) */
    H = __SUB2(B, D);
    /* E0 = (ya + yc), E1 = (xa + xc) */
    E = __ADD2(A, C);
    /* F0 = (ya - yc), F1 = (xa - xc) */
    F = __SUB2(A, C);
    t0 = (int16_t) H[0];
    t1 = (int16_t) H[1];
    A = __SRA2(E, s1);
    B = __SRA2(G, s1);
    /* C0 = (xb - xd), C1 = (yd - yb) */
    C = __PACK2(-t1, t0);
    /* D0 = (xd - xb), D1 = (yb - yd) */
    D = __PACK2(t1, -t0);
    /* E0 = (ya+yc) - (yb+yd), E1 = (xa+xc) - (xb+xd) */
    E = __SUB2(E, G);
    /* G1 = (ya-yc) + (xb-xd), G0 = (xa-xc) - (yb-yd) */
    G = __ADD2(F, C);
    /* H1 = (ya-yc) - (xb-xd), H0 = (xa-xc) + (yb-yd) */
    H = __ADD2(F, D);
    /* xc' = (xa-xb+xc-xd)* co2 + (ya-yb+yc-yd)* (si2) */
    /* yc' = (ya-yb+yc-yd)* co2 - (xa-xb+xc-xd)* (si2) */
    t0 = (int16_t)(__DOTP2(CoSi2, E) >> 16U);
    t1 = (int16_t)(__DOTP2(C2, E) >> 16U);
    /* xb' = (xa+yb-xc-yd)* co1 + (ya-xb-yc+xd)* (si1) */
    /* yb' = (ya-xb-yc+xd)* co1 - (xa+yb-xc-yd)* (si1) */
    t2 = (int16_t)(__DOTP2(CoSi1, H) >> 16U);
    t3 = (int16_t)(__DOTP2(C1, H) >> 16U);
    /* xd' = (xa-yb-xc+yd)* Co3 + (ya+xb-yc-xd)* (si3) */
    /* yd' = (ya+xb-yc-xd)* Co3 - (xa-yb-xc+yd)* (si3) */
    t4 = (int16_t)(__DOTP2(CoSi3, G) >> 16U);
    t5 = (int16_t)(__DOTP2(C3, G) >> 16U);
    /* ya' = ya + yb + yc + yd */
    /* xa' = xa + xb + xc + xd */
    A = __ADD2(A, B);
    E = __PACK2(t0, t1);
    F = __PACK2(t2, t3);
    G = __PACK2(t4, t5);
    *((v2s *)&pIn[i0 * 2U]) = A;
    *((v2s *)&pIn[i1 * 2U]) = E;
    *((v2s *)&pIn[i2 * 2U]) = F;
    *((v2s *)&pIn[i3 * 2U]) = G;
    #else
    v2s s1, s2;
    /* Read yb (real), xb(imag) input */
    B = *(v2s *)&pIn[i1 * 2U];
    /* Read yd (real), xd(imag) input */
    D = *(v2s *)&pIn[i3 * 2U];
    /* Read ya (real), xa (imag) input */
    A = *(v2s *)&pIn[i0 * 2U];
    /* Read yc (real), xc(imag) input */
    C = *(v2s *)&pIn[i2 * 2U];
    asm volatile (
    "addi %[s1], zero, 1;"
    "slli %[s1], %[s1], 0x10;"
    "addi %[s1], %[s1], 1;"
    "addi %[s2], zero, 2;"
    "slli %[s2], %[s2], 0x10;"
    "addi %[s2], %[s2], 2;"
    "pv.sra.h  %[B],%[B],%[s2];"
    "pv.sra.h  %[D],%[D],%[s2];"
    "pv.sra.h  %[A],%[A],%[s2];"
    "pv.sra.h  %[C],%[C],%[s2];"
    "pv.add.h  %[G],%[B],%[D];"
    "pv.sub.h  %[H],%[B],%[D];"
    "pv.add.h  %[E],%[A],%[C];"
    "pv.sub.h  %[F],%[A],%[C];"
    "pv.extract.h  %[t0],%[H],0;"
    "pv.extract.h  %[t1],%[H],1;"
    "pv.sra.h  %[A],%[E],%[s1];"
    "pv.sra.h  %[B],%[G],%[s1];"
    "sub %[t3],zero,%[t1];"
    "pv.pack.h %[C],%[t3],%[t0];"
    "sub %[t4],zero,%[t0];"
    "pv.pack.h %[D],%[t1],%[t4];"
    "pv.sub.h  %[E],%[E],%[G];"
    "pv.add.h  %[G],%[F],%[C];"
    "pv.add.h  %[H],%[F],%[D];"
    "pv.dotsp.h  %[C],%[CoSi2],%[E];"
    "pv.dotsp.h  %[D],%[C2],%[E];"
    "pv.dotsp.h  %[E],%[CoSi1],%[H];"
    "pv.dotsp.h  %[F],%[C1],%[H];"
    "srai  %[t0],%[C],0x10;"
    "srai  %[t1],%[D],0x10;"
    "pv.dotsp.h  %[C],%[CoSi3],%[G];"
    "pv.dotsp.h  %[D],%[C3],%[G];"
    "srai  %[t2],%[E],0x10;"
    "srai  %[t3],%[F],0x10;"
    "srai  %[t4],%[C],0x10;"
    "srai  %[t5],%[D],0x10;"
    "pv.add.h  %[A],%[A],%[B];"
    "pv.pack.h %[E],%[t0],%[t1];"
    "pv.pack.h %[F],%[t2],%[t3];"
    "pv.pack.h %[G],%[t4],%[t5];"
    : [A] "+&r" (A), [B] "+&r" (B), [C] "+&r" (C), [D] "+&r" (D),
      [E] "=&r" (E), [F] "=&r" (F), [G] "=&r" (G), [H] "=&r" (H),
      [t0] "=&r" (t0), [t1] "=&r" (t1), [t2] "=&r" (t2), [t3] "=&r" (t3),
      [t4] "=&r" (t4), [t5] "=&r" (t5), [s1] "=&r" (s1), [s2] "=&r" (s2),
      [C1] "+r" (C1), [C2] "+r" (C2), [C3] "+r" (C3) :
      [CoSi1] "r" (CoSi1), [CoSi2] "r" (CoSi2), [CoSi3] "r" (CoSi3)
    : );
    *((v2s *)&pIn[i0 * 2U]) = A;
    *((v2s *)&pIn[i1 * 2U]) = E;
    *((v2s *)&pIn[i2 * 2U]) = F;
    *((v2s *)&pIn[i3 * 2U]) = G;
    #endif
}

static inline void radix4_butterfly_middle(  int16_t* pIn,
                                              uint32_t i0,
                                              uint32_t n2,
                                              v2s CoSi1,
                                              v2s CoSi2,
                                              v2s CoSi3) {
    int16_t t0, t1, t2, t3, t4, t5;
    uint32_t i1, i2, i3;
    v2s A, B, C, D, E, F, G, H;
    v2s C1, C2, C3;

    /*  index calculation for the input as, */
    /*  pIn[i0 + 0], pIn[i0 + fftLen/4], pIn[i0 + fftLen/2], pIn[i0 +
     * 3fftLen/4] */
    i1 = i0 + n2;
    i2 = i1 + n2;
    i3 = i2 + n2;
    asm volatile(
    "pv.extract.h  %[t1],%[CoSi1],1;"
    "pv.extract.h  %[t3],%[CoSi2],1;"
    "pv.extract.h  %[t5],%[CoSi3],1;"
    "pv.extract.h  %[t0],%[CoSi1],0;"
    "pv.extract.h  %[t2],%[CoSi2],0;"
    "pv.extract.h  %[t4],%[CoSi3],0;"
    "sub           %[t1],zero,%[t1];"
    "sub           %[t3],zero,%[t3];"
    "sub           %[t5],zero,%[t5];"
    "pv.pack.h %[C1],%[t1],%[t0];"
    "pv.pack.h %[C2],%[t3],%[t2];"
    "pv.pack.h %[C3],%[t5],%[t4];"
    : [C1] "=r" (C1), [C2] "=r" (C2), [C3] "=r" (C3),
      [t0] "=&r" (t0), [t1] "=&r" (t1), [t2] "=&r" (t2), [t3] "=&r" (t3),
      [t4] "=&r" (t4), [t5] "=&r" (t5)
    : [CoSi1] "r" (CoSi1), [CoSi2] "r" (CoSi2), [CoSi3] "r" (CoSi3)
    : );

    #ifndef ASM
    v2s s1 = {1, 1};
    /* Read yb (real), xb(imag) input */
    B = *(v2s *)&pIn[i1 * 2U];
    /* Read yd (real), xd(imag) input */
    D = *(v2s *)&pIn[i3 * 2U];
    /* Read ya (real), xa(imag) input */
    A = *(v2s *)&pIn[i0 * 2U];
    /* Read yc (real), xc(imag) input */
    C = *(v2s *)&pIn[i2 * 2U];
    /* G0 = (yb + yd), G1 = (xb + xd) */
    G = __ADD2(B, D);
    /* H0 = (yb - yd), H1 = (xb - xd) */
    H = __SUB2(B, D);
    /* E0 = (ya + yc), E1 = (xa + xc) */
    E = __ADD2(A, C);
    /* F0 = (ya - yc), F1 =(xa - xc) */
    F = __SUB2(A, C);
    G = __SRA2(G, s1);
    H = __SRA2(H, s1);
    E = __SRA2(E, s1);
    F = __SRA2(F, s1);
    t0 = (int16_t) H[0];
    t1 = (int16_t) H[1];
    /* C0 = (ya+yc) - (yb+yd), C1 = (xa+xc) - (xb+xd) */
    C = __SUB2(E, G);
    /* D0 = (ya+yc) + (yb+yd), D1 = (xa+xc) + (xb+xd) */
    D = __ADD2(E, G);
    /* A0 = (xb-xd), A1 = (yd-yb) */
    A = __PACK2(t1, -t0);
    /* B0 = (xd-xb), B1 = (yb-yd) */
    B = __PACK2(-t1, t0);
    /* xa' = xa + xb + xc + xd */
    /* ya' = ya + yb + yc + yd */
    *((v2s *)&pIn[i0 * 2U]) = __SRA2(D, s1);
    /* E1 = (ya-yc) + (xb-xd),  E0 = (xa-xc) - (yb-yd)) */
    E = __ADD2(F, A);
    /* F1 = (ya-yc) - (xb-xd), F0 = (xa-xc) + (yb-yd)) */
    F = __ADD2(F, B);
    /* xc' = (xa-xb+xc-xd)* co2 + (ya-yb+yc-yd)* (si2) */
    /* yc' = (ya-yb+yc-yd)* co2 - (xa-xb+xc-xd)* (si2) */
    t0 = (int16_t)(__DOTP2(CoSi2, C) >> 16U);
    t1 = (int16_t)(__DOTP2(C2, C) >> 16U);
    /* xb' = (xa+yb-xc-yd)* co1 + (ya-xb-yc+xd)* (si1) */
    /* yb' = (ya-xb-yc+xd)* co1 - (xa+yb-xc-yd)* (si1) */
    t2 = (int16_t)(__DOTP2(CoSi1, F) >> 16U);
    t3 = (int16_t)(__DOTP2(C1, F) >> 16U);
    /* xd' = (xa-yb-xc+yd)* Co3 + (ya+xb-yc-xd)* (si3) */
    /* yd' = (ya+xb-yc-xd)* Co3 - (xa-yb-xc+yd)* (si3) */
    t4 = (int16_t)(__DOTP2(CoSi3, E) >> 16U);
    t5 = (int16_t)(__DOTP2(C3, E) >> 16U);
    A = __PACK2(t0, t1);
    B = __PACK2(t2, t3);
    C = __PACK2(t4, t5);
    *((v2s *)&pIn[i1 * 2U]) = A;
    *((v2s *)&pIn[i2 * 2U]) = B;
    *((v2s *)&pIn[i3 * 2U]) = C;
    #else
    v2s s1;
    /* Read yb (real), xb(imag) input */
    B = *(v2s *)&pIn[i1 * 2U];
    /* Read yd (real), xd(imag) input */
    D = *(v2s *)&pIn[i3 * 2U];
    /* Read ya (real), xa(imag) input */
    A = *(v2s *)&pIn[i0 * 2U];
    /* Read yc (real), xc(imag) input */
    C = *(v2s *)&pIn[i2 * 2U];
    asm volatile (
    "pv.add.h  %[G],%[B],%[D];"
    "pv.sub.h  %[H],%[B],%[D];"
    "pv.add.h  %[E],%[A],%[C];"
    "pv.sub.h  %[F],%[A],%[C];"
    "addi %[s1], zero, 0x01;"
    "slli %[s1], %[s1], 0x10;"
    "addi %[s1], %[s1], 0x01;"
    "pv.sra.h  %[G],%[G],%[s1];"
    "pv.sra.h  %[H],%[H],%[s1];"
    "pv.sra.h  %[E],%[E],%[s1];"
    "pv.sra.h  %[F],%[F],%[s1];"
    "pv.extract.h  %[t0],%[H],0;"
    "pv.extract.h  %[t1],%[H],1;"
    "pv.sub.h  %[C],%[E],%[G];"
    "pv.add.h  %[D],%[E],%[G];"
    "sub %[t3],zero,%[t0];"
    "pv.pack.h %[A],%[t1],%[t3];"
    "sub %[t4],zero,%[t1];"
    "pv.pack.h %[B],%[t4],%[t0];"
    "pv.sra.h  %[D],%[D],%[s1];"
    "pv.add.h  %[E],%[F],%[A];"
    "pv.add.h  %[F],%[F],%[B];"
    "pv.dotsp.h  %[G],%[CoSi2],%[C];"
    "pv.dotsp.h  %[H],%[C2],%[C];"
    "pv.dotsp.h  %[A],%[CoSi1],%[F];"
    "pv.dotsp.h  %[B],%[C1],%[F];"
    "srai  %[t0],%[G],0x10;"
    "srai  %[t1],%[H],0x10;"
    "pv.dotsp.h  %[G],%[CoSi3],%[E];"
    "pv.dotsp.h  %[H],%[C3],%[E];"
    "srai  %[t2],%[A],0x10;"
    "srai  %[t3],%[B],0x10;"
    "srai  %[t4],%[G],0x10;"
    "srai  %[t5],%[H],0x10;"
    "pv.pack.h %[A],%[t0],%[t1];"
    "pv.pack.h %[B],%[t2],%[t3];"
    "pv.pack.h %[C],%[t4],%[t5];"
    : [A] "+&r" (A), [B] "+&r" (B), [C] "+&r" (C), [D] "+&r" (D),
      [E] "=&r" (E), [F] "=&r" (F), [G] "=&r" (G), [H] "=&r" (H),
      [t0] "=&r" (t0), [t1] "=&r" (t1), [t2] "=&r" (t2), [t3] "=&r" (t3),
      [t4] "=&r" (t4), [t5] "=&r" (t5), [s1] "=&r" (s1),
      [C1] "+r" (C1), [C2] "+r" (C2), [C3] "+r" (C3) :
      [CoSi1] "r" (CoSi1), [CoSi2] "r" (CoSi2), [CoSi3] "r" (CoSi3)
    : );
    *((v2s *)&pIn[i0 * 2U]) = D;
    *((v2s *)&pIn[i1 * 2U]) = A;
    *((v2s *)&pIn[i2 * 2U]) = B;
    *((v2s *)&pIn[i3 * 2U]) = C;
    #endif
}

static inline void radix4_butterfly_last(    int16_t* pIn,
                                              uint32_t i0,
                                              uint32_t n2) {
    int16_t t0, t1;
    uint32_t i1, i2, i3;
    v2s A, B, C, D, E, F, G, H;

    /*  index calculation for the input as, */
    /*  pIn[i0 + 0], pIn[i0 + fftLen/4],
        pIn[i0 + fftLen/2], pIn[i0 + 3fftLen/4] */
    i1 = i0 + n2;
    i2 = i1 + n2;
    i3 = i2 + n2;
    #ifndef ASM
    v2s s1 = {1, 1};
    /* Read yb (real), xb(imag) input */
    B = *(v2s *)&pIn[i1 * 2U];
    /* Read yd (real), xd(imag) input */
    D = *(v2s *)&pIn[i3 * 2U];
    /* Read ya (real), xa(imag) input */
    A = *(v2s *)&pIn[i0 * 2U];
    /* Read yc (real), xc(imag) input */
    C = *(v2s *)&pIn[i2 * 2U];
    /* H0 = (yb-yd), H1 = (xb-xd) */
    H = __SUB2(B, D);
    /* G0 = (yb+yd), G1 = (xb+xd) */
    G = __ADD2(B, D);
    /* E0 = (ya+yc), E1 = (xa+xc) */
    E = __ADD2(A, C);
    /* F0 = (ya-yc), F1 = (xa-xc) */
    F = __SUB2(A, C);
    H = __SRA2(H, s1);
    G = __SRA2(G, s1);
    E = __SRA2(E, s1);
    t0 = (int16_t) H[0];
    t1 = (int16_t) H[1];
    F = __SRA2(F, s1);
    /* xa' = (xa+xb+xc+xd) */
    /* ya' = (ya+yb+yc+yd) */
    *((v2s *)&pIn[i0 * 2U]) = __ADD2(E, G);
    /* A0 = (xb-xd), A1 = (yd-yb) */
    A = __PACK2(t1, -t0);
    /* B0 = (xd-xb), B1 = (yb-yd) */
    B = __PACK2(-t1, t0);
    /* xc' = (xa-xb+xc-xd) */
    /* yc' = (ya-yb+yc-yd) */
    E = __SUB2(E, G);
    /* xb' = (xa+yb-xc-yd) */
    /* yb' = (ya-xb-yc+xd) */
    A = __ADD2(F, A);
    /* xd' = (xa-yb-xc+yd) */
    /* yd' = (ya+xb-yc-xd) */
    B = __ADD2(F, B);
    *((v2s *)&pIn[i1 * 2U]) = E;
    *((v2s *)&pIn[i2 * 2U]) = A;
    *((v2s *)&pIn[i3 * 2U]) = B;
    #else
    /* Read yb (real), xb(imag) input */
    B = *(v2s *)&pIn[i1 * 2U];
    /* Read yd (real), xd(imag) input */
    D = *(v2s *)&pIn[i3 * 2U];
    /* Read ya (real), xa(imag) input */
    A = *(v2s *)&pIn[i0 * 2U];
    /* Read yc (real), xc(imag) input */
    C = *(v2s *)&pIn[i2 * 2U];
    int16_t t2, t3;
    v2s s1;
    asm volatile (
    "pv.sub.h  %[H],%[B],%[D];"
    "pv.add.h  %[G],%[B],%[D];"
    "pv.add.h  %[E],%[A],%[C];"
    "pv.sub.h  %[F],%[A],%[C];"
    "addi %[s1], zero, 1;"
    "slli %[s1], %[s1], 0x10;"
    "addi %[s1], %[s1], 1;"
    "pv.sra.h  %[H],%[H],%[s1];"
    "pv.sra.h  %[G],%[G],%[s1];"
    "pv.sra.h  %[E],%[E],%[s1];"
    "pv.extract.h  %[t0],%[H],0;"
    "pv.extract.h  %[t1],%[H],1;"
    "pv.sra.h  %[F],%[F],%[s1];"
    "sub %[t2], zero, %[t0];"
    "pv.pack.h %[A],%[t1],%[t2];"
    "sub %[t3],zero,%[t1];"
    "pv.pack.h %[B],%[t3],%[t0];"
    "pv.add.h  %[H],%[E],%[G];"
    "pv.sub.h  %[E],%[E],%[G];"
    "pv.add.h  %[A],%[F],%[A];"
    "pv.add.h  %[B],%[F],%[B];"
    : [A] "+&r" (A), [B] "+&r" (B), [C] "+&r" (C), [D] "+&r" (D),
      [E] "=&r" (E), [F] "=&r" (F), [G] "=&r" (G), [H] "=&r" (H),
      [t0] "=&r" (t0), [t1] "=&r" (t1), [t2] "=&r" (t2), [t3] "=&r" (t3), [s1] "=&r" (s1)
    : : );
    *((v2s *)&pIn[i0 * 2U]) = H;
    *((v2s *)&pIn[i1 * 2U]) = E;
    *((v2s *)&pIn[i2 * 2U]) = A;
    *((v2s *)&pIn[i3 * 2U]) = B;
    #endif
}

#endif
