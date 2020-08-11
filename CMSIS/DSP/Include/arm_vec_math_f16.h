/******************************************************************************
 * @file     arm_vec_math_f16.h
 * @brief    Public header file for CMSIS DSP Library
 ******************************************************************************/
/*
 * Copyright (c) 2010-2020 Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _ARM_VEC_MATH_H
#define _ARM_VEC_MATH_H

#include "arm_math_types_f16.h"
#include "arm_common_tables_f16.h"
#include "arm_helium_utils.h"

#ifdef   __cplusplus
extern "C"
{
#endif

#if defined(ARM_FLOAT16_SUPPORTED)


#if (defined(ARM_MATH_MVEF) || defined(ARM_MATH_HELIUM)) && !defined(ARM_MATH_AUTOVECTORIZE)


static const float16_t __logf_rng_f16=0.693147180f16;

/* fast inverse approximation (3x newton) */
__STATIC_INLINE f16x8_t vrecip_medprec_f16(
    f16x8_t x)
{
    q15x8_t         m;
    f16x8_t         b;
    any16x8_t       xinv;
    f16x8_t         ax = vabsq(x);

    xinv.f = ax;

    m = 0x03c00 - (xinv.i & 0x07c00);
    xinv.i = xinv.i + m;
    xinv.f = 1.41176471f16 - 0.47058824f16 * xinv.f;
    xinv.i = xinv.i + m;

    b = 2.0f16 - xinv.f * ax;
    xinv.f = xinv.f * b;

    b = 2.0f16 - xinv.f * ax;
    xinv.f = xinv.f * b;

    b = 2.0f16 - xinv.f * ax;
    xinv.f = xinv.f * b;

    xinv.f = vdupq_m(xinv.f, F16INFINITY, vcmpeqq(x, 0.0f));
    /*
     * restore sign
     */
    xinv.f = vnegq_m(xinv.f, xinv.f, vcmpltq(x, 0.0f));

    return xinv.f;
}

/* fast inverse approximation (4x newton) */
__STATIC_INLINE f16x8_t vrecip_hiprec_f16(
    f16x8_t x)
{
    q15x8_t         m;
    f16x8_t         b;
    any16x8_t       xinv;
    f16x8_t         ax = vabsq(x);

    xinv.f = ax;

    m = 0x03c00 - (xinv.i & 0x07c00);
    xinv.i = xinv.i + m;
    xinv.f = 1.41176471f16 - 0.47058824f16 * xinv.f;
    xinv.i = xinv.i + m;

    b = 2.0f16 - xinv.f * ax;
    xinv.f = xinv.f * b;

    b = 2.0f16 - xinv.f * ax;
    xinv.f = xinv.f * b;

    b = 2.0f16 - xinv.f * ax;
    xinv.f = xinv.f * b;

    b = 2.0f16 - xinv.f * ax;
    xinv.f = xinv.f * b;

    xinv.f = vdupq_m(xinv.f, F16INFINITY, vcmpeqq(x, 0.0f));
    /*
     * restore sign
     */
    xinv.f = vnegq_m(xinv.f, xinv.f, vcmpltq(x, 0.0f));

    return xinv.f;
}

__STATIC_INLINE f16x8_t vdiv_f16(
    f16x8_t num, f16x8_t den)
{
    return vmulq(num, vrecip_hiprec_f16(den));
}


/**
  @brief         Single-precision taylor dev.
  @param[in]     x              f16  vector input
  @param[in]     coeffs         f16  vector coeffs
  @return        destination    f16  vector
 */

__STATIC_INLINE float16x8_t vtaylor_polyq_f16(
        float16x8_t           x,
        const float16_t * coeffs)
{
    float16x8_t         A = vfmasq(vdupq_n_f16(coeffs[4]), x, coeffs[0]);
    float16x8_t         B = vfmasq(vdupq_n_f16(coeffs[6]), x, coeffs[2]);
    float16x8_t         C = vfmasq(vdupq_n_f16(coeffs[5]), x, coeffs[1]);
    float16x8_t         D = vfmasq(vdupq_n_f16(coeffs[7]), x, coeffs[3]);
    float16x8_t         x2 = vmulq(x, x);
    float16x8_t         x4 = vmulq(x2, x2);
    float16x8_t         res = vfmaq(vfmaq_f16(A, B, x2), vfmaq_f16(C, D, x2), x4);

    return res;
}

__STATIC_INLINE float16x8_t vmant_exp_f16(
    float16x8_t     x,
    int16x8_t * e)
{
    any16x8_t       r;
    int16x8_t       n;

    r.f = x;
    n = r.i >> 10;
    n = n - 15;
    r.i = r.i - (n << 10);

    *e = n;
    return r.f;
}


__STATIC_INLINE float16x8_t vlogq_f16(float16x8_t vecIn)
{
    q15x8_t             vecExpUnBiased;
    float16x8_t         vecTmpFlt0, vecTmpFlt1;
    float16x8_t         vecAcc0, vecAcc1, vecAcc2, vecAcc3;
    float16x8_t         vecExpUnBiasedFlt;

    /*
     * extract exponent
     */
    vecTmpFlt1 = vmant_exp_f16(vecIn, &vecExpUnBiased);

    vecTmpFlt0 = vecTmpFlt1 * vecTmpFlt1;
    /*
     * a = (__logf_lut_f16[4] * r.f) + (__logf_lut_f16[0]);
     */
    vecAcc0 = vdupq_n_f16(__logf_lut_f16[0]);
    vecAcc0 = vfmaq(vecAcc0, vecTmpFlt1, __logf_lut_f16[4]);
    /*
     * b = (__logf_lut_f16[6] * r.f) + (__logf_lut_f16[2]);
     */
    vecAcc1 = vdupq_n_f16(__logf_lut_f16[2]);
    vecAcc1 = vfmaq(vecAcc1, vecTmpFlt1, __logf_lut_f16[6]);
    /*
     * c = (__logf_lut_f16[5] * r.f) + (__logf_lut_f16[1]);
     */
    vecAcc2 = vdupq_n_f16(__logf_lut_f16[1]);
    vecAcc2 = vfmaq(vecAcc2, vecTmpFlt1, __logf_lut_f16[5]);
    /*
     * d = (__logf_lut_f16[7] * r.f) + (__logf_lut_f16[3]);
     */
    vecAcc3 = vdupq_n_f16(__logf_lut_f16[3]);
    vecAcc3 = vfmaq(vecAcc3, vecTmpFlt1, __logf_lut_f16[7]);
    /*
     * a = a + b * xx;
     */
    vecAcc0 = vfmaq(vecAcc0, vecAcc1, vecTmpFlt0);
    /*
     * c = c + d * xx;
     */
    vecAcc2 = vfmaq(vecAcc2, vecAcc3, vecTmpFlt0);
    /*
     * xx = xx * xx;
     */
    vecTmpFlt0 = vecTmpFlt0 * vecTmpFlt0;
    vecExpUnBiasedFlt = vcvtq_f16_s16(vecExpUnBiased);
    /*
     * r.f = a + c * xx;
     */
    vecAcc0 = vfmaq(vecAcc0, vecAcc2, vecTmpFlt0);
    /*
     * add exponent
     * r.f = r.f + ((float32_t) m) * __logf_rng_f16;
     */
    vecAcc0 = vfmaq(vecAcc0, vecExpUnBiasedFlt, __logf_rng_f16);
    // set log0 down to -inf
    vecAcc0 = vdupq_m(vecAcc0, -F16INFINITY, vcmpeqq(vecIn, 0.0f));
    return vecAcc0;
}

__STATIC_INLINE float16x8_t vexpq_f16(
    float16x8_t x)
{
    // Perform range reduction [-log(2),log(2)]
    int16x8_t       m = vcvtq_s16_f16(vmulq_n_f16(x, 1.4426950408f16));
    float16x8_t     val = vfmsq_f16(x, vcvtq_f16_s16(m), vdupq_n_f16(0.6931471805f16));

    // Polynomial Approximation
    float16x8_t         poly = vtaylor_polyq_f16(val, exp_tab_f16);

    // Reconstruct
    poly = (float16x8_t) (vqaddq_s16((int16x8_t) (poly), vqshlq_n_s16(m, 10)));

    poly = vdupq_m(poly, 0.0f, vcmpltq_n_s16(m, -14));
    return poly;
}

__STATIC_INLINE float16x8_t arm_vec_exponent_f16(float16x8_t x, int16_t nb)
{
    float16x8_t         r = x;
    nb--;
    while (nb > 0) {
        r = vmulq(r, x);
        nb--;
    }
    return (r);
}

__STATIC_INLINE f16x8_t vpowq_f16(
    f16x8_t val,
    f16x8_t n)
{
    return vexpq_f16(vmulq_f16(n, vlogq_f16(val)));
}


#endif /* (defined(ARM_MATH_MVEF) || defined(ARM_MATH_HELIUM)) && !defined(ARM_MATH_AUTOVECTORIZE)*/



#ifdef   __cplusplus
}
#endif

#endif /* ARM FLOAT16 SUPPORTED */

#endif /* _ARM_VEC_MATH_H */

/**
 *
 * End of file.
 */
