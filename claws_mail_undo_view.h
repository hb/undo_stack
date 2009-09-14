/* Claws Mail -- a GTK+ based, lightweight, and fast e-mail client
 * Copyright (C) 2009 Holger Berndt <hb@claws-mail.org>
 * and the Claws Mail team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CLAWS_MAIL_UNDO_VIEW_H
#define CLAWS_MAIL_UNDO_VIEW_H

#include <glib-object.h>

#include<gtk/gtk.h>

#include "claws_mail_undo.h"


G_BEGIN_DECLS

#define CLAWS_MAIL_TYPE_UNDO_VIEW claws_mail_undo_view_get_type()

#define CLAWS_MAIL_UNDO_VIEW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), CLAWS_MAIL_TYPE_UNDO_VIEW, ClawsMailUndoView))
#define CLAWS_MAIL_UNDO_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), CLAWS_MAIL_TYPE_UNDO_VIEW, ClawsMailUndoViewClass))
#define CLAWS_MAIL_IS_UNDO_VIEW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CLAWS_MAIL_TYPE_UNDO_VIEW))
#define CLAWS_MAIL_IS_UNDO_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CLAWS_MAIL_TYPE_UNDO_VIEW))
#define CLAWS_MAIL_UNDO_VIEW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), CLAWS_MAIL_TYPE_UNDO_VIEW, ClawsMailUndoViewClass))

typedef struct _ClawsMailUndoView ClawsMailUndoView;
typedef struct _ClawsMailUndoViewClass ClawsMailUndoViewClass;
typedef struct _ClawsMailUndoViewPrivate ClawsMailUndoViewPrivate;

struct _ClawsMailUndoView
{
  GtkVBox parent;

  ClawsMailUndoViewPrivate *priv;
};

struct _ClawsMailUndoViewClass
{
  GtkVBoxClass parent_class;
};

GType claws_mail_undo_view_get_type(void);

GtkWidget* claws_mail_undo_view_new(ClawsMailUndo *undo);

G_END_DECLS

#endif /* CLAWS_MAIL_UNDO_VIEW_H */
