/* $Id: base64.h,v 1.7 2006/05/13 01:12:59 jonz Exp $ */

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

#ifndef _BASE64_H
#  define _BASE64_H

#ifndef NCORE
char *base64decode (const char *);
#endif
char *base64encode (const char *);

#endif /* _BASE64_H */
