//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
//  02111-1307, USA.
//
// DESCRIPTION:
//      unicode paths for fopen() on Windows

#ifndef __WIN_D_FOPEN__
#define __WIN_D_FOPEN__

#ifdef _WIN32
#include <stdio.h>

FILE* D_fopen(const char *filename, const char *mode);
int D_remove(const char *path);

#undef  fopen
#define fopen(n, m) D_fopen(n, m)

#undef  remove
#define remove(p) D_remove(p)
#endif

#endif