/*
 * perror.c - print an error message on the standard error output
 */
/* $Header: perror.c,v 1.1 89/05/30 13:31:30 eck Exp $ */

#if	defined(_POSIX_SOURCE)
#include	<sys/types.h>
#endif
#include	<stdio.h>
#include	<errno.h>
#include	<stdio.h>
#include	<string.h>
#include <sys/inter.h>
#include <stdlib.h>

void
perror(const char *s)
{
	char *p;

	p = strerror(errno);
	if (s && *s) {
		write(STDERR_FILENO, s, strlen(s));
		write(STDERR_FILENO, ": ", 2);
	}
	write(STDERR_FILENO, p, strlen(p));
	write(STDERR_FILENO, "\n", 1);
}
