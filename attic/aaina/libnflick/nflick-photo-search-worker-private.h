/******************************************************************************/
/*                                                                            */
/* GPL license, Copyright (c) 2005-2006 by:                                   */
/*                                                                            */
/* Authors:                                                                   */
/*      Michael Dominic K. <michaldominik@gmail.com>                          */
/*                                                                            */
/* This program is free software; you can redistribute it and/or modify it    */
/* under the terms of the GNU General Public License as published by the      */
/* Free Software Foundation; either version 2, or (at your option) any later  */
/* version.                                                                   */
/*                                                                            */
/* This program is distributed in the hope that it will be useful, but        */
/* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY */
/* or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   */
/* for more details.                                                          */
/*                                                                            */
/* You should have received a copy of the GNU General Public License along    */
/* with this program; if not, write to the Free Software Foundation, Inc., 59 */
/* Temple Place - Suite 330, Boston, MA 02111-1307, USA.                      */
/*                                                                            */
/******************************************************************************/

static NFlickWorkerClass*       ParentClass = NULL;

struct                          _NFlickPhotoSearchWorkerPrivate
{
        gchar *UserNsid;
        gchar *Token;
        GList *PhotoSets;
};

enum 
{
        ARG_0,
};

static void                     
nflick_photo_search_worker_class_init (NFlickPhotoSearchWorkerClass *klass);

static void                     
nflick_photo_search_worker_init (NFlickPhotoSearchWorker *self);

static gboolean                 
private_init (NFlickPhotoSearchWorker *self, 
              NFlickPhotoSearchWorkerPrivate *priv);

static void                     
private_dispose (NFlickPhotoSearchWorkerPrivate *priv);

static void                     
nflick_photo_search_worker_dispose (NFlickPhotoSearchWorker *self);

static void                     
nflick_photo_search_worker_finalize (NFlickPhotoSearchWorker *self);

static NFlickWorkerStatus       
thread_func (NFlickPhotoSearchWorker *self);

static void                     
nflick_photo_search_worker_get_property (NFlickPhotoSearchWorker *self, 
                                         guint propid, 
                                         GValue *value, GParamSpec *pspec);

