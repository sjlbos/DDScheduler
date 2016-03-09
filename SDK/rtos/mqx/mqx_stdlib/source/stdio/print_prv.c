
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
*   output.
*   $Header:io_dopr.c, 7, 2/23/2004 2:36:23 PM, $
*   $NoKeywords$
*
*
*END************************************************************************/

#include "print_prv.h"

static void _doprint_prt(int32_t, unsigned char *, int32_t, bool);
static void _doprint_prt64(int64_t, unsigned char *, int32_t, bool);

#if STD_FP_IO
#include "fp_prv.h"
//int _dtoe(char *, double arg, char *, char, char, char, int32_t, char);
//int _dtof(char *, double arg, char *, char, char, char, int32_t, char);
//int _dtog(char *, double arg, char *, char, char, char, int32_t, char);
#endif

// Keil: suppress ellipsis warning in va_arg usage below
#if defined(__CC_ARM)
#pragma diag_suppress 1256
#endif


// Utility function prototypes
static int _mputc(FILE *, PUTCHAR_FUNC func_ptr, int max_count, char c, int count);
static int _putstr(FILE *, PUTCHAR_FUNC func_ptr, int max_count, char *str);

/*!
 * \brief Gets given character count times.
 *
 * \note I/O is performed by calling given function pointer using following
 * (*func_ptr)(c,farg);
 *
 * \param[in] farg      Argument to func_ptr
 * \param[in] func_ptr  Function to put character out.
 * \param[in] max_count Maximum character count for snprintfand vsnprintf.
 * Default value is 0 (unlimited size).
 * \param[in] c         The character to put out.
 * \param[in] count     Number of characters to be printed out.
 *
 * \return Character count.
 */
static int _mputc(FILE *farg, PUTCHAR_FUNC func_ptr, int max_count, char c, int count)
{
    int32_t   cnt = 0;

    if (count < 0)
    {
        count = 0;
    }

    for (;(cnt < count); cnt++)
    {
        if (max_count > (cnt + 1))
        {
            if (c != func_ptr((int) c, farg))
            {
                break;
            }
        }
    }

    return cnt;
}

/*!
 * \brief Gets given string.
 *
 * \note I/O is performed by calling given function pointer using following
 * (*func_ptr)(c,farg);
 *
 * \param[in] farg      Argument to func_ptr.
 * \param[in] func_ptr  Function to put character out.
 * \param[in] max_count Maximum character count for snprintfand vsnprintf.
 * Default value is 0 (unlimited size).
 * \param[in] str       The string to put out.
 *
 * \return String.
 */
static int _putstr(FILE *farg, PUTCHAR_FUNC func_ptr, int max_count, char *str)
{
    int res = 0;
    bool compare = true;

    for ( ; *str && compare; res++, str++)
    {
        if (max_count > (res + 1))
        {
            compare = ((int) *str == func_ptr((int) *str, farg));
        }
        else
        {
            compare = true;
        }
    }

    if (*str)
    {
        res--;
    }
    return res;
}

/*!
 * \brief This function outputs its parameters according to a formatted string.
 *
 * \note I/O is performed by calling given function pointer using following
 * (*func_ptr)(c,farg);
 *
 * \param[in] farg      Argument to func_ptr.
 * \param[in] func_ptr  Function to put character out.
 * \param[in] max_count Maximum character count for snprintfand vsnprintf.
 * Default value is 0 (unlimited size).
 * \param[in] fmt_ptr   Format string for printf.
 * \param[in] args_ptr  Arguments to printf.
 *
 * \return Number of characters
 * \return EOF (End Of File found.)
 */
int _io_doprint(FILE *farg, PUTCHAR_FUNC func_ptr, int max_count, char *fmt_ptr, va_list args_ptr)
{
   unsigned char    string[32];   /* The string str points to this output   */
   char     c;
   int32_t  i;
   int64_t  i64;
   int      f;            /* the format character (comes after %)   */
   unsigned char      *str_ptr;  /* running pointer in string              */
   /*    from number conversion              */
   int      length;       /* Length of string "str"                 */
   char     fill;         /* Fill character ( ' ' or '0' )          */
   int      leftjust;     /* 0 = right-justified, else left-just.   */
   int      prec,width;   /* Field specifications % width . precision */
   int      leading;      /* No. of leading/trailing fill chars.    */
   unsigned char    sign;         /* Set to '-' for negative decimals       */
   int      digit1;       /* Offset to add to first numeric digit.  */
   char     use_caps;     /* Hex output to use capital letters.     */
   char     use_signs;    /* Always output a sign for %d            */
   char     use_space;    /* If positive and not sign, print space for %d */
   char     use_prep;     /* Prepend 0 for octal, 0x , 0X for hex   */
   char     use_int_prec; /* Precision usage with d, i, o, u, x, X  */
   char     zero_value;   /* indicates 0 output value */
   char     zero_char;    /* indicates 0x00 char  */
   char     prepend;      /* What prepend type 3 octal 2 0x, 1 0X   */
   int      arg_length;   /* optional argument length               */
   char     done;
   char     is_integer;
   int      number_of_characters;  /* # chars printed out */
#if STD_FP_IO
   char     prec_set;
   double   fnumber;
   char     fstring[324]; /* Floating point output string           */
#endif
   uint32_t *temp_ptr;
   int    func_result = 0;

   enum types{ HALF, HALFHALF, LONG, LONGLONG, LONGDOUBLE };

   number_of_characters = 0;
   /* if max_count == -1 then maximal positive number is stored in max_count */
   /* it is not gnu standard because max count shoud be unsigned */
   if (max_count == -1)
   {
        max_count = INT32_MAX;
   }
   for ( ;; )
   {
      /* Echo characters until '%' or end of fmt_ptr string */
      while ((c = *fmt_ptr++ ) != '%')
      {
         if (c == '\0')
         {
            if (func_result == EOF)
            {
               return(EOF);
            }
            else
            {
               return number_of_characters;
            }
         }
         number_of_characters++;
         if (max_count > number_of_characters)
         {
            func_result = (*func_ptr)((int)c, farg);
         }
      }

      /* Echo "...%%..." as '%' */
      if (  *fmt_ptr == '%'  )
      {
         number_of_characters++;
         if (max_count > number_of_characters)
         {
             func_result = (*func_ptr)((int)*fmt_ptr++, farg);
         }
            continue;
      }

      /* Set default flags */
      fill         = ' ';
      sign         = '\0';
      arg_length   = sizeof(int32_t);
      leftjust     = 0;
      use_caps     = 0;
      use_space    = 0;
      use_prep     = 0;
      use_signs    = 0;
      use_int_prec = 0;
      prepend      = 0;
      zero_value   = 0;
      zero_char    = 0;
#if STD_FP_IO
      prec_set   = 0;
#endif

      /* Collect Valid Flags */
      done = false;
      while ( *fmt_ptr && ! done )
      {
         switch ( *fmt_ptr )
         {

            case '-':
               /* Check for "%-..." EQ Left-justified output */
               leftjust = 1;
               fmt_ptr++;
               break;

            case '0':
               fill = '0';
               fmt_ptr++;
               break;

            case '+':
               use_signs = 1;
               fmt_ptr++;
               break;

            case ' ':
               use_space = 1;
               fmt_ptr++;
               break;

            case '#':
               use_prep = 1;
               fmt_ptr++;
               break;

            default:
               done = true;
            break;
         }
      }

      /* Allow for minimum field width specifier for %d,u,x,o,c,s */
      /* Also allow %* for variable width ( %0* as well)           */
      width = 0;
      if (  *fmt_ptr == '*'  )
      {
         width = va_arg(args_ptr, uint32_t);
         ++fmt_ptr;
      }
      else
      {
         while (  ('0' <= *fmt_ptr) && (*fmt_ptr <= '9')  )
         {
            width = width * 10 + *fmt_ptr++ - '0';
         }
         /* Allow for maximum string width for %s */
      }

      prec = -1;
      if (  *fmt_ptr == '.'  )
      {
         prec = 0;
         if (  *(++fmt_ptr) == '*'  )
         {
            prec = va_arg(args_ptr, uint32_t);
#if STD_FP_IO
            prec_set = 1;
#endif
            ++fmt_ptr;
         }
         else
         {
            while (  ('0' <= *fmt_ptr) && (*fmt_ptr <= '9')  )
            {
               prec = prec * 10 + *fmt_ptr++ - '0';
#if STD_FP_IO
               prec_set = 1;
#endif
            }
         }
      }

      str_ptr = string;
      if (  (f = *fmt_ptr++) == '\0'  )
      {
         number_of_characters++;
         if (max_count > number_of_characters)
         {
             func_result = (*func_ptr)((int)'%', farg);
         }
         if (func_result == EOF)
         {
            return(EOF);
         }
         else
         {
            return number_of_characters;
         }
      }

        switch (f)
        {
        case 'h' :
            arg_length = HALF;
            f = *fmt_ptr++;
            if (f == 'h')
            {
                arg_length = HALFHALF;
                f = *fmt_ptr++;
            }
            break;
        case 'l' :
            arg_length = LONG;
            f = *fmt_ptr++;
            if (f == 'l')
            {
                arg_length = LONGLONG;
                f = *fmt_ptr++;
            }
            break;
        case 'L' :
            arg_length = LONGDOUBLE;
            f = *fmt_ptr++;
            break;
        default:
            break;
        }

      use_caps = false;
      is_integer = false;
      length = 0;
      switch (  f  )
      {
         case 'c':
         case 'C':
            string[0] = (char)va_arg(args_ptr, int);
            if (string[0] == 0)
            {
               length += 1;
               zero_char = 1;
            }
            string[1] = '\0';
            prec = 0;
            fill = ' ';
            use_prep = 0;
            break;

         case 's':
         case 'S':
            str_ptr = (unsigned char *)va_arg(args_ptr, void *);
            fill = ' ';
            use_prep = 0;
            break;

         case 'i':
         case 'I':
         case 'd':
         case 'D':
            is_integer = true;
         case 'u':
         case 'U':
            if (f == 'u' || f == 'U')
            {
            }
            if (prec > 0)
            {
                use_int_prec = 1;
            }
            if (is_integer)
            {
               if (arg_length == LONGLONG)
               {
                  i64 = va_arg(args_ptr, int64_t);
                  zero_value = i64 == 0L;
                  if (i64 < 0)
                  {
                     sign = '-';
                     i64 = -i64;
                  }
                  else if ( use_signs )
                  {  /* Always a sign */
                     sign = '+';
                  }
                  else if ( use_space )
                  {  /* use spaces */
                     sign = ' ';
                  }
               }
               else
               {
                  if (arg_length == HALF)
                  {
                     i = (int16_t) va_arg(args_ptr,  signed int);
                  }
                  else if (arg_length == HALFHALF)
                  {
                     i = (int8_t) va_arg(args_ptr,  signed int);
                  }
                  else if (arg_length == LONG)
                  {
                     i = va_arg(args_ptr, int32_t);
                  }
                  else
                  {
                     i = va_arg(args_ptr, int);
                  }
                  zero_value = i == 0;
                  if (i < 0)
                  {
                     sign = '-';
                     i = -i;
                  }
                  else if ( use_signs )
                  {  /* Always a sign */
                     sign = '+';
                  }
                  else if ( use_space )
                  {  /* use spaces */
                     sign = ' ';
                  }
               }
            }
            else
            {
               if (arg_length == HALF)
               {
                  i = (uint16_t) va_arg(args_ptr, unsigned int);
                  zero_value = i == 0;
               }
               else if (arg_length == HALFHALF)
               {
                  i = (uint8_t) va_arg(args_ptr, unsigned int);
                  zero_value = i == 0;
               }
               else if (arg_length == LONG)
               {
                  i = va_arg(args_ptr, uint32_t);
                  zero_value = i == 0;
               }
               else if (arg_length == LONGLONG)
               {
                  i64 = va_arg(args_ptr, uint64_t);
                  zero_value = i64 == 0L;
               }
               else
               {
                  i = va_arg(args_ptr, unsigned int);
                  zero_value = i == 0;
               }
            }
            digit1 = '\0';
            /* "negative" longs in unsigned format       */
            /* can't be computed with long division      */
            /* convert *args_ptr to "positive", digit1   */
            /* = how much to add back afterwards         */
            if (arg_length == LONGLONG)
            {
               while (i64 < 0)
               {
                  i64 -= _MQX_IO_DIVISION_ADJUST_64;
                  ++digit1;
               }
               _doprint_prt64(i64, str_ptr + 1, PRINT_DECIMAL_64, (bool)use_caps);
               str_ptr[1] += digit1;
               if (str_ptr[1] > '9')
               {
                  str_ptr[1] -= 10;
                  str_ptr[0] = '1';
               }
               else
               {
                  str_ptr += 1;
               }
            }
            else
            {
               while (i < 0)
               {
                  i -= _MQX_IO_DIVISION_ADJUST;
                  ++digit1;
               }
               _doprint_prt(i, str_ptr, PRINT_DECIMAL, (bool)use_caps);
               str_ptr[0] += digit1;
            }
            use_prep = 0;
            break;

         case 'o':
         case 'O':
            if (prec > 0)
            {
               use_int_prec = 1;
            }
            if (arg_length == LONGLONG)
            {
               i64 = va_arg(args_ptr, uint64_t);
               zero_value = i64 == 0L;
               _doprint_prt64(i64, str_ptr, PRINT_OCTAL_64, (bool)use_caps);
            }
            else
            {
               if (arg_length == HALF)
               {
                  i = (uint16_t) va_arg(args_ptr, unsigned int);
               }
               else if (arg_length == HALFHALF)
               {
                  i = (uint8_t) va_arg(args_ptr, unsigned int);
               }
               else if (arg_length == LONG)
               {
                  i = va_arg(args_ptr, uint32_t);
               }
               else
               {
                  i = va_arg(args_ptr, unsigned int);
               }
               zero_value = i == 0;
               _doprint_prt(i, str_ptr, PRINT_OCTAL, (bool)use_caps);
            }
            prepend = 3;
            break;

         case 'P':
            use_caps = true;
         case 'p':
            leading = 0;
            temp_ptr = (uint32_t *) va_arg(args_ptr, void *);
            _doprint_prt((uint32_t)temp_ptr, str_ptr, PRINT_ADDRESS,
               (bool)use_caps);
         break;

         case 'X':
            use_caps = true;
            prepend  = 1;
         case 'x':
            if (prec > 0)
            {
                use_int_prec = 1;
            }
            prepend++;
            if (arg_length == LONGLONG)
            {
               i64 = va_arg(args_ptr, uint64_t);
               zero_value = i64 == 0L;
               _doprint_prt64(i64, str_ptr, PRINT_HEX_64, (bool)use_caps);
            }
            else
            {
               if (arg_length == HALF)
               {
                  i = (uint16_t) va_arg(args_ptr, unsigned int);
               }
               else if (arg_length == HALFHALF)
               {
                  i = (uint8_t) va_arg(args_ptr, unsigned int);
               }
               else if (arg_length == LONG)
               {
                  i = va_arg(args_ptr, uint32_t);
               }
               else
               {
                  i = va_arg(args_ptr, unsigned int);
               }
               zero_value = i == 0;
               _doprint_prt(i, str_ptr, PRINT_HEX, (bool)use_caps);
            }
            break;

         case 'n':   /* Defaults to size of number on stack */
         case 'N':   /* Defaults to size of number on stack */
            str_ptr = (unsigned char *)va_arg(args_ptr, void *);
            *(int *)(str_ptr) = (int32_t)number_of_characters;
            continue;

#if STD_FP_IO

         case 'g':
         case 'G':

             if (! prec_set)
             {
                prec = 6;
             }
             if (arg_length == LONGDOUBLE)
             {
                fnumber = va_arg(args_ptr, long double);
             }
             else
             {
                fnumber = va_arg(args_ptr, double);
             }
             i = _dtog(fstring, fnumber, &fill, use_signs, use_space, use_prep,
                (prec - 1), (char)f);
             str_ptr = (unsigned char *)fstring;
             if (*str_ptr == '+')
             {
                sign = '+';
                str_ptr++;
             }
             else if (*str_ptr == '-')
             {
                sign = '-';
                str_ptr++;
             }
             use_prep = 0;
             prec = 0;
          break;

         case 'f':
         case 'F':
            if (! prec_set)
            {
               prec = 6;
            }
            if (prec == 0)
            {
               prec = 1;
            }
            if (arg_length == LONGDOUBLE)
            {
               fnumber = va_arg(args_ptr, long double);
            }
            else
            {
               fnumber = va_arg(args_ptr, double);
            }
            i = _dtof(fstring, fnumber, &fill, use_signs, use_space, use_prep,
               prec, (char)f);
            str_ptr = (unsigned char *)fstring;
            if (*str_ptr == '+')
            {
               sign = '+';
               str_ptr++;
            }
            else if (*str_ptr == '-')
            {
               sign = '-';
               str_ptr++;
            }
            use_prep = 0;
            prec = 0;
         break;

         case 'e':
         case 'E':
            if (! prec_set)
            {
               prec = 6;
            }
            if (arg_length == LONGDOUBLE)
            {
               fnumber = va_arg(args_ptr, long double);
            }
            else
            {
               fnumber = va_arg(args_ptr, double);
            }
            i = _dtoe(fstring, fnumber, &fill, use_signs, use_space, use_prep,
               prec, (char)f);
            str_ptr = (unsigned char *)fstring;
            if (*str_ptr == '+')
            {
               sign = '+';
               str_ptr++;
            }
            else if (*str_ptr == '-')
            {
               sign = '-';
               str_ptr++;
            }
            use_prep = 0;
            prec = 0;
         break;
#endif

         default:
            number_of_characters++;
            if (max_count > number_of_characters)
            {
                func_result = (*func_ptr)((int)f, farg);
            }
            if (func_result == EOF)
            {
               return(EOF);
            }
            else
            {
               return number_of_characters;
            }
      } /* End Switch */

      for (; str_ptr[length] != '\0'; length++ )
      {
         ;
      }

      if ( (width < 0) )
      {
         width = 0;
      }
      if ( (prec < 0) )
      {
         prec = 0;
      }
      else
      {
         if (('s' == f) || ('S' == f))
         {
            if (length > prec)
            {
               length = prec;
            }
         }
      }
      leading = 0;

      if ( (prec != 0) || (width != 0) )
      {
         /*
          * prec = precision
          *    For a string, the maximum number of characters to be printed
          *    from the string.
          *    for f,e,E the number of digits after the decimal point
          *    for g,G the number of significant digits
          *    for integer, the minimum number of digits to be printed
          *       leading 0's will precede
          */
         if ( !use_int_prec && (prec != 0) )
         {
            if ( length > prec )
            {
               if ( str_ptr == string )
               {
                  /* this was a numeric input */
                  /* thus truncate to the left */
                  str_ptr += (length - prec);
               }
               length = prec;
            }
         }

         /* width = minimum field width
          *    argument will be printed in a field at least this wide,
          *    and wider if necessary.  If argument has fewer characters
          *    than the field width, is will be padded on left or right
          *    to make up the width.  Normally pad ' ' unless 0 specified.
          */
         if (use_int_prec)
         {
            if (width > prec)
            {
               leading = (prec > length) ? width - prec : width - length;
               if ( !zero_value && use_prep && (prepend == 3) && (prec > length))
               {
                  leading++; /* correction for octal because one prepend 0 is pad as well as prepend character */
               }
            }
            else
            {
               leading = 0;
            }
         }
         else if ( width != 0 )
         {
            leading = width - length;
         }

         if ( sign )
         {
            --leading;
         }
         if ( use_prep && !zero_value )
         {
            --leading;
            if ( prepend < 3 )
            { /* HEX */
               --leading;
            }
         }

      }

      if (leading < 0)
      {
          leading = 0;
      }

      if ( leftjust == 0 )
      { /* main flow for right justification */
         if (use_int_prec)
         {
            /* print out leading spaces first */
            number_of_characters += _mputc(farg, func_ptr, max_count - number_of_characters, ' ', leading);

            /* print out sign if there's any */
            if ( ((sign == '+') || (sign == '-') || (sign == ' ')) )
            { /* Leading '-', '+', ' ' */
               number_of_characters += _mputc(farg, func_ptr, max_count - number_of_characters, sign, 1);
            }

            /* handle leading '0', '0x' or '0X' */
            if ( use_prep && !zero_value )
            {
               switch (prepend)
               {
               case 1:
                  number_of_characters += _putstr(farg, func_ptr, max_count - number_of_characters, "0x");
                  break;
               case 2:
                  number_of_characters += _putstr(farg, func_ptr, max_count - number_of_characters, "0X");
                  break;
               default:
                  if (!((prec - length) > 0))
                  {
                     number_of_characters += _mputc(farg, func_ptr, max_count - number_of_characters, '0', 1);
                  }
               }
            }

            /* print out leading 0's given by precision */
            number_of_characters += _mputc(farg, func_ptr, max_count - number_of_characters, '0', prec - length);
         }
         else
         { /* right justification without precision for integer types */
            if (fill == '0')
            { /* flow for leading zeroes */
               /* print out sign if there's any */
               if ((sign == '+') || (sign == '-') || (sign == ' '))
               { /* Leading '-', '+', ' ' */
                  number_of_characters += _mputc(farg, func_ptr, max_count - number_of_characters, sign, 1);
               }

               /* handle leading '0', '0x' or '0X' */
               if ( use_prep && !zero_value )
               {
                  switch (prepend)
                  {
                  case 1:
                     number_of_characters += _putstr(farg, func_ptr, max_count - number_of_characters, "0x");
                     break;
                  case 2:
                     number_of_characters += _putstr(farg, func_ptr, max_count - number_of_characters, "0X");
                     break;
                  default:
                     number_of_characters += _mputc(farg, func_ptr, max_count - number_of_characters, '0', 1);
                  }
               }

               /* print out leading 0's */
               number_of_characters += _mputc(farg, func_ptr, max_count - number_of_characters, fill, leading);
            }
            else
            { // flow for leading spaces
               /* print out leading spaces */
               number_of_characters += _mputc(farg, func_ptr, max_count - number_of_characters, fill, leading);

               /* print out sign if there's any */
               if ((sign == '+') || (sign == '-') || (sign == ' '))
               { /* Leading '-', '+', ' ' */
                  number_of_characters += _mputc(farg, func_ptr, max_count - number_of_characters, sign, 1);
               }

               /* handle leading '0', '0x' or '0X' */
               if ( use_prep && !zero_value )
               {
                  switch (prepend)
                  {
                  case 1:
                     number_of_characters += _putstr(farg, func_ptr, max_count - number_of_characters, "0x");
                     break;
                  case 2:
                     number_of_characters += _putstr(farg, func_ptr, max_count - number_of_characters, "0X");
                     break;
                  default:
                      number_of_characters += _mputc(farg, func_ptr, max_count - number_of_characters, '0', 1);
                  }
               }
            }
         }
      }
      else
      { /* left justification has different flow */
         /* print out sign if there's any */
         if ( ((sign == '+') || (sign == '-') || (sign == ' ')) )
         { /* Leading '-', '+', ' ' */
            number_of_characters += _mputc(farg, func_ptr, max_count - number_of_characters, sign, 1);
         }

         /* handle leading '0', '0x' or '0X' */
         if ( use_prep && !zero_value )
         {
            switch (prepend)
            {
            case 1:
               number_of_characters += _putstr(farg, func_ptr, max_count - number_of_characters, "0x");
               break;
            case 2:
               number_of_characters += _putstr(farg, func_ptr, max_count - number_of_characters, "0X");
               break;
            default:
               if (!use_int_prec || ((prec - length) < 0))
               {
                   number_of_characters += _mputc(farg, func_ptr, max_count - number_of_characters, '0', 1);
               }
            }
         }

         if (use_int_prec)
         {
            /* print out leading 0's given by precision */
            number_of_characters += _mputc(farg, func_ptr, max_count - number_of_characters, '0', prec - length);
         }
      }

      /* print out actual characters */
      length++;
      if (length > max_count - number_of_characters)
      {
      	 length = max_count - number_of_characters;
      }

      if(zero_char)
      {
         number_of_characters += _mputc(farg, func_ptr, length, *str_ptr, 1);
      }
      else
      {
         number_of_characters += _putstr(farg, func_ptr, length, (char *) str_ptr);
      }

      if (leftjust != 0)
      { /* Print trailing pad characters */
         /* print out leading 0's */
         number_of_characters += _mputc(farg, func_ptr, max_count - number_of_characters, ' ', leading);
      }

   }
}


static const char upper_hex_string[] = "0123456789ABCDEF";
static const char lower_hex_string[] = "0123456789abcdef";
/*!
 * \brief Converts a number into a printable string.
 *
 * \param[in]     num      The number to convert.
 * \param[in,out] str_ptr  The string to generate.
 * \param[in]     type     What base to use.
 * \param[in]     use_caps Specifies whether to use CAPITALS in hex, or not.
 */
static void _doprint_prt(int32_t num, unsigned char *str_ptr, int32_t type, bool use_caps)
{
   int32_t i;
   char     temp[16];

   temp[0] = '\0';
   if (type == PRINT_OCTAL)
   {
      for (i = 1; i <= type; i++)
      {
         temp[i] = (char)((num & 0x7) + '0');
         num >>= 3;
      }
      temp[PRINT_OCTAL] &= '3';
   }
   else if (type == PRINT_DECIMAL)
   {
      for (i = 1; i <= type; i++)
      {
         temp[i] = (char)(num % 10 + '0');
         num /= 10;
      }
   }
   else
   {
      for (i = 1; i <= type; i++)
      {
         temp[i] = use_caps ? upper_hex_string[num & 0xF] : lower_hex_string[num & 0xF];
         num >>= 4;
      }
   }
   for (i = type; temp[i] == '0'; i--)
   {
      ;
   }
   if (i == 0)
   {
      i++;
   }
   while (i >= 0)
   {
      *str_ptr++ = temp[i--];
   }

}

/*!
 * \brief Converts a 64bit number into a printable string.
 *
 * \param[in]     num      The number to convert.
 * \param[in,out] str_ptr  The string to generate.
 * \param[in]     type     What base to use.
 * \param[in]     use_caps Specifies whether to use CAPITALS in hex, or not.
 */
static void _doprint_prt64
   (
      int64_t         num,
      unsigned char     *str_ptr,
      int32_t         type,
      bool        use_caps
   )
{
   int32_t i;
   char     temp[32];

   temp[0] = '\0';
   if (type == PRINT_OCTAL_64)
   {
      for (i = 1; i <= type; i++)
      {
         temp[i] = (char)((num & 0x7) + '0');
         num >>= 3;
      }
      temp[PRINT_OCTAL_64] &= '1';
   }
   else if (type == PRINT_DECIMAL_64)
   {
      for (i = 1; i <= type; i++)
      {
         temp[i] = (char)(num % 10 + '0');
         num /= 10;
      }
   }
   else
   {
      for (i = 1; i <= type; i++)
      {
         temp[i] = use_caps ? upper_hex_string[num & 0xF] : lower_hex_string[num & 0xF];
         num >>= 4;
      }
   }
   for (i = type; temp[i] == '0'; i--)
   {
      ;
   }
   if (i == 0)
   {
      i++;
   }
   while (i >= 0)
   {
      *str_ptr++ = temp[i--];
   }
}

/*!
 * \brief Erites the character into the string located by the string pointer and
 * updates the string pointer.
 *
 * \param[in]      c                The character to put into the string.
 * \param[in, out] input_string This is an updated pointer to a string pointer.
 *
 * \return Character written into string.
 */
int _io_sputc(int c, FILE * input_string)
{
   char **string_ptr = (char  **)((void *)input_string);

   *(*string_ptr)++ = (char)c;
   return c;

}
