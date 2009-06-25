/* $Id: read_config.c,v 1.18 2009/06/02 01:12:44 sbajic Exp $ */

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

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#ifdef SPLIT_CONFIG
#include <dirent.h>
#endif
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#include <string.h>

#include "config_shared.h"
#include "read_config.h"
#include "config.h"
#include "error.h"
#include "language.h"
#include "libdspam.h"
#include "pref.h"
#include "util.h"

#ifdef SPLIT_CONFIG
long dirread(const char *path, config_t *attrib, long num_root);
long fileread(const char *path, config_t *attrib, long num_root);
#endif

static char *next_normal_token(char **p)
{
  char *start = *p;
  while (**p) {
    if (**p == ' ' || **p == '\t') {
      **p = 0;
      *p += 1;
      break;
    }
    *p += 1;
  }
  return start;
}

static char *next_quoted_token(char **p)
{
  char *start = *p;
  while (**p) {
    if (**p == '\"') {
      **p = 0;
      *p += 1;
      break;
    }
    *p += 1;
  }
  return start;
}

static char *tokenize(char *text, char **next)
{
  /* Initialize */
  if (text)
    *next = text;

  while (**next) {
    /* Skip leading whitespace */
    if (**next == ' ' || **next == '\t') {
      *next += 1;
      continue;
    }

    /* Strip off one token */
    if (**next == '\"') {
      *next += 1;
      return next_quoted_token(next);
    } else
      return next_normal_token(next);
  }

  return NULL;
}

#ifdef SPLIT_CONFIG
// Read the files in the directory and pass it to fileread
// or if it is a file, pass it to fileread.
long dirread(const char *path, config_t *attrib, long num_root) {
  DIR *dir_p;
  char *fulldir;
  struct dirent *dir_entry_p;
  int n, m;

  // Strip "\n"
  char *ptr = strrchr(path, '\n');
  if (ptr)
    *ptr = '\0';

  if ((dir_p = opendir(path))) {
    while((dir_entry_p = readdir(dir_p)))
    {
      // We don't need the . and ..
      if (strcmp(dir_entry_p->d_name, ".") == 0 ||
          strcmp(dir_entry_p->d_name, "..") == 0)
        continue;

      // only use files which end in .conf:
      if (strncmp(dir_entry_p->d_name + strlen(dir_entry_p->d_name) - 5,
                 ".conf", 5) != 0) {
       continue;
      }

      n = strlen(dir_entry_p->d_name);
      m = strlen(path);
      fulldir = (char *)malloc(n+m+2);
      strcpy(fulldir, (char *)path);
      strcat(fulldir, "/");
      strcat(fulldir, dir_entry_p->d_name);
      num_root = fileread((const char *)fulldir, attrib, num_root);
      free(fulldir);
    }
    closedir(dir_p);
  } else {
    // Could be a file.
    return fileread((const char *)path, attrib, num_root);
  }

  return num_root;
}

// Read the file and check if there is an Include directive, if so then pass
// it to dirread.
long fileread(const char *path, config_t *attrib, long num_root) {
  config_t ptr;
#else
config_t read_config(const char *path) {
  config_t attrib, ptr;
#endif
  FILE *file;
#ifdef SPLIT_CONFIG
  long attrib_size = 128;
#else
  long attrib_size = 128, num_root = 0;
#endif
  char buffer[1024];
  char *a, *c, *v, *bufptr = buffer;

#ifndef SPLIT_CONFIG
  attrib = calloc(1, attrib_size*sizeof(attribute_t));
  if (attrib == NULL) {
    LOG(LOG_CRIT, ERR_MEM_ALLOC);
    return NULL;
  }
#endif

  if (path == NULL)
    file = fopen(CONFIG_DEFAULT, "r");
  else
    file = fopen(path, "r");

  if (file == NULL) {
#ifdef SPLIT_CONFIG
    if (path == NULL) {
      LOG(LOG_ERR, ERR_IO_FILE_OPEN, CONFIG_DEFAULT, strerror(errno));
    } else {
      LOG(LOG_ERR, ERR_IO_FILE_OPEN, path, strerror(errno));
    }
    free(*attrib);
    return 0;
#else
    LOG(LOG_ERR, ERR_IO_FILE_OPEN, CONFIG_DEFAULT, strerror(errno));
    free(attrib);
    return NULL;
#endif
  }

  while(fgets(buffer, sizeof(buffer), file)!=NULL) {

    chomp(buffer);

    /* Remove comments */
    if ((c = strchr(buffer, '#')) || (c = strchr(buffer, ';')))
      *c = 0;

    /* Parse attribute name */
    if (!(a = tokenize(buffer, &bufptr)))
      continue; /* Ignore whitespace-only lines */

    while ((v = tokenize(NULL, &bufptr)) != NULL) {
#ifdef SPLIT_CONFIG
      // Check for include directive
      if (strcmp(a, "Include") == 0) {
        // Give v (value) to dirraed
        num_root = dirread(v, attrib, num_root);
      } else {
        if (_ds_find_attribute((*attrib), a)!=NULL) { 
          _ds_add_attribute((*attrib), a, v);
        }
        else {
          num_root++;
          if (num_root >= attrib_size) {
            attrib_size *=2;
            ptr = realloc((*attrib), attrib_size*sizeof(attribute_t)); 
            if (ptr)
              *attrib = ptr;
            else
              LOG(LOG_CRIT, ERR_MEM_ALLOC);
          } 
          _ds_add_attribute((*attrib), a, v);
        }
#else
      if (_ds_find_attribute(attrib, a)!=NULL) { 
        _ds_add_attribute(attrib, a, v);
      }
      else {
        num_root++;
        if (num_root >= attrib_size) {
          attrib_size *=2;
          ptr = realloc(attrib, attrib_size*sizeof(attribute_t)); 
          if (ptr) 
            attrib = ptr;
          else
            LOG(LOG_CRIT, ERR_MEM_ALLOC);
        } 
        _ds_add_attribute(attrib, a, v);
#endif
      }
    }
  }

  fclose(file);

#ifdef SPLIT_CONFIG
  return num_root;
}

config_t read_config(const char *path) {
  config_t attrib;
  long attrib_size = 128, num_root = 0;

  attrib = calloc(1, attrib_size*sizeof(attribute_t));
  if (attrib == NULL) {
    LOG(LOG_CRIT, ERR_MEM_ALLOC);
    return NULL;
  }

  if (fileread(path, &attrib, num_root) == 0)
    return NULL;
#else
  ptr = realloc(attrib, ((num_root+1)*sizeof(attribute_t))+1);
  if (ptr)
    return ptr;
  LOG(LOG_CRIT, ERR_MEM_ALLOC);
#endif

  return attrib;
}

int configure_algorithms(DSPAM_CTX *CTX) {
  if (_ds_read_attribute(agent_config, "Algorithm"))
    CTX->algorithms = 0;
                                                                                
  if (_ds_match_attribute(agent_config, "Algorithm", "graham"))
    CTX->algorithms |= DSA_GRAHAM;
                                                                                
  if (_ds_match_attribute(agent_config, "Algorithm", "burton"))
    CTX->algorithms |= DSA_BURTON;
                                                                                
  if (_ds_match_attribute(agent_config, "Algorithm", "robinson"))
    CTX->algorithms |= DSA_ROBINSON;

  if (_ds_match_attribute(agent_config, "Algorithm", "naive"))
    CTX->algorithms |= DSA_NAIVE;
                                                                                
  if (_ds_match_attribute(agent_config, "PValue", "robinson"))
    CTX->algorithms |= DSP_ROBINSON;
  else if (_ds_match_attribute(agent_config, "PValue", "markov"))
    CTX->algorithms |= DSP_MARKOV;
  else
    CTX->algorithms |= DSP_GRAHAM;

  if (_ds_match_attribute(agent_config, "Tokenizer", "word")) 
    CTX->tokenizer = DSZ_WORD;
  else if (_ds_match_attribute(agent_config, "Tokenizer", "chain") ||
           _ds_match_attribute(agent_config, "Tokenizer", "chained"))
    CTX->tokenizer = DSZ_CHAIN;
  else if (_ds_match_attribute(agent_config, "Tokenizer", "sbph"))
    CTX->tokenizer = DSZ_SBPH;
  else if (_ds_match_attribute(agent_config, "Tokenizer", "osb"))
    CTX->tokenizer = DSZ_OSB;
 
  if (_ds_match_attribute(agent_config, "Algorithm", "chi-square"))
  {
    if (CTX->algorithms != 0 && CTX->algorithms != DSP_ROBINSON) {
      LOG(LOG_WARNING, "Warning: Chi-Square algorithm enabled with other algorithms. False positives may ensue.");
    }
    CTX->algorithms |= DSA_CHI_SQUARE;
  }

  return 0;
}

agent_pref_t pref_config(void)
{
  agent_pref_t PTX = malloc(sizeof(agent_attrib_t)*PREF_MAX);
  agent_pref_t ptr;
  attribute_t attrib;
  char *p, *q;
  char *ptrptr = NULL;
  int i = 0;

  if (PTX == NULL) {
    LOG(LOG_CRIT, ERR_MEM_ALLOC);
    return NULL;
  }
  PTX[0] = NULL;

  /* Apply default preferences from dspam.conf */
                                                                                
  attrib = _ds_find_attribute(agent_config, "Preference");
                                                                                
  LOGDEBUG("Loading preferences from dspam.conf");
                                                                                
  while(attrib != NULL) {
    char *pcopy = strdup(attrib->value);
                                                                              
    p = strtok_r(pcopy, "=", &ptrptr);
    if (p == NULL) {
      free(pcopy);
      continue;
    }
    q = p + strlen(p)+1;

    PTX[i] = _ds_pref_new(p, q);
    PTX[i+1] = NULL;

    i++;
    attrib = attrib->next;
    free(pcopy);
  }

  ptr = realloc(PTX, sizeof(agent_attrib_t)*(i+1));
  if (ptr)
    return ptr;
  
  LOG(LOG_CRIT, ERR_MEM_ALLOC);
  return PTX;
}
