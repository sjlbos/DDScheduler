
/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
* Copyright 2004-2008 Embedded Access Inc.
* Copyright 1989-2008 ARC International
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
*   This file contains the functions for the formatting of
*   output for floating point numbers
*
*
*END************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "fp_prv.h"

double modf( double , double  *);

#if 0 /* Currently not needed, may be reused in the future. */
double strtod(char  *, char **);
#endif

#if 0 /* Currently not needed, may be reused in the future. */
/*!
 * \brief Converts a string to a double.
 *
 * \param[in]     nptr   Pointer to the string containing the number.
 * \param[in,out] endptr Pointer to the string address after the number.
 *
 * \return Converted string.
 * \return 0.0 (Failure.)
 */
static double _strtod( char  *nptr, char  **endptr)
{

   return strtod(nptr, endptr);

}
#endif

/*!
 * \brief Gets the fractional part.
 *
 * \param[in]  input_dbl   The number to split.
 * \param[out] intpart_ptr Where to write the integral part of the double.
 *
 * \return Fractional part of number.
 * \return 0 if exponent is larger than mantissa
 */
static double _modf(double input_dbl, double  *intpart_ptr)
{
   return modf(input_dbl, intpart_ptr);
}

/*!
 * \brief Converts argument to ASCII string.
 *
 * \param[in]  arg     The number to be converted.
 * \param[in]  ndigits The maximal width of the field.
 * \param[out] decpt   Where the decimal place should be.
 * \param[out] sign    The sign of the number.
 * \param[in]  eflag   Is this an E format number.
 * \param[out] buf     The character string of digits for the number.
 * \param[in]  type    Use lower or upper case.
 *
 * \return ASCII string.
 */
static char  *_cvt(double arg, int32_t ndigits, int32_t *decpt, char *sign, char eflag, char *buf, char type)
{
    int32_t      r2;
    double      intpart;
    double      tmpintpart;
    char   *p;
    char       *p1;
    uint32_t      i;
    int32_t      tmp = 0;

   if (ndigits < 0)
   {
      ndigits = 0;
   }
   if (ndigits >= NDIG-1)
   {
      ndigits = NDIG-2;
   }
   r2 = 0;
   p = &buf[0];
   if (SIGNOF((unsigned long *)((void *)&arg)))
   {
        *sign = (char)true;
        arg = -arg;
   }
   else
   {
        *sign = (char)false;
   }
   if (ISINFINITY((unsigned long *)((void *)&arg)))
   {
      buf[0] = 'i';
      return(buf);
   }
   if (ISNAN((unsigned long *)((void *)&arg)))
   {
      *sign = (char)false;
      buf[0] = 'n';
      return(buf);
   }
   if (ISZERO((unsigned long *)((void *)&arg)))
   {
      if (( type == 'e') || (type == 'E'))
      {
        *decpt = 1; /* avoid e_or_E type create string 0.000e-1 */
      }
      else
      {
        *decpt = 0;
      }
      for(i = 0; i < ndigits; i++)
      {
        buf[i] = '0';
      }
      buf[ndigits] = '\0';
      return(buf);
   }

   /* modf return fraction part and set integer part */
   arg = _modf(arg, (double *)(&intpart));

   /*
    * arg has decimal part, intpart has integer part.
    * take the inparts digits one by one and stick them into the end
    * of the buffer then move them to the beginning of it.
    */
   if (intpart != 0)
   {
      p1 = &buf[NDIG];
      while (intpart != 0)
      {
            /*
             * It's possible that inaccuracy will cause the division
             * by 10 to come out a little bit low.  In C, conversions
             * from floating point to integer are done by truncation.
             * We add in .03 so that temp will be rounded up when it
             * is converted to the nearest number of tenths, as done
             * by the call to modf and the assignment to i.
             */
         tmpintpart = (intpart / 10) + 0.03;
         tmpintpart = _modf(tmpintpart, (double *)(&intpart));
         tmpintpart = tmpintpart * 10.0;
         *--p1 = (char)((uint32_t)tmpintpart + '0');
         r2++;
      }
      while (p1 < &buf[NDIG])
      {
         *p++ = *p1++;
      }
   }
   else
   {

      /*
       * if the intpart was zero, then get the decimal part so that it
       * does not have any zero directly to the right of the decimal
       * point.  with each shift left of the decimal number adjust the
       * exponent.
       */
      if (arg > 0)
      {
        double temp;
        while (1 > (temp = arg*10))
        {
            arg = temp;
            r2--;
         }
      }

   }

   p1 = &buf[ndigits];
   if (eflag == 0)
   {
      tmp = r2;
      /*
       * Loop around adjusting the pointer up or down in the buffer.
       * We must be careful not to move the pointer out of the range
       * of the buffer.  If the pointer goes outside that range and
       * the buffer is in very low or very high memory, the pointer
       * value could "wrap" and cause later pointer comparisons to fail.
       */
      if (tmp > 0)
      {
         while (tmp && p1 < &buf[NDIG-1])
         {
            p1++;
            tmp--;
         }
      }
      else
      {
         while (tmp && p1 > buf)
         {
            p1--;
            tmp++;
         }
         if (tmp < 0)
         {      /* pointer would have underflowed buffer */
            for (i = 0; i < ndigits; i++)
            {
                buf[i] = '0';
            }
            *decpt = 0;
            buf[ndigits] = '\0';
               /*
                * This indicates that the number is
                * smaller than the precision they've
                * given us.  The result will be 0.0....
                */
            return(buf);
         }
      }
   }
   *decpt = r2;

   /*
    * we now stick the fractional part in digit by digit regardless
    * of whether there was a non-zero integer part
    */
   while (p <= p1)
   {
      tmpintpart = arg * 10;
      tmpintpart = _modf(tmpintpart, (double *)(&intpart));
      arg = tmpintpart;
      *p++ = (char)((int32_t)intpart + '0');
   }
   if (!eflag && tmp)
   {
      /* precision is too large.   */
      /* return with what we have. */
      buf[NDIG-1] = '0';
      return(buf);
   }

   /*
    * round the number
    */
   p = p1;
   *p1 += 5;
   while (*p1 > '9')
   {
      *p1 = '0';
      if (p1 > buf)
      {
         p1--;
         *p1 += 1;
      }
      else
      {
         *p1 = '1';
         (*decpt)++;
         if (eflag == 0)
         {
            if (p > buf)
            {
               *p = '0';
            }
            p++;
         } /* Endif eflag == 0 */
      }
   }
   *p = '\0';
   return(buf);

}

/*!
 * \brief Make string Not A Number, and INFinite.
 *
 * \param[out] bp          Where to put the number.
 * \param[in]  numstr      'i' for Infinite, 'n' for NAN, 'z' for ZERO.
 * \param[in]  printsign   Should the sign be printed.
 * \param[in]  printprefix Should a prefix be printed.
 * \param[in]  sign        The sign of the number.
 * \param[in]  type        Type of number for upper or lower case.
 */
static void _inf_or_nan(char *bp, char *numstr, char printsign, char printprefix, char sign, char type)
{
    char shift = 'a' - 'A'; /* for lovercase */
    /* Conversion gave 0, infinity, or NAN, use f or F format for output */
     if (sign)
     {
        *bp++ =  '-';
     }
     else if (printsign)
     {
        *bp++ =  '+';
     }
     else if (printprefix)
     {
        *bp++ =  ' ';
     }
    /* for uppercase */
    if (type == 'G' || type == 'E' || type == 'F')
    {
        shift = (char)0;
    }
    /* Infinity */
    if ( *numstr == 'i')
    {
    *bp++ = 'I' + shift;
    *bp++ = 'N' + shift;
    *bp++ = 'F' + shift;
    }
    /* NAN */
    else if (*numstr == 'n')
    {
    *bp++ = 'N' + shift;
    *bp++ = 'A' + shift;
    *bp++ = 'N' + shift;
    }
    *bp = '\0';
}

/*!
 * \brief Convert a number (exponent) to ASCII
 *
 * \param[in] number The number to convert.
 * \param[in] str_ptr Where to store the converted number.
 *
 * \return ASCII number.
 */
int32_t _dtoa( int32_t number, char *str_ptr)
{
   int32_t   frac;
   char     i;
   int32_t   digits;

   if (number < 0)
   {
      number = -number;
   }
   if (number > 99)
   {
      digits = 3;
   }
   else
   {
      digits = 2;
   }
   for (i = 1; i <= digits; i++)
   {
      frac = number % 10;
      number = number / 10;
      str_ptr[digits-i] = (char)(frac + '0');
   }
    return (digits);

}

/*!
 * \brief Converts a double to E format output.
 *
 * \param[out] buf         Where to put the number.
 * \param[in]  arg         The number to convert.
 * \param[in]  fillchar    The fill character to use.
 * \param[in]  printsign   Should the sign be printed.
 * \param[in]  printprefix Should a prefix be printed.
 * \param[in]  altformat   Makes sure that output is printed out as floating.
 * \param[in]  precision   What preicision to use.
 * \param[in]  e_or_E      Use lower or uppercase E.
 *
 * \return Number in E format.
 */
int32_t _dtoe(char *buf, double arg, char *fillchar, char printsign, char printprefix, char altformat, int32_t precision, char e_or_E)
{
   char       sign;
   int32_t     decpt;
   char      *numstr;
   char      *bp;
   char       cvtbuf[NDIG];

   numstr = _cvt(arg, precision + 1, &decpt, &sign, 1, cvtbuf, e_or_E);
   bp = buf;
   if ((*numstr == 'i') || (*numstr == 'n'))
   {
      /* infinity, or NAN */
        _inf_or_nan(bp, numstr, printsign, printprefix, sign, e_or_E);
        *fillchar = ' ';
   }
   else
   {

      if (sign)
      {
         *bp++ =  '-';
      } else if (printsign)
      {
         *bp++ =  '+';
      }
      else if (printprefix)
      {
         *bp++ =  ' ';
      }
      *bp++ = *numstr++;
      if ((*numstr != '\0') || altformat)
      {
         *bp++ = '.';
      }
      while (*numstr != '\0')
      {

         *bp++ = *numstr++;
      }
      *bp++ = e_or_E;
      if (decpt -1 >= 0)
      {
         *bp++ = '+';
      }
      else
      {
         *bp++ = '-';
      }
      /* update bp after putting in exp */
      bp += _dtoa(decpt - 1, bp);
      *bp = '\0';
   }

   return (int)(bp - buf);

}


/*!
 * \brief Converts a double to F format output
 *
 * \param[out] buf         Where to put the number.
 * \param[in]  arg         The number to convert.
 * \param[in]  fillchar    The fill character to use.
 * \param[in]  printsign   Should the sign be printed.
 * \param[in]  printprefix Should a prefix be printed.
 * \param[in]  altformat   Makes sure that output is printed out as floating.
 * \param[in]  precision   What preicision to use.
 * \param[in]  f_or_F      Use lower or uppercase F.
 *
 * \return Number in F format.
 */
int32_t _dtof(char *buf, double arg, char *fillchar, char printsign, char printprefix, char altformat, int32_t precision, char f_or_F)
{
   char       sign;
   int32_t     decpt;
   char      *numstr;
   char      *bp;
   char       cvtbuf[NDIG];

   bp = buf;
   if (precision > NDIG)
   {
      precision = NDIG;
   }
   numstr = _cvt(arg, precision, &decpt, &sign, 0, cvtbuf, f_or_F);

   if ((*numstr == 'i') || (*numstr == 'n'))
   {
        _inf_or_nan(bp, numstr,printsign , printprefix, sign, f_or_F);
        *fillchar = ' ';
   }
   else
   {
      if (sign)
      {
         *bp++ =  '-';
      } else if (printsign)
      {
         *bp++ =  '+';
      }
      else if (printprefix)
      {
         *bp++ =  ' ';
      }
      if (decpt <= 0)
      {
         *bp++ = '0';
         *bp++ = '.';
         while (decpt++ < 0)
         {
            *bp++ = '0';
         }
         while ((*bp++ = *numstr++) != 0)
         {
         }
         --bp; /* bump bp back to point to the null */
      }
      else
      {
         while ((*numstr != '\0') && (decpt-- != 0))
         {
            *bp++ = *numstr++;
         }
         if ((*numstr != '\0') || altformat)
         {
            *bp++ = '.';
         }
         if (*numstr != '\0')
         {
            while ((*bp++ = *numstr++) != 0)
            { ;
            }
            --bp; /* bump bp back to point to the null */
         }
         else
         {
            *bp = '\0';
         }
      }
   }

   return (int)(bp - buf); /* same as strlen(buf) */

}

/*!
 * \brief Converts a double to G format output
 *
 * \param[out] buf         Where to put the number.
 * \param[in]  arg         The number to convert.
 * \param[in]  fillchar    The fill character to use.
 * \param[in]  printsign   Should the sign be printed.
 * \param[in]  printprefix Should a prefix be printed.
 * \param[in]  altformat   Makes sure that output is printed out as floating.
 * \param[in]  precision   What preicision to use.
 * \param[in]  g_or_G      Use lower or uppercase G.
 *
 * \return Number in G format.
 */
int32_t _dtog(char *buf, double arg, char *fillchar, char printsign, char printprefix, char altformat, int32_t precision, char g_or_G)
{
   char          sign;
   int32_t       decpt, i;
   char         *numstr;
   char         *bp;
   char          cvtbuf[NDIG];

   /* First get e or E format */
   g_or_G -= 2;
   numstr = _cvt(arg, precision + 1, &decpt, &sign, 1, cvtbuf, g_or_G);
   bp = buf;

   if ((*numstr == 'i') || (*numstr == 'n'))
   {
        _inf_or_nan(bp, numstr, printsign , printprefix, sign, g_or_G);
        *fillchar = ' ';
   }
   else
   {
      /* Check whether f format is more suitable */
      if ((decpt - 1 >= -4) && (decpt - 1 < precision))
      {
         if (precision > NDIG)
         {
            precision = NDIG;
         }
         numstr = _cvt(arg, precision, &decpt, &sign, 0, cvtbuf, (g_or_G - 2));
         g_or_G += 1;
      }

      /* Remove trailing zeros */
      if (! altformat)
      {
         for (i = 0; numstr[i] != 0; i++)
         { ;
         }
         for (--i; i >= 0; i--)
         {
            if ((numstr[i] != '0') || (i == decpt)) break;
         }
         if ((i >= 0) && (numstr[i] == '0'))
         {
            i--;
         }
         if (i == -1)
         {
            if ((g_or_G == 'e') || (g_or_G == 'E'))
            {
               numstr[0] = '0';
               i = 0;
            }
         }
         else if ((numstr[i] < '0') || (numstr[i] > '9'))
         {
            numstr[++i] = '0';
         }
         numstr[i + 1] = '\0';
      }

      if ((g_or_G == 'e') || (g_or_G == 'E'))
      {
         /* Output in e, E format */
         if (sign)
         {
            *bp++ =  '-';
         }
         else if (printsign)
         {
            *bp++ =  '+';
         }
         else if (printprefix)
         {
            *bp++ =  ' ';
         }
         *bp++ = *numstr++;
         if ((*numstr != '\0') || altformat)
         {
            *bp++ = '.';
         }
         while (*numstr != '\0')
         {
            *bp++ = *numstr++;
         }
         *bp++ = g_or_G;
         if (decpt -1 >= 0)
         {
            *bp++ = '+';
         }
         else
         {
            *bp++ = '-';
         }
         /* update bp after putting in exp */
         bp += _dtoa(decpt - 1, bp);
         *bp = '\0';
      }
      else
      {
         /* Output in f format */
         if (sign)
         {
            *bp++ =  '-';
         }
         else if (printsign)
         {
            *bp++ =  '+';
         }
         else if (printprefix)
         {
            *bp++ =  ' ';
         }
         if (decpt <= 0)
         {
            *bp++ = '0';
            if(*numstr != 0)
            {
                *bp++ = '.';
            }
            while (decpt++ < 0)
            {
               *bp++ = '0';
            }
            while ((*bp++ = *numstr++) != 0)
            {
            }
            --bp; /* bump bp back to point to the null */
         }
         else
         {
            while ((*numstr != '\0') && (decpt-- != 0))
            {
               *bp++ = *numstr++;
            }
            if ((*numstr != '\0') || altformat)
            {
               *bp++ = '.';
            }
            if (*numstr != '\0')
            {
               while ((*bp++ = *numstr++) != 0)
               { ;
               }
               --bp; /* bump bp back to point to the null */
            }
            else
            {
               *bp = '\0';
            }
         }
      }
   }

   return (int)(bp - buf);

}

