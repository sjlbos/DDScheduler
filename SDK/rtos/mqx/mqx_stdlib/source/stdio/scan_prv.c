
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
*
*
*
*END************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "scan_prv.h"

static char scanline_ignore_white_space(char  **, unsigned int *, unsigned int);
static char scanline_format_ignore_white_space(char  **, unsigned int *);
static int32_t scanline_base_convert(unsigned char, unsigned int);
static bool scanline_is_octal_digit(char);
static bool scanline_is_hex_digit(char);

/*!
 * \brief Converts an input line of ASCII characters based upon a provided
 * string format.
 *
 * \param[in] line_ptr The input line of ASCII data.
 * \param[in] format   Format first points to the format string.
 * \param[in] args_ptr The list of parameters.
 *
 * \return Number of input items converted and assigned.
 * \return IO_EOF - When line_ptr is empty string "".
 */
int io_scan_prv(const char *line_ptr, char *format, va_list args_ptr)
{
            char        suppress_field;
            int         c;
            unsigned int  n;
            char        *sptr = NULL;
            int         sign;
            uint32_t    val;
            int         width;
            int         numtype;  /* used to indicate bit size of argument */
            int         number_of_chars;
            unsigned int  temp;
            unsigned int  base;
            void      *tmp_ptr;
#if STD_FP_IO
            double     dnum;
#endif

   if (*line_ptr == '\0')
   {
      return EOF;
   }

   n = 0;
   number_of_chars = 0;
   while ((c = *format++) != 0)
   {

      width = 0;

      /*
       * skip white space in format string, and any in input line
       */
      if ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r') || (c == '\v') || (c == '\f'))
      {
         if (! scanline_format_ignore_white_space((char  **)&format, &temp))
         {
            /*
             * End of format string encountered, scanning is finished.
             */
            return (int)n;
         }

         if (! scanline_ignore_white_space((char  **)&line_ptr , &temp, 0))
         {
            /*
             * End of line string encountered, no more input to scan.
             */
            return (int)n;
         }
         number_of_chars += temp;
         continue;
      }

      if (c != '%')
      {

         /*
          * if format character is not %, then it must match text
          * in input line
          */
         if (c != scanline_ignore_white_space((char  **)&line_ptr , &temp, 0))
         {
            /*
             * Text not matched, stop scanning..
             */
            return (int)n;
         }
         else
         {
            ++line_ptr;
         }
         number_of_chars += temp;

      }
      else
      {
         /*
          * process % format conversion
          */
         suppress_field = 0;
         width = 0;
         numtype = SCAN_MLONG;
         sign = 1;
         val = 0;

         /*
          * Skip whitespace. Check for " %  ", return if found, otherwise
          * get next character.
          */
         if (! scanline_format_ignore_white_space((char  **)&format, &temp))
         {
            return (int)n;
         }
         c = *format;

         /*
          * Check for assignment suppression. if "*" given,
          * suppress assignment
          */
         if (c == '*')
         {
            ++suppress_field;
            c = *(++format);
         }

         /*
          * Skip whitespace. Check for " %  *  ", return if found, otherwise
          * get next character.
          */
         if (! scanline_format_ignore_white_space((char  **)&format, &temp))
         {
            return (int)n;
         }
         c = *format;

         /*
          * Now check for a width field
          */
         width = 0;
         while (('0' <= c) && (c <= '9'))
         {
            width = width * 10 + (int32_t)(c - '0');
            c = *(++format);
         }

         /*
          * Skip whitespace. Check for " %  *  23 ", return if found,
          * otherwise get next character.
          */
         if (! scanline_format_ignore_white_space((char  **)&format, &temp))
         {
            return (int)n;
         }
         c = *format++;

         /*
          * Check to see if c is lower case, if so convert to upper case
          */
         if ((c >= 'a') && (c <= 'z'))
         {
            c -= 32;
         }

         /*
          * Now check to see if c is a type specifier.
          */
         if (c  == 'H')
         {
            numtype = SCAN_WLONG;
            if (! scanline_format_ignore_white_space((char  **)&format, &temp))
            {
               return (int)n;
            }
            c = *format++;
         }
         else if (c == 'L')
         {
            numtype = SCAN_LLONG;
            if (! scanline_format_ignore_white_space((char  **)&format, &temp))
            {
               return (int)n;
            }
            c = *format++;
         }
         else if (c == 'Z')
         {
            numtype = SCAN_BLONG;
            if (! scanline_format_ignore_white_space((char  **)&format, &temp))
            {
               return (int)n;
            }
            c = *format++;
         }
         else if (c == 'M')
         {
            numtype = SCAN_MLONG;
            if (! scanline_format_ignore_white_space((char  **)&format, &temp))
            {
               return (int)n;
            }
            c = *format++;
         }

         /*
          * Check to see if c is lower case, if so convert to upper case
          */
         if ((c >= 'a') && (c <= 'z'))
         {
            c -= 32;
         }

         /*
          * Now check to see if c is a valid format specifier.
          */
         switch (c)
         {

            case 'I':
               c = scanline_ignore_white_space((char  **)&line_ptr, &temp, 0);
               if (*line_ptr == '0')
               {
                  ++number_of_chars;
                  ++line_ptr;
                  if (width)
                  {
                     if (--width <= 0)
                     {
                        goto print_val;
                     }
                  }
                  if ((*line_ptr == 'x') || (*line_ptr == 'X'))
                  {
                     base = 16;
                     ++line_ptr;
                     ++number_of_chars;
                     if (width)
                     {
                        if (--width <= 0)
                        {
                           goto print_val;
                        }
                     }
                  }
                  else if ((*line_ptr == 'b') || (*line_ptr == 'B'))
                  {
                     base = 2;
                     ++line_ptr;
                     ++number_of_chars;
                     if (width)
                     {
                        if (--width <= 0)
                        {
                           goto print_val;
                        }
                     }
                  }
                  else
                  {
                     base = 8;
                     if (! scanline_is_octal_digit(*line_ptr))
                     {
                        goto print_val;
                     }
                  }
               }
               else
               {
                  goto decimal;
               }
               goto doval;

            case 'P':
            case 'X':
               base = 16;
               c = scanline_ignore_white_space((char  **)&line_ptr, &temp, 0);
               if (*line_ptr == '0')
               {
                  ++line_ptr;
                  ++number_of_chars;
                  if (width)
                  {
                     if (--width <= 0)
                     {
                        goto print_val;
                     }
                  }
                  if ((*line_ptr == 'x') || (*line_ptr == 'X'))
                  {
                     ++line_ptr;
                     ++number_of_chars;
                     if (width)
                     {
                        if (--width <= 0)
                        {
                           goto print_val;
                        }
                     }
                  } else if (! scanline_is_hex_digit(*line_ptr))
                  {
                     goto print_val;
                  }
               }
               goto doval;

            case 'O':
               base = 8;
               c = scanline_ignore_white_space((char  **)&line_ptr, &temp, 0);
               if (*line_ptr == '0')
               {
                  ++number_of_chars;
                  ++line_ptr;
                  if (width)
                  {
                     if (--width <= 0)
                     {
                        goto print_val;
                     }
                  }
                  if (! scanline_is_octal_digit(*line_ptr))
                  {
                     goto print_val;
                  }
               }
               goto doval;

            case 'B':
               base = 2;
               c = scanline_ignore_white_space((char  **)&line_ptr, &temp, 0);
               if (*line_ptr == '0')
               {
                  ++line_ptr;
                  ++number_of_chars;
                  if (width)
                  {
                     if (--width <= 0)
                     {
                        goto print_val;
                     }
                  }
                  if ((*line_ptr == 'b') || (*line_ptr == 'B'))
                  {
                     ++line_ptr;
                     ++number_of_chars;
                     if (width)
                     {
                        if (--width <= 0)
                        {
                           goto print_val;
                        }
                     }
                  } else if (! scanline_is_hex_digit(*line_ptr))
                  {
                     goto print_val;
                  }
               }
               goto doval;

            case 'D':
               decimal:
               base = 10;
               temp = 0;
               if (scanline_ignore_white_space((char  **)&line_ptr, &temp, 0) == '-')
               {
                  number_of_chars += temp;
                  sign = -1;
                  ++line_ptr;
                  ++number_of_chars;
                  if (width)
                  {
                     width -= (int32_t)temp;
                     if (width <= 0)
                     {
                        goto print_val;
                     }
                  }
               }
               else
               {
                  number_of_chars += temp;
               }

            case 'U':
               base = 10;
               c = scanline_ignore_white_space((char  **)&line_ptr, &temp, 0);
doval:
               val = 0;
               /* remove spaces if any but don't */
               /* parse passed end of line       */
               c = *line_ptr;
               number_of_chars += temp;
               if (width)
               {
                  width -= temp;
                  if (width <= 0)
                  {
                     break;
                  }
               }
               if (scanline_base_convert((unsigned char)c, base) == SCAN_ERROR)
               {
                  return (int)n;
               }

               while ((c = scanline_base_convert(*line_ptr, base)) != SCAN_ERROR)
               {
                  ++line_ptr;
                  ++number_of_chars;
                  val = val * base + (uint32_t)((unsigned char)c & 0x7F);
                  if (width)
                  {
                     if (--width <= 0)
                     {
                        break;
                     }
                  }
               }

print_val:
               if (! suppress_field)
               {
                  /* assign value using appropriate pointer */
                  val *= sign;
                  tmp_ptr = (void *)va_arg(args_ptr, void *);
                  switch (numtype)
                  {
                     case SCAN_LLONG:
                        *((uint32_t *)tmp_ptr) = val;
                        break;
                     case SCAN_WLONG:
                        *((uint16_t *)tmp_ptr) = (uint16_t)val;
                        break;
                     case SCAN_BLONG:
                        *((unsigned char *)tmp_ptr) = (unsigned char)val;
                        break;
                     case SCAN_MLONG:
                        *((unsigned int *)tmp_ptr) = (unsigned int)val;
                        break;
                     default:
                        break;
                  }
                  ++n;
               }
               break;

            case 'S':
               temp = 0;
               scanline_ignore_white_space((char  **)&line_ptr, &temp, 0);
               number_of_chars += temp;
               if (! suppress_field)
               {
                  sptr = (char *)va_arg(args_ptr, void *);
               }
               if (width)
               {
                  width -= (int32_t)temp;
                  if (width <= 0)
                  {
                     goto string_done;
                  }
               }

               while ((c = *line_ptr) != 0)
               {
                  ++line_ptr;
                  ++number_of_chars;
                  if (c == *format)
                  {
                     ++format;
                     break;
                  }

                  if (! suppress_field)
                  {
                     *sptr++ = (char)c;
                  }
                  if (width)
                  {
                     if (--width <= 0)
                     {
                        break;
                     }
                  }

               }

string_done:
               if (! suppress_field)
               {
                  ++n;
                  *sptr = '\0';
               }
               break;

            case 'C':
               if (width == 0)
               {
                  width = 1;
               }

               if (! suppress_field)
               {
                  sptr = (char *)va_arg(args_ptr, void *);
               }
               while (width-- > 0)
               {
                  if (! suppress_field)
                  {
                     *sptr++ = (unsigned char)*line_ptr;
                  }

                  ++line_ptr;
                  ++number_of_chars;
               }

               if (! suppress_field)
               {
                  ++n;
               }
               break;

            case 'N':
               if (! suppress_field)
               {
                  tmp_ptr = va_arg(args_ptr, void *);
                  *(int *)(tmp_ptr) = (int)number_of_chars;
               }
               break;

#if STD_FP_IO
         case 'G':
         case 'F':
         case 'E':
            dnum = strtod((char *)line_ptr, (char  **)&tmp_ptr);

            if ((dnum == HUGE_VAL) || (dnum == -HUGE_VAL))
            {
               return (int)n;
            }

            line_ptr = tmp_ptr;
            tmp_ptr = (void *)va_arg(args_ptr, void *);
            if (SCAN_LLONG == numtype)
            {
               *((double *)tmp_ptr) = dnum;
            }
            else
            {
               *((float *)tmp_ptr) = (float)dnum;
            }
            ++n;
         break;
#endif

            default:
               return (int)n;

         }

      }

      /* if end of input string, return */

   }
   return (int)n;

}


/*!
 * \brief Scanline function which ignores white spaces.
 *
 * \param[in,out] s_ptr The addres of the string pointer to update.
 * \param[out]    count The number of characters skipped.
 * \param[in]     width Maximum number of white spaces to skip.
 *
 * \return String without white spaces.
 */
static char scanline_ignore_white_space(char  **s_ptr, unsigned int *count, unsigned int  width)
{
   register char c;

   c = **s_ptr;
   *count = 0;
   while ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r') || (c == '\v') || (c == '\f'))
   {
      c = *(++*s_ptr);
      (*count)++;
      if (width)
      {
         if (--width == 0)
         {
            return(c);
         }
      }
   }
   return (c);

}

/*!
 * \brief Scanline function which ignores white spaces.
 *
 * \param[in,out] s_ptr The address of the string pointer.
 * \param[out]    count The number of characters skipped.
 *
 * \return String without white spaces.
 */
static char scanline_format_ignore_white_space(char  **s_ptr, unsigned int *count)
{
   register char c;

   *count = 0;
   c = **s_ptr;
   while ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r') || (c == '\v') || (c == '\f'))
   {
      c = *(++*s_ptr);
      (*count)++;
   }
   return (c);

}

/*!
 * \brief Converts input string.
 *
 * \param[in] c    The character to convert.
 * \param[in] base The base to convert the character in.
 *
 * \return Input string.
 * \return SCAN_ERROR (Failure.)
 */
static int32_t scanline_base_convert(unsigned char c, unsigned int base)
{

   if (c >= 'a' && c <= 'z')
   {
      /* upper case c */
      c -= 32;
   }

   if (c >= 'A' && c <= 'Z')
   {
      /* reduce hex digit */
      c -= 55;
   } else if ((c >= '0') && (c <= '9'))
   {
      /* reduce decimal digit */
      c -= 0x30;
   }
   else
   {
      return SCAN_ERROR;
   }
   if ((unsigned int)c  > (base-1))
   {
      return SCAN_ERROR;
   }
   else
   {
      return c;
   }

}

/*!
 * \brief Determines whether input character is an octal number.
 *
 * \param[in] c The character to check.
 *
 * \return true (Input is an octal number.), false (Input is not an octal number.)
 */
static bool scanline_is_octal_digit(char c)
{

   if ((c >= '0') && (c <= '7'))
   {   /* An octal digit */
      return true;
   }
   else
   {
      return false;
   }

}

/*!
 * \brief Determines whether input character is a hexadecimal number.
 *
 * \param[in] c The character to check.
 *
 * \return true (Input is a hexadecimal number.), false (Input is not a
 * hexadecimal number.)
 */
static bool scanline_is_hex_digit(char c)
{
   if (((c >= '0') && (c <= '9')) ||
      ((c >= 'a') && (c <= 'f')) ||
      ((c >= 'A') && (c <= 'F')))
   {   /* A hex digit */
      return true;
   }
   else
   {
      return false;
   }
}
