/*
 *  olist.h
 * 
 *  $Id$
 *
 *  odbc-bench - a TPCA and TPCC benchmark program for databases 
 *  Copyright (C) 2000-2002 OpenLink Software <odbc-bench@openlinksw.com>
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
#ifndef __OLIST_H__
#define __OLIST_H__

typedef struct _OList		OList;
typedef struct _OSList		OSList;

struct _OList
{
  void *data;
  OList *next;
  OList *prev;
};

struct _OSList
{
  void *data;
  OSList *next;
};

/* Doubly linked lists
 */
OList* o_list_alloc	  (void);
void   o_list_free	  (OList     *list);
OList* o_list_append	  (OList     *list,
			   void *   data);
OList* o_list_last	  (OList     *list);
#define o_list_next(list) ((list) ? (((OList *)list)->next) : NULL)

/* Singly linked lists
 */
OSList* o_slist_alloc	    (void);
void	o_slist_free	    (OSList   *list);
OSList* o_slist_append	    (OSList   *list,
			     void *  data);
OSList* o_slist_last	    (OSList   *list);
#define o_slist_next(list) ((list) ? (((OSList *)list)->next) : NULL)

#endif /* __OLIST_H__ */
