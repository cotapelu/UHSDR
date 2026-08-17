//
// This file is part of the µOS++ III distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

#if defined(TRACE)

#include <stdarg.h>
#include "diag/Trace.h"
#include "string.h"

#ifndef OS_INTEGER_TRACE_PRINTF_TMP_ARRAY_SIZE
#define OS_INTEGER_TRACE_PRINTF_TMP_ARRAY_SIZE (128)
#endif

// Minimal vsnprintf without newlib dependency.
// Supports: %d, %u, %x, %X, %s, %c, %%
static int
trace_vsnprintf(char* buf, size_t bufsz, const char* format, va_list ap)
{
    size_t i = 0;
    for (const char* p = format; *p != '\0' && i + 1 < bufsz; p++)
    {
        if (*p != '%')
        {
            buf[i++] = *p;
            continue;
        }

        const char* p2 = p + 1;
        if (*p2 == '%')
        {
            buf[i++] = '%';
            p = p2;
            continue;
        }

        if (*p2 == 's')
        {
            const char* s = va_arg(ap, const char*);
            if (s == NULL) s = "(null)";
            while (*s != '\0' && i + 1 < bufsz)
            {
                buf[i++] = *s++;
            }
            p = p2;
            continue;
        }

        if (*p2 == 'c')
        {
            buf[i++] = (char)va_arg(ap, int);
            p = p2;
            continue;
        }

        if (*p2 == 'd' || *p2 == 'u')
        {
            char tmp[12]; // enough for 32-bit signed/unsigned
            size_t ti = 0;
            uint32_t val = (uint32_t)va_arg(ap, int);
            if (*p2 == 'd' && (int)val < 0)
            {
                buf[i++] = '-';
                val = (uint32)(-(int)val);
            }
            // convert to decimal
            if (val == 0)
            {
                tmp[ti++] = '0';
            }
            else
            {
                while (val != 0 && ti < sizeof(tmp) - 1)
                {
                    tmp[ti++] = '0' + (val % 10);
                    val /= 10;
                }
            }
            while (ti > 0 && i + 1 < bufsz)
            {
                buf[i++] = tmp[--ti];
            }
            p = p2;
            continue;
        }

        if (*p2 == 'x' || *p2 == 'X')
        {
            char tmp[9];
            size_t ti = 0;
            uint32_t val = (uint32_t)va_arg(ap, int);
            while (ti < 8 && (val != 0 || ti == 0))
            {
                uint8_t nib = (uint8_t)(val & 0xF);
                tmp[ti++] = (nib < 10) ? ('0' + nib) : (*p2 == 'x' ? 'a' : 'A' + (nib - 10));
                val >>= 4;
            }
            while (ti > 0 && i + 1 < bufsz)
            {
                buf[i++] = tmp[--ti];
            }
            p = p2;
            continue;
        }

        // Unsupported specifier: copy literally
        buf[i++] = *p;
    }

    buf[i] = '\0';
    return (int)i;
}

// ----------------------------------------------------------------------------

int
trace_printf(const char* format, ...)
{
  int ret;
  va_list ap;

  va_start (ap, format);

  static char buf[OS_INTEGER_TRACE_PRINTF_TMP_ARRAY_SIZE];

  // Print to the local buffer
  ret = trace_vsnprintf(buf, sizeof(buf), format, ap);
  if (ret > 0)
    {
      // Transfer the buffer to the device
      ret = trace_write (buf, (size_t)ret);
    }

  va_end (ap);
  return ret;
}

int
trace_puts(const char *s)
{
  trace_write(s, strlen(s));
  return trace_write("\n", 1);
}

int
trace_putchar(int c)
{
  trace_write((const char*)&c, 1);
  return c;
}

void
trace_dump_args(int argc, char* argv[])
{
  trace_printf("main(argc=%d, argv=[", argc);
  for (int i = 0; i < argc; ++i)
    {
      if (i != 0)
        {
          trace_printf(", ");
        }
      trace_printf("\"%s\"", argv[i]);
    }
  trace_printf("]);\n");
}

// ----------------------------------------------------------------------------

#endif // TRACE
