/*
 *  olist.c
 *
 *  $Id$
 *
 *  odbc-bench - a TPC-A and TPC-C like benchmark program for databases
 *  Copyright (C) 2000-2018 OpenLink Software <odbc-bench@openlinksw.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <stdio.h>
#include <stdlib.h>
#include "olist.h"


OList *
o_list_alloc (void)
{
  OList *new_list;

  new_list = (OList *) calloc (1, sizeof (OList));
  new_list->data = NULL;
  new_list->next = NULL;
  new_list->prev = NULL;

  return new_list;
}


void
o_list_free (OList * list)
{
  OList *iter = list;
  OList *next;

  if (list)
    {
      while (iter)
	{
	  next = o_list_next (iter);
	  free (iter);
	  iter = next;
	}
    }
}


OList *
o_list_append (OList * list, void *data)
{
  OList *new_list;
  OList *last;

  new_list = o_list_alloc ();
  new_list->data = data;

  if (list)
    {
      last = o_list_last (list);
      last->next = new_list;
      new_list->prev = last;

      return list;
    }
  else
    return new_list;
}


OList *
o_list_last (OList * list)
{
  if (list)
    {
      while (list->next)
	list = list->next;
    }

  return list;
}


OSList *
o_slist_alloc (void)
{
  OSList *new_list;

  new_list = (OSList *) calloc (1, sizeof (OSList));

  new_list->data = NULL;
  new_list->next = NULL;

  return new_list;
}


void
o_slist_free (OSList * list)
{
  OSList *iter = list;
  OSList *next;

  if (list)
    {
      while (iter)
	{
	  next = o_slist_next (iter);
	  free (iter);
	  iter = next;
	}
    }
}


OSList *
o_slist_append (OSList * list, void *data)
{
  OSList *new_list;
  OSList *last;

  new_list = o_slist_alloc ();
  new_list->data = data;

  if (list)
    {
      last = o_slist_last (list);
      last->next = new_list;

      return list;
    }
  else
    return new_list;
}


OSList *
o_slist_last (OSList * list)
{
  if (list)
    {
      while (list->next)
	list = list->next;
    }

  return list;
}
