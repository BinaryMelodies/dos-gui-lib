
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "api.h"

# define _FMT_LL '\1'

/** Prints a formatted string to a (far) pointer, writing at most the specified number of bytes **/
int _fsnprintf(char far * str, size_t n, const char far * fmt, ...)
{
	/* TODO: use n even when formatting arguments */
	int i, len;
	int fill;
	int width;
	int size;
	int prec;

	va_list list;
	va_start(list, fmt);
	for(i = len = 0; fmt[i] && len < n - 1; i++)
	{
		if(fmt[i] == '%')
		{
			fill = ' ';
			width = 0;
			size = 0;
			prec = -1;
		retry:
			switch(fmt[++i])
			{
			case '%':
				str[len++] = '%';
				break;
			case '0':
				if(width == 0 && prec == -1)
				{
					fill = '0';
					goto retry;
				}
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if(prec == -1)
					width = 10 * width + fmt[i] - '0';
				else
					prec = 10 * prec + fmt[i] - '0';
				goto retry;
			case '.':
				if(prec == -1)
					prec = 0;
				goto retry;
			case '*':
				{
					int arg = va_arg(list, int);
					if(prec == -1)
						width = arg;
					else
						prec = arg;
				}
				goto retry;
			case 'l':
				if(size == 'l')
					size = _FMT_LL;
				else
					size = fmt[i];
				goto retry;
			case 'W':
				size = fmt[i - 1];
				goto retry;
			case 's':
				{
					char far * arg;
					if(size == 'W')
					{
						arg = va_arg(list, char far *);
					}
					else
					{
						arg = va_arg(list, char *);
					}

					if(prec == -1)
					{
						_fstrcpy(str + len, arg);
						len += _fstrlen(arg);
					}
					else
					{
						int newlen;
						_fstrncpy(str + len, arg, prec);
						newlen = _fstrlen(str);
						if(newlen < len + prec)
							len = newlen;
						else
							len += prec;
					}
				}
				break;
			case 'd':
				{
					static char tmp[24];
					int arglen;
					if(size == _FMT_LL)
					{
						long long arg = va_arg(list, long long);
						_lltoa(arg, tmp, 10);
					}
					else if(size == 'l')
					{
						long arg = va_arg(list, long);
						_ltoa(arg, tmp, 10);
					}
					else
					{
						int arg = va_arg(list, int);
						_itoa(arg, tmp, 10);
					}
					arglen = strlen(tmp);
					while(arglen < width)
					{
						str[len++] = fill;
						width --;
					}
					_fstrcpy(str + len, tmp);
					len += arglen;
				}
				break;
			case 'u':
				{
					static char tmp[21];
					int arglen;
					if(size == _FMT_LL)
					{
						unsigned long long arg = va_arg(list, unsigned long long);
						_ulltoa(arg, tmp, 10);
					}
					else if(size == 'l')
					{
						unsigned long arg = va_arg(list, unsigned long);
						_ultoa(arg, tmp, 10);
					}
					else
					{
						unsigned arg = va_arg(list, unsigned);
						_utoa(arg, tmp, 10);
					}
					arglen = strlen(tmp);
					while(arglen < width)
					{
						str[len++] = fill;
						width --;
					}
					_fstrcpy(str + len, tmp);
					len += arglen;
				}
				break;
			case 'X':
			case 'x':
				{
					static char tmp[17];
					int arglen;
					if(size == _FMT_LL)
					{
						unsigned long long arg = va_arg(list, unsigned long long);
						_ulltoa(arg, tmp, 16);
					}
					else if(size == 'l')
					{
						unsigned long arg = va_arg(list, unsigned long);
						_ultoa(arg, tmp, 16);
					}
					else
					{
						unsigned arg = va_arg(list, unsigned);
						_utoa(arg, tmp, 16);
					}
					arglen = strlen(tmp);
					while(arglen < width)
					{
						str[len++] = fill;
						width --;
					}
					_fstrcpy(str + len, tmp);
					len += arglen;
				}
				break;
			}
		}
		else
		{
			str[len++] = fmt[i];
		}
	}
	str[len] = '\0';
	va_end(list);
	return len;
}

