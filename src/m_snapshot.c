//
//  Copyright (C) 2022 Fabian Greffrath
//
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
//      Savegame snapshots
//

#include <sys/stat.h>
#include <time.h>

#include "doomtype.h"
#include "doomstat.h"

#include "i_video.h"
#include "m_io.h"
#include "v_video.h"

static const char snapshot_str[] = "WOOF_SNAPSHOT";
static const int snapshot_len = arrlen(snapshot_str);
static const int snapshot_size = ORIGWIDTH * ORIGHEIGHT;

static byte *snapshots[10];
static byte *current_snapshot;
static char savegametimes[10][32];

const int M_SnapshotDataSize (void)
{
  return snapshot_len + snapshot_size;
}

void M_ResetSnapshot (int i)
{
  if (snapshots[i])
  {
    free(snapshots[i]);
    snapshots[i] = NULL;
  }
}

// [FG] try to read snapshot data from the end of a savegame file

boolean M_ReadSnapshot (int i, FILE *fp)
{
  char str[16] = {0};

  M_ResetSnapshot (i);

  if (fseek(fp, -M_SnapshotDataSize(), SEEK_END) != 0)
    return false;

  if (fread(str, 1, snapshot_len, fp) != snapshot_len)
    return false;

  if (strncasecmp(str, snapshot_str, snapshot_len) != 0)
    return false;

  if ((snapshots[i] = malloc(snapshot_size * sizeof(**snapshots))) == NULL)
    return false;

  if (fread(snapshots[i], 1, snapshot_size, fp) != snapshot_size)
    return false;

  return true;
}

void M_ReadSavegameTime (int i, char *name)
{
  struct stat st;

  if (M_stat(name, &st) == -1)
    savegametimes[i][0] = '\0';
  else
// [FG] suppress the most useless compiler warning ever
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-y2k"
#endif
    strftime(savegametimes[i], sizeof(savegametimes[i]), "%x %X", localtime(&st.st_mtime));
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
}

char *M_GetSavegameTime (int i)
{
  return savegametimes[i];
}

// [FG] take a snapshot in ORIGWIDTH*ORIGHEIGHT resolution, i.e.
//      in hires mode only only each second pixel in each second row is saved,
//      in widescreen mode only the non-widescreen part in the middle is saved

void M_TakeSnapshot (void)
{
  const int inc = hires ? 2 : 1;
  int x, y;
  byte *p;
  const byte *s = screens[0];
  static int old_gametic = -1;

  if (old_gametic != gametic)
  {
    old_gametic = gametic;
  }
  else
  {
    return;
  }

  if (!current_snapshot)
  {
    current_snapshot = malloc(snapshot_size * sizeof(**snapshots));
  }
  p = current_snapshot;

  for (y = 0; y < (SCREENHEIGHT << hires); y += inc)
  {
    for (x = 0; x < (NONWIDEWIDTH << hires); x += inc)
    {
      *p++ = s[y * (SCREENWIDTH << hires) + (WIDESCREENDELTA << hires) + x];
    }
  }
}

void M_WriteSnapshot (byte *p)
{
  memcpy(p, snapshot_str, snapshot_len);
  p += snapshot_len;

  memcpy(p, current_snapshot, snapshot_size);
  p += snapshot_size;
}

// [FG] draw snapshot for the n'th savegame, if no snapshot is found
//      fill the area with palette index 0 (i.e. mostly black)

boolean M_DrawSnapshot (int n, int x, int y, int w, int h)
{
  byte *dest = screens[0] + y * (SCREENWIDTH << (2 * hires)) + (x << hires);

  if (!snapshots[n])
  {
    int desty;

    for (desty = 0; desty < (h << hires); desty++)
    {
      memset(dest, 0, w << hires);
      dest += SCREENWIDTH << hires;
    }

    return false;
  }
  else
  {
    const fixed_t step_x = (ORIGWIDTH << FRACBITS) / (w << hires);
    const fixed_t step_y = (ORIGHEIGHT << FRACBITS) / (h << hires);
    int destx, desty;
    fixed_t srcx, srcy;
    byte *destline, *srcline;

    for (desty = 0, srcy = 0; desty < (h << hires); desty++, srcy += step_y)
    {
      destline = dest + desty * (SCREENWIDTH << hires);
      srcline = snapshots[n] + (srcy >> FRACBITS) * ORIGWIDTH;

      for (destx = 0, srcx = 0; destx < (w << hires); destx++, srcx += step_x)
      {
        *destline++ = srcline[srcx >> FRACBITS];
      }
    }
  }

  return true;
}
