/*
 *  NcursesNetRadio - network radio playlists' frontend 
 *  Copyright (C) 2010 Kurashov Artem Konstantinovich
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */
#include <curses.h>
#include <menu.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

FILE *fptr;
ITEM **mplaylistitems;
char **playlist;

const char *plrcmd[] = {
  "ogg123",
  "mpg123"
};

char *nullstring[] = { "NULL" };

char *
stringreader ()
{
  int c, k = 1;
  char *sbuf;
  while ((c = fgetc (fptr)) != '\n')
    k++;
  if (k == 1)
    return NULL;
  fseek (fptr, -1 * k, SEEK_CUR);
  sbuf = (char *) malloc (sizeof (char) * k);
  k = 0;
  while ((c = fgetc (fptr)) != '\n')
    {
      sbuf[k] = c;
      k++;
    }
  sbuf[k] = '\0';
  return sbuf;
}

int
stringcounter ()
{
  int c, k = 0;
  while ((c = fgetc (fptr)) != EOF)
    if (c == '\n')
      k++;
  rewind (fptr);
  return k;
}

int
readplaylist (char *filename)
{
  int s, k = 0;
  char *srstr;
  if ((fptr = fopen (filename, "r")) == NULL)
    return 1;
  s = stringcounter ();
  if (s % 2 == 1)
    {
      fclose (fptr);
      return 1;
    }
  playlist = (char **) malloc (sizeof (char *) * s);
  mplaylistitems = (ITEM **) malloc (sizeof (ITEM *) * s / 2 + 1);
  for (k = 0; k < s; k++)
    {
      if ((srstr = stringreader ()) == NULL)
	{
	  playlist[k] = nullstring;
	}
      else
	{
	  playlist[k] = (char *) malloc (sizeof (char) * strlen (srstr));
	  strcpy (playlist[k], srstr);
	  free (srstr);
	}
      if (k % 2 == 0)
	mplaylistitems[k / 2] = new_item (playlist[k], "");
    }
  fclose (fptr);
  return 0;
}

main (int argc, char **argv)
{
  WINDOW *nrw;
  MENU *mplaylist;
  char key0;
  int icur, rpstat, pindx;
  pid_t rplayer;
  if (argc < 2)
    {
      printf ("usage: nradio playlistfile\n");
      return 1;
    }
  initscr ();
  clear ();
  noecho ();
  curs_set (0);
  cbreak ();
  if (readplaylist (argv[1]) == 1)
    {
      endwin ();
      printf ("error: cann't read playlist file\n");
      return 1;
    }
  nrw = newwin (LINES, COLS, 0, 0);
  mplaylist = new_menu ((ITEM **) mplaylistitems);
  set_menu_win (mplaylist, nrw);
  set_menu_sub (mplaylist, derwin (nrw, LINES, COLS, 0, 0));
  set_menu_mark (mplaylist, "");
  post_menu (mplaylist);
  wsyncup (nrw);
  for (;;)
    {
      key0 = wgetch (nrw);
      switch (key0)
	{
	case 'j':
	  menu_driver (mplaylist, REQ_DOWN_ITEM);
	  break;
	case 'k':
	  menu_driver (mplaylist, REQ_UP_ITEM);
	  break;
	case 'q':
	  endwin ();
	  return 0;
	case 10:
	  icur = current_item (mplaylist)->index;
	  if ((strstr (playlist[icur * 2 + 1], ".ogg")) == NULL)
	    pindx = 1;
	  else
	    pindx = 0;
	  rplayer = fork ();
	  if (rplayer == 0)
	    {
	      touchwin (stdscr);
	      move (0, 0);
	      refresh ();
	      execlp (plrcmd[pindx], "-d oss -q", playlist[icur * 2 + 1]);
	    }
	  else
	    {
	      while (getch () != 'q');
	      kill (rplayer, 9);
	      clear ();
	      touchwin (stdscr);
	      touchwin (nrw);
	      refresh ();
	    }
	}
      wrefresh (nrw);
    }
  endwin ();
}
