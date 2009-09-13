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

/* General purpose undo stack. */

#ifndef UNDO_H
#define UNDO_H

#include <glib-object.h>

G_BEGIN_DECLS

#define CLAWS_MAIL_TYPE_UNDO claws_mail_undo_get_type()

#define CLAWS_MAIL_UNDO(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), CLAWS_MAIL_TYPE_UNDO, ClawsMailUndo))
#define CLAWS_MAIL_UNDO_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), CLAWS_MAIL_TYPE_UNDO, ClawsMailUndoClass))
#define CLAWS_MAIL_IS_UNDO(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CLAWS_MAIL_TYPE_UNDO))
#define CLAWS_MAIL_IS_UNDO_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CLAWS_MAIL_TYPE_UNDO))
#define CLAWS_MAIL_UNDO_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), CLAWS_MAIL_TYPE_UNDO, ClawsMailUndoClass))

typedef struct _ClawsMailUndo ClawsMailUndo;
typedef struct _ClawsMailUndoClass ClawsMailUndoClass;

struct _ClawsMailUndo {
  GObject parent;

  /* private */
  GList *undo_stack;
  GList *redo_stack;
  gint len_undo;
  gint len_redo;
  gboolean group_add_mode;
  GHashTable *method_hash;

  /* properties */
  gint maxlen;
};

struct _ClawsMailUndoClass {
  GObjectClass parent_class;

  /* Signals */
  guint signal_id_can_undo;
  guint signal_id_can_redo;
};

GType claws_mail_undo_get_type(void);

typedef struct _ClawsMailUndoSet ClawsMailUndoSet;
struct _ClawsMailUndoSet {
  gchar *description;
  void (*do_undo)(gpointer);
  void (*do_redo)(gpointer);
  void (*do_free)(gpointer);
};

/* API */
ClawsMailUndo *claws_mail_undo_new(void);
void claws_mail_undo_register_set(ClawsMailUndo *undo, const char *name, ClawsMailUndoSet *set);
void claws_mail_undo_add(ClawsMailUndo *undo, const char *set_name, gpointer data, gchar *description);
void claws_mail_undo_clear(ClawsMailUndo *undo);

/* Debugging. These will disappear again */
void claws_mail_undo_print(ClawsMailUndo *undo);
void claws_mail_undo_print_undo(ClawsMailUndo *undo);
void claws_mail_undo_print_redo(ClawsMailUndo *undo);

G_END_DECLS

#endif /* UNDO_H */
