/* $Id: util.c,v 1.24 2009/05/25 02:40:10 sbajic Exp $ */

/*
 DSPAM
 COPYRIGHT (C) 2002-2009 DSPAM PROJECT

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; version 2
 of the License.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifdef HAVE_CONFIG_H
#include <auto-config.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <error.h>
#ifndef _WIN32
#   include <pwd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#   include <unistd.h>
#endif
#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <signal.h>

#ifdef TIME_WITH_SYS_TIME
#   include <sys/time.h>
#   include <time.h>
#else
#   ifdef HAVE_SYS_TIME_H
#       include <sys/time.h>
#   else
#       include <time.h>
#   endif
#endif

#include "language.h"
#include "config.h"
#include "util.h"
#include "libdspam.h"

#ifdef _WIN32
    #include <direct.h>

    #define mkdir(filename, perm) _mkdir(filename)
#endif

#ifndef HAVE_STRTOK_R
char *
strtok_r(char *s1, const char *s2, char **lasts)
{
  char *ret;
                                                                                
  if (s1 == NULL)
    s1 = *lasts;
  while(*s1 && strchr(s2, *s1))
    ++s1;
  if(*s1 == '\0')
    return NULL;
  ret = s1;
  while(*s1 && !strchr(s2, *s1))
    ++s1;
  if(*s1)
    *s1++ = '\0';
  *lasts = s1;
  return ret;
}
#endif /* HAVE_STRTOK_R */

double _ds_gettime(void)
{
  double t;

#ifdef _WIN32
  t = GetTickCount()/1000.;
#else /* !_WIN32 */
  struct timeval tv;
  if (gettimeofday(&tv, NULL) != -1 )
    t = tv.tv_usec/1000000.0 + tv.tv_sec;
  else
    t = 0.;
#endif /* _WIN32/!_WIN32 */

  return t;
}

/* Compliments of Jay Freeman <saurik@saurik.com> */

#ifndef HAVE_STRSEP
char *
strsep (char **stringp, const char *delim)
{
  char *ret = *stringp;
  if (ret == NULL)
    return (NULL);
  if ((*stringp = strpbrk (*stringp, delim)) != NULL)
    *((*stringp)++) = '\0';
  return (ret);
}
#endif

void
chomp (char *string)
{
  int len;
  if (string == NULL)
    return;
  len = strlen (string);
  if (len && string[len - 1] == 10)
  {
    string[len - 1] = 0;
    len--;
  }
  if (len && string[len - 1] == 13)
    string[len - 1] = 0;
  return;
}

char *
ltrim (char *str)
{
  char *p;
  if (!str || !str[0])
    return str;
  for (p = str; isspace ((int) *p); ++p)
  {
    /* do nothing */
  }
  if (p > str)
    strcpy (str, p);            /* __STRCPY_CHECKED__ */
  return str;
}

char *
rtrim (char *str)
{
  size_t offset;
  char *p;
  if (!str || !str[0])
    return str;
  offset = strlen (str);
  p = str + offset - 1;         /* now p points to the last character in
                                 * string */
  for (; p >= str && isspace ((int) *p); --p)
  {
    *p = 0;
  }
  return str;
}

#ifndef HAVE_STRLCPY
/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(src) + MIN(siz, strlen(initial dst)).
 * If retval >= siz, truncation occurred.
 */
size_t
strlcat (dst, src, siz)
     char *dst;
     const char *src;
     size_t siz;
{
  register char *d = dst;
  register const char *s = src;
  register size_t n = siz;
  size_t dlen;

  /* Find the end of dst and adjust bytes left but don't go past end */
  while (n-- != 0 && *d != '\0')
    d++;
  dlen = d - dst;
  n = siz - dlen;

  if (n == 0)
    return (dlen + strlen (s));
  while (*s != '\0')
  {
    if (n != 1)
    {
      *d++ = *s;
      n--;
    }
    s++;
  }
  *d = '\0';

  return (dlen + (s - src));    /* count does not include NUL */
}

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t
strlcpy (dst, src, siz)
     char *dst;
     const char *src;
     size_t siz;
{
  register char *d = dst;
  register const char *s = src;
  register size_t n = siz;

  /* Copy as many bytes as will fit */
  if (n != 0 && --n != 0)
  {
    do
    {
      if ((*d++ = *s++) == 0)
        break;
    }
    while (--n != 0);
  }

  /* Not enough room in dst, add NUL and traverse rest of src */
  if (n == 0)
  {
    if (siz != 0)
      *d = '\0';                /* NUL-terminate dst */
    while (*s++)
      ;
  }

  return (s - src - 1);         /* count does not include NUL */
}
#endif

const char * _ds_userdir_path (char *path, const char *home, const char *filename, const char *extension)
{
  char username[MAX_FILENAME_LENGTH];
  char userpath[MAX_FILENAME_LENGTH];
#ifdef DOMAINSCALE
  char *f, *domain, *user;
  char *ptrptr;
#endif
#ifdef HOMEDIR
  struct passwd *p;
#if defined(_REENTRANT) && defined(HAVE_GETPWNAM_R)
  struct passwd pwbuf;
  char buf[1024];
#endif
  char userhome[MAX_FILENAME_LENGTH];
#endif

  if (filename == NULL || filename[0] == 0)
  {
    path[0] = 0;
    return path;
  }

#ifdef HOMEDIR
#if defined(_REENTRANT) && defined(HAVE_GETPWNAM_R)
  if (getpwnam_r(filename, &pwbuf, buf, sizeof(buf), &p))
    p = NULL;
#else
  p = getpwnam(filename);
#endif

  if (p == NULL) 
      strcpy(userhome, home);
  else
      strlcpy(userhome, p->pw_dir, sizeof(userhome));

  if (extension != NULL
      && (!strcmp (extension, "nodspam") || !strcmp (extension, "dspam")))
  {
    if (p != NULL) {
      snprintf (path, MAX_FILENAME_LENGTH, "%s/.%s", p->pw_dir, extension);
#ifdef DEBUG
      LOGDEBUG ("using %s as path", path);
#endif
      return path;
    }
  }
#endif /* HOMEDIR */

#ifdef DOMAINSCALE
  f = strdup(filename);
  user = strtok_r(f, "@", &ptrptr);
  domain = strtok_r(NULL, "@", &ptrptr);
                                                                                
  if (domain == NULL)
    domain = "local";
  snprintf(userpath, MAX_FILENAME_LENGTH, "%s/%s", domain, user);
  strlcpy(username, user, MAX_FILENAME_LENGTH);
  free(f);
#else
  strlcpy(username, filename, MAX_FILENAME_LENGTH);
  strcpy(userpath, username);
#endif

  /* Use home/opt-in/ and home/opt-out/ to store opt files, instead of
     each user's directory */

  if (extension != NULL
      && (!strcmp (extension, "nodspam") || !strcmp (extension, "dspam")))
  {
    snprintf (path, MAX_FILENAME_LENGTH, "%s/opt-%s/%s.%s", home, 
      (!strcmp(extension, "nodspam")) ? "out" : "in", userpath, extension);
#ifdef DEBUG
      LOGDEBUG ("using %s as path", path);
#endif
      return path; 
  }

#ifdef LARGESCALE
  if (filename[1] != 0)
  {
    if (extension == NULL)
    {
      snprintf (path, MAX_FILENAME_LENGTH, "%s/data/%c/%c/%s",
                home, filename[0], filename[1], filename);
    }
    else
    {
      if (extension[0] == 0)
        snprintf (path, MAX_FILENAME_LENGTH, "%s/data/%c/%c/%s/%s",
                  home, filename[0], filename[1], filename, filename);
      else
        snprintf (path, MAX_FILENAME_LENGTH, "%s/data/%c/%c/%s/%s.%s",
                  home, filename[0], filename[1], filename, filename,
                  extension);
    }
  }
  else
  {
    if (extension == NULL)
    {
      snprintf (path, MAX_FILENAME_LENGTH, "%s/data/%c/%s",
                home, filename[0], filename);
    }
    else
    {
      if (extension[0] == 0)
        snprintf (path, MAX_FILENAME_LENGTH, "%s/data/%c/%s/%s",
                  home, filename[0], filename, filename);
      else
        snprintf (path, MAX_FILENAME_LENGTH, "%s/data/%c/%s/%s.%s",
                  home, filename[0], filename, filename, extension);
    }
  }
#else
  if (extension == NULL)
  {
#ifdef HOMEDIR
    snprintf (path, MAX_FILENAME_LENGTH, "%s/.dspam", userhome);
#else
    snprintf (path, MAX_FILENAME_LENGTH, "%s/data/%s", home, userpath);
#endif
  }
  else
  {
#ifdef HOMEDIR
    snprintf(path, MAX_FILENAME_LENGTH, "%s/.dspam/%s.%s", userhome, username,
             extension);
#else
    snprintf (path, MAX_FILENAME_LENGTH, "%s/data/%s/%s.%s",
              home, userpath, username, extension);
#endif
  }
#endif

  return path;
}

int
_ds_prepare_path_for (const char *filename)
{
  char path[MAX_FILENAME_LENGTH];
  char *dir, *file;
  char *file_buffer_start;
  struct stat s;
                                                                                
  if (!filename) 
    return EINVAL;

  file = strdup (filename);
  if (!file) {
    LOG (LOG_ERR, ERR_MEM_ALLOC);
    return EFAILURE;
  }

#ifdef _WIN32
  /*
     Windows uses both slash and backslash as path separators while the code
     below only deals with slashes -- make it work by adjusting the path.
   */
  {
    char *p;
    for ( p = strchr(file, '\\'); p; p = strchr(p + 1, '\\') )
    {
      *p = '/';
    }
  }
#endif

  file_buffer_start = file;
  path[0] = 0;
                                                                                
  dir = strsep (&file, "/");
  while (dir != NULL)
  {
    strlcat (path, dir, sizeof (path));
    dir = strsep (&file, "/");

#ifdef _WIN32
    /* don't try to create root directory of a drive */
    if ( path[2] != '\0' || path[1] != ':' )
#endif
   {
#ifdef EXT_LOOKUP
	  /* don't create users data dir if user verification is required */
      if (dir != NULL && stat (path, &s) && path[0] != 0 && verified_user == 1)
#else
	  if (dir != NULL && stat (path, &s) && path[0] != 0)
#endif
      {
        int x;
        LOGDEBUG ("creating directory '%s'", path);
        x = mkdir (path, 0770);
        if (x)
        {
          LOG(LOG_ERR, ERR_IO_DIR_CREATE, path, strerror (errno));
          free (file_buffer_start);
          return EFILE;
        }
      }
    }

    strlcat (path, "/", sizeof (path));
  }
  free (file_buffer_start);
  return 0;
}

int
lc (char *buff, const char *string)
{
  char *buffer;
  int i, j = 0;
  int len = strlen (string);

  buffer = malloc (len + 1);

  if (len == 0)
  {
    buff[0] = 0;
    free (buffer);
    return 0;
  }

  for (i = 0; i < len; i++)
  {
    if (isupper ((int) string[i]))
    {
      buffer[i] = tolower (string[i]);
      j++;
    }
    else
    {
      buffer[i] = string[i];
    }
  }

  buffer[len] = 0;
  strcpy (buff, buffer);

  free (buffer);
  return j;
}

unsigned long long
_ds_getcrc64 (const char *s)
{
  static unsigned long long CRCTable[256];
  unsigned long long crc = 0;
  static int init = 0;
                                                                                
  if (!init)
  {
    int i;
    init = 1;
    for (i = 0; i <= 255; i++)
    {
      int j;
      unsigned long long part = i;
      for (j = 0; j < 8; j++)
      {
        if (part & 1)
          part = (part >> 1) ^ POLY64REV;
        else
          part >>= 1;
      }
      CRCTable[i] = part;
    }
  }
  for (; *s; s++)
  {
    unsigned long long temp1 = crc >> 8;
    unsigned long long temp2 =
      CRCTable[(crc ^ (unsigned long long) *s) & 0xff];
    crc = temp1 ^ temp2;
  }
                                                                                
  return crc;
}

int _ds_compute_weight(const char *token) {
  int complexity = _ds_compute_complexity(token);
  int sparse = _ds_compute_sparse(token);

/*
 * Mathematically correct algorithm (but slower):
 *
 * int weight = 0;
 *
 * if (complexity >= 1 && complexity <= SPARSE_WINDOW_SIZE) {
 *   weight = (int)pow(2.0,(2*(complexity-sparse-1)));
 *   if (weight < 1)
 *     return 1;
 *   else
 *     return weight;
 * }
 */


/*
 * The same (+/-) as above but without using an algorithm (and
 * therefore faster then calling each time the pow() function).
 * Using reverse order of complexity to speed up processing.
 *
 */

  if (complexity == 5) {
    if (sparse == 1)  /*  the * brown fox jumped | the quick * fox jumped | the quick brown * jumped  */
      return 64;
    if (sparse == 2)  /*  the * * fox jumped | the * brown * jumped | the quick * * jumped  */
      return 16;
    if (sparse == 3)  /*  the * * * jumped  */
      return 4;
    if (sparse == 0)  /*  the quick brown fox jumped  */
      return 256;
  }

  if (complexity == 4) {
    if (sparse == 1)  /*  quick * fox jumped | quick brown * jumped  */
      return 16;
    if (sparse == 2)  /*  quick * * jumped  */
      return 4;
    if (sparse == 0)  /*  quick brown fox jumped  */
      return 64;
  }

  if (complexity == 3) {
    if (sparse == 1)  /*  bron * jumped  */
      return 4;
    if (sparse == 0)  /*  brown fox jumped  */
      return 16;
  }

  if (complexity == 2) {
    if (sparse == 0)  /*  fox jumped  */
      return 4;
  }

  if (complexity == 1) {
    if (sparse == 0)  /*  jumped  */
      return 1;
  }

  LOG(LOG_WARNING, "_ds_compute_weight: no rule to compute markovian weight for '%s'; complexity: %d; sparse: %d", token, complexity, sparse);
  return 1;
}

int _ds_compute_sparse(const char *token) {
  int sparse = 0, i;

  if (!strncmp(token, "#+", 2))
    sparse++;
  if (strlen(token)>=2 && !strncmp((token+strlen(token))-2, "+#", 2))
    sparse++;
  for(i=0;token[i];i++) {
    if (!strncmp(token+i, "+#+", 3)) {
      sparse++;
      i++;
    }
  }

  return sparse;
}

int _ds_compute_complexity(const char *token) {
  int i, complexity = 1;

  if (token == NULL)
    return 1;

  for(i=0;token[i];i++) {
    if (token[i] == '+') {
      complexity++;
      i++;
    }
  }

  return complexity;
}

int _ds_extract_address(char *buf, const char *address, size_t len) {
  char *str = strdup(address);
  char *x, *y;

  if (str == NULL)
    return EUNKNOWN;
 
  x = strchr(str, '<');
  if (!x) {
    free(str);
    return EFAILURE;
  }

  y = strchr(x, '>');
  if (y) y[0] = 0;

  strlcpy(buf, x+1, len);
  free(str);
  return 0;
}

double chi2Q (double x, int v)
{
  int i;
  double m, s, t;

  m = x / 2.0;
  s = exp(-m);
  t = s;

  for(i=1;i<(v/2);i++) {
    t *= m / i;
    s += t;
  }

  return MIN(s, 1.0);
}

void timeout(){}

int _ds_get_fcntl_lock(int fd) {
#ifdef _WIN32
  return 0;
#else
  struct flock f;
  int r;

  f.l_type = F_WRLCK;
  f.l_whence = SEEK_SET;
  f.l_start = 0;
  f.l_len = 0;

  signal(SIGALRM,timeout);
  alarm(300);
  r=fcntl(fd, F_SETLKW, &f);
  alarm(0);
  signal(SIGALRM,SIG_DFL);

  return r;
#endif
}

int _ds_free_fcntl_lock(int fd) {
#ifdef _WIN32
  return 0;
#else
  struct flock f;
                                                                                
  f.l_type = F_UNLCK;
  f.l_whence = SEEK_SET;
  f.l_start = 0;
  f.l_len = 0;

  return fcntl(fd, F_SETLKW, &f);
#endif
} 

int _ds_pow(int base, unsigned int exp) {
  int result = 1;
  while (exp > 0) {
    if (exp & 1)
      result *= base;
    base *= base;
    exp /= 2;
  }
  return result;
}

int _ds_pow2(int exp) {
  return _ds_pow(2, exp);
}

float _ds_round(float n) {
  int r = (n*100);
  while(r % 5)
    r++;
  return (r/100.0);
}

#ifndef HAVE_STRCASESTR
char *
strcasestr(s, find)
	const char *s, *find;
{
	char c, sc;
	size_t len;

	if ((c = *find++) != 0) {
		c = tolower((unsigned char)c);
		len = strlen(find);
		do {
			do {
				if ((sc = *s++) == 0)
					return (NULL);
			} while ((char)tolower((unsigned char)sc) != c);
		} while (strncasecmp(s, find, len) != 0);
		s--;
	}
	return ((char *)s);
}
#endif

#ifndef HAVE_INET_NTOA_R
/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

char *
inet_ntoa_r(struct in_addr in, char *buf, int len)
{
  char b[18];
  register char *p;

  p = (char *)&in;
#define	UC(b)	(((int)b)&0xff)
  snprintf(b, sizeof(b),
  "%d.%d.%d.%d", UC(p[0]), UC(p[1]), UC(p[2]), UC(p[3]));

  if (len <= strlen(b)) {
    errno = ERANGE;
    return(NULL);
  }

  return strcpy(buf, b);
}
#endif
