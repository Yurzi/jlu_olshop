#ifndef __EXTRAIO_H__
#define __EXTRAIO_H__

#include<malloc.h>
#include<stdio.h>

#if defined (WINNT)||defined(_WIN32)||defined(_WIN64)

int vasprintf (char **ptr, const char *format, va_list ap) {
	int len;	//to save length of ap buf
	len = _vscprintf_p(format, ap) + 1;	//get the count of agrs

	*ptr = (char*)malloc(len * sizeof(char));

	if (!*ptr) {return -1;}

	return _vsprintf_p(*ptr, len, format, ap);
}

#else

int vasprintf (char **ptr, const char *format, va_list ap) {
	va_list ap_copy;

	//make sure it is determinate, despite manuals indicating othrewise
	*ptr = 0;

	va_copy(ap_copy, ap);
	int count = vsnprintf(NULL, 0, format, ap);
	if (count >= 0) {
		char* buffer = malloc(count + 1);
		if (buffer != NULL) {
			count = vsnprintf(buffer, count + 1, format, ap_copy);
			if (count < 0) {
				free(buffer);
			} else {
				*ptr = buffer;
			}
		}
	}

	va_end(ap_copy);

	return count;
}

#endif

#endif	//__EXTRAIO_H__