
/*HEADER**********************************************************************
*
* Copyright 2013 Freescale Semiconductor, Inc.
*
* This software is owned or controlled by Freescale Semiconductor.
* Use of this software is governed by the Freescale MQX RTOS License
* distributed with this Material.
* See the MQX_RTOS_LICENSE file distributed for more details.
*
* Brief License Summary:
* This software is provided in source form for you to use free of charge,
* but it is not open source software. You are allowed to use this software
* but you cannot redistribute it or derivative works of it in source form.
* The software may be used only in connection with a product containing
* a Freescale microprocessor, microcontroller, or digital signal processor.
* See license agreement file for full license terms including other restrictions.
*****************************************************************************
*
* Comments:
*
*   This is a private header.
*
*
*END************************************************************************/
#ifndef __fp_prv_h__
#define __fp_prv_h__

/*----------------------------------------------------------------------*/
/*
 *                          CONSTANT DEFINITIONS
 */

/* Floating point Default PRECision Value.  */
#define FDPRECV         (6)

/* Number of decimal digits to the left of an e format output. */
#define LEFTOFE         (1)

/*
 * Number of decimal digits to the left
 * of an f format output.  This is
 * presumably a reasonable maximum.
 */
#define LEFTOFF         (308)

/*
 * IEEE 754 single-precision binary floating-point format
 * The maximum number of chars in string that can represent any IEE 754 floating number is determined.
 * The sign is one bit and represents max. one char '+' / '-' or nothing
 * The exponent has 8 bits and its meaning is the floating point multiplication from E = 2^(-127) to 2^(+128),
 * The fraction has 23 bits and it can take maximum 23 chars, when the last bit of fraction is set:
 *   2^(-1) = 0.5 (first decimal place set)
 *   2^(-2) = 0.25 (second decimal place set)
 *   2^(-23) = 0.xxxxx5 (23rd decimal place set)\
 *   so the fraction is from (0.xxx...xxx5 to 1.yyy...yyy5)
 *  The exponent can only multiplicate the fraction:
 *    in the case of negative E we will get more decimal places than in the case of positive E
 *    the smallest negative E value is -127.
 *   2^(-127) = 0.zzzzz5 (127th place is set to 5)
 *  So the maximum number of chars needed to display IEEE 754 value is:
 *  max(S) + max('0') + max('.') + max(E) + max(F) = 1 + 1 + 1 + 127 + 23 = 153
 *  and add zero-ending string space (1B)
 */

#define NDIG            (154)
#define MAX_PRECISION   (17)

#define FP_WORDSIZE     (16)
#define FP_NUMWORDS     (4)
#define FP_FLT          (0)
#define FP_DBL          (1)
#define BIAS            (1023)
#define MAX_EXP         (2046)
#define MIN_EXP         (-51)
#define FMAX_EXP        (254)
#define FMIN_EXP        (-22)

#ifndef DBL_MAX_10_EXP
#define DBL_MAX_10_EXP  (308)
#endif

#ifndef HUGE_VAL
#define HUGE_VAL         99.e999
#endif

#if PSP_ENDIAN == MQX_LITTLE_ENDIAN
#define HIGH              (1)
#define LOW               (0)
#else
#define HIGH              (0)
#define LOW               (1)
#endif

#define BYTES_PER_DOUBLE  (8)
#define MANT_BITS         (52)
#define NON_MANT_BITS     (12)
#define EXP_BIAS          (1023)

/*--------------------------------------------------------------------------*/
/*
 *                            MACRO DEFINTIONS
 */

#define EXP_MASK        (0x7ff00000L)
#define FEXP_MASK       (0x7f800000L)

#define SIGN_MASK       (0x80000000L)

#define MAXEXP_MASK     (0x000007ffL)
#define FMAXEXP_MASK    (0x000000ffL)

#define MANT_MASK       (0x000fffffL)
#define FMANT_MASK      (0x007fffffL)

#define HIDDEN_MASK     (0x00100000L)
#define FHIDDEN_MASK    (0x00800000L)

/* macros for a, where a is the address of 64 bits of ieee double */
#define EXPOF(a)        (((a)[HIGH] & EXP_MASK) >> 20)
#define FEXPOF(a)       (((a) & FEXP_MASK) >> 23)

#define SIGNOF(a)       (((a)[HIGH] & SIGN_MASK) != 0)
#define FSIGNOF(a)      (((a) & SIGN_MASK) != 0)

#define ISZERO(a)       ( !( ((a)[HIGH] & ~SIGN_MASK) | ((a)[LOW])) )
#define FISZERO(a)      (((a) & 0x7fffffffL) == 0)

#define MANTZERO(a)     ( !( ((a)[HIGH] &  MANT_MASK) | ((a)[LOW])) )
#define FMANTZERO(a)    (((a) &  FMANT_MASK) == 0)

#define ISINFINITY(a)   ((EXPOF(a) == MAXEXP_MASK) &&  MANTZERO(a))
#define FISINFINITY(a)  ((FEXPOF(a) == FMAXEXP_MASK) &&  FMANTZERO(a))

#define ISNAN(a)        ((EXPOF(a) == MAXEXP_MASK) && !(MANTZERO(a)))
#define FISNAN(a)       ((FEXPOF(a) == FMAXEXP_MASK) && !(FMANTZERO(a)))

int32_t     _dtoa(int32_t, char *);
int32_t     _dtoe(char *, double, char *, char, char, char, int32_t, char);
int32_t     _dtof(char *, double, char *, char, char, char, int32_t, char);
int32_t     _dtog(char *, double, char *, char, char, char, int32_t, char);
#endif
