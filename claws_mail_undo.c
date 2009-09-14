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

#include "claws_mail_undo.h"

G_DEFINE_TYPE(ClawsMailUndo, claws_mail_undo, G_TYPE_OBJECT)

enum {
  PROP_0,

  PROP_MAXLEN
};

typedef enum {
  UNDO_ENTRY_GROUP_START,
  UNDO_ENTRY_GROUP_END,
  UNDO_ENTRY_DATA
} UndoEntryType;

typedef struct _UndoEntry UndoEntry;
struct _UndoEntry {
  UndoEntryType type;
  gchar *description;
  ClawsMailUndoSet *set;
  gpointer data;
};

static void claws_mail_undo_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
  ClawsMailUndo *self = CLAWS_MAIL_UNDO(object);

  switch(property_id) {

  case PROP_MAXLEN:
    g_value_set_int(value, self->maxlen);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

/* Free a undo entry itself and all members. */
static void undo_entry_free(ClawsMailUndo *undo, UndoEntry *entry)
{
  /* Call virutal functions for data entries */
  if(entry->type == UNDO_ENTRY_DATA) {
    g_return_if_fail(entry->set);
    if(entry->set->do_free)
      entry->set->do_free(entry->data);
  }
  g_free(entry->description);
  g_free(entry);
}

/* Change length of the undo stack */
static void undo_change_len_undo(ClawsMailUndo *undo, gint num)
{
  undo->len_undo = undo->len_undo + num;

  if((num > 0) && (undo->len_undo == 1))
    g_signal_emit(undo, CLAWS_MAIL_UNDO_GET_CLASS(undo)->signal_id_can_undo, 0, TRUE);
  else if((num < 0) && (undo->len_undo == 0))
    g_signal_emit(undo, CLAWS_MAIL_UNDO_GET_CLASS(undo)->signal_id_can_undo, 0, FALSE);
}

/* Change length of the redo stack */
static void undo_change_len_redo(ClawsMailUndo *undo, gint num)
{
  undo->len_redo = undo->len_redo + num;

  if((num > 0) && (undo->len_redo == 1))
    g_signal_emit(undo, CLAWS_MAIL_UNDO_GET_CLASS(undo)->signal_id_can_redo, 0, TRUE);
  else if((num < 0) && (undo->len_redo == 0))
    g_signal_emit(undo, CLAWS_MAIL_UNDO_GET_CLASS(undo)->signal_id_can_redo, 0, FALSE);
}

/* Free last element of undo stack (usually because the stack
 * grew beyond its maximum length). */
static void undo_entry_free_last(ClawsMailUndo *undo)
{
  GList *last;
  UndoEntry *entry;

  g_return_if_fail(undo->undo_stack != NULL);
  g_return_if_fail(undo->len_undo > 0);

  last = g_list_last(undo->undo_stack);
  if(!last)
    return;

  entry = last->data;

  /* Free a single entry */
  if(entry->type == UNDO_ENTRY_DATA) {
    undo_entry_free(undo, entry);
    undo->undo_stack = g_list_delete_link(undo->undo_stack, last);
  }
  /* Free a group */
  else if(entry->type == UNDO_ENTRY_GROUP_START) {
    GList *new_end;
    GList *walk = last->prev;
    walk ? (entry = (UndoEntry*)walk->data) : (entry = NULL);
    while(entry && entry->type == UNDO_ENTRY_DATA) {
      undo_entry_free(undo, entry);
      walk = walk->prev;
      walk ? (entry = (UndoEntry*)walk->data) : (entry = NULL);
    }

    if(!(entry && entry->type == UNDO_ENTRY_GROUP_END)) {
      g_warning("Didn't find group end entry corresponding to group "
        "start entry in undo stack\n");
      walk = walk->next;
    }

    /* Remove all elements from walk until last from the undo stack */
    new_end = walk->prev;
    new_end->next = NULL;
    walk->prev = NULL;
    g_list_free(walk);
  }

  else
    g_warning("Unexpected entry at the end of the undo list: %d\n", entry->type);

  undo_change_len_undo(undo, -1);
}

static void claws_mail_undo_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  ClawsMailUndo *self = CLAWS_MAIL_UNDO(object);

  if(self->group_add_mode) {
    g_warning("Currently in group add mode. Cannot set properties\n");
    return;
  }

  switch(property_id) {

  case PROP_MAXLEN:
  {
    gint new_maxlen;
    new_maxlen = g_value_get_int(value);
    if(new_maxlen != -1)
      while(self->len_undo > new_maxlen)
        undo_entry_free_last(self);
      self->maxlen = new_maxlen;
  }
  break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

/* Clear the undo stack */
static void undo_clear_undo(ClawsMailUndo *undo)
{
  GList *walk;

  undo_change_len_undo(undo, -undo->len_undo);

  for(walk = undo->undo_stack; walk; walk = g_list_next(walk)) {
    UndoEntry *entry = walk->data;
    if(entry)
      undo_entry_free(undo, entry);
  }
  g_list_free(undo->undo_stack);
  undo->undo_stack = NULL;
}

/* Clear the redo stack */
static void undo_clear_redo(ClawsMailUndo *undo)
{
  GList *walk;

  undo_change_len_redo(undo, -undo->len_redo);

  for(walk = undo->redo_stack; walk; walk = g_list_next(walk)) {
    UndoEntry *entry = walk->data;
    if(entry)
      undo_entry_free(undo, entry);
  }
  g_list_free(undo->redo_stack);
  undo->redo_stack = NULL;
}


void claws_mail_undo_clear(ClawsMailUndo *undo)
{
  g_return_if_fail(CLAWS_MAIL_IS_UNDO(undo));

  undo_clear_undo(undo);
  undo_clear_redo(undo);
}

static void claws_mail_undo_finalize(GObject *object)
{
  ClawsMailUndo *self = CLAWS_MAIL_UNDO(object);

  claws_mail_undo_clear(self);
  g_hash_table_destroy(self->method_hash);
  G_OBJECT_CLASS(claws_mail_undo_parent_class)->finalize (object);
}

static void claws_mail_undo_class_init(ClawsMailUndoClass *klass)
{
  GParamSpec *pspec;
  GType ptypes[1];
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->get_property = claws_mail_undo_get_property;
  object_class->set_property = claws_mail_undo_set_property;
  object_class->finalize = claws_mail_undo_finalize;

  /* Properties */
  pspec = g_param_spec_int("maxlen",
               "maximum length",
               "Maximum length of undo list.",
               -1,
               G_MAXINT,
               -1,
               G_PARAM_READWRITE);
  g_object_class_install_property(object_class, PROP_MAXLEN, pspec);

  /* Signals */
  ptypes[0] = G_TYPE_BOOLEAN;
  klass->signal_id_can_undo =
    g_signal_newv("can-undo",
          G_TYPE_FROM_CLASS(klass),
          G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
          NULL /* class closure */,
          NULL /* accumulator */,
          NULL /* accu_data */,
          g_cclosure_marshal_VOID__BOOLEAN,
          G_TYPE_NONE /* return_type */,
          1     /* n_params */,
          ptypes  /* param_types */);
  klass->signal_id_can_redo =
    g_signal_newv("can-redo",
          G_TYPE_FROM_CLASS(klass),
          G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
          NULL /* class closure */,
          NULL /* accumulator */,
          NULL /* accu_data */,
          g_cclosure_marshal_VOID__BOOLEAN,
          G_TYPE_NONE /* return_type */,
          1     /* n_params */,
          ptypes /* param_types */);
}

static void destroy_undoset(gpointer data)
{
  ClawsMailUndoSet *set = data;
  g_free(set->description);
  g_free(set);
}

static void claws_mail_undo_init(ClawsMailUndo *self)
{
  self->undo_stack = NULL;
  self->redo_stack = NULL;
  self->len_undo = 0;
  self->len_redo = 0;
  self->group_add_mode = FALSE;
  self->maxlen = -1;
  self->method_hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, destroy_undoset);
}


ClawsMailUndo * claws_mail_undo_new(void)
{
  return g_object_new(CLAWS_MAIL_TYPE_UNDO, NULL);
}

void claws_mail_undo_register_set(ClawsMailUndo *undo, const char *name, ClawsMailUndoSet *set)
{
  ClawsMailUndoSet *val;

  g_return_if_fail(CLAWS_MAIL_IS_UNDO(undo) && name && set);

  /* add the set to the method hash if it's not present yet */
  if(g_hash_table_lookup(undo->method_hash, name))
    g_print("A set with the name '%s' has already been registered\n", name);
  val = g_new0(ClawsMailUndoSet, 1);
  *val = *set;
  val->description = g_strdup(set->description);
  g_hash_table_insert(undo->method_hash, g_strdup(name), val);
}

void claws_mail_undo_set_maxlen(ClawsMailUndo *undo, gint maxlen)
{
  g_object_set(undo, "maxlen", (gint)maxlen, NULL);
}

gint claws_mail_undo_get_maxlen(ClawsMailUndo *undo)
{
  gint maxlen;
  g_object_get(undo, "maxlen", &maxlen, NULL);
  return maxlen;
}

void claws_mail_undo_add(ClawsMailUndo *undo, const char *set_name, gpointer data, gchar *description)
{
  ClawsMailUndoSet *set;
  UndoEntry *entry;

  if(undo->group_add_mode) {
    g_warning("Currently in group add mode. Cannot undo.\n");
    return;
  }

  g_return_if_fail(undo && set_name);

  if(undo->maxlen == 0)
    return;

  set = g_hash_table_lookup(undo->method_hash, set_name);
  g_return_if_fail(set);

  entry = g_new0(UndoEntry, 1);
  entry->type = UNDO_ENTRY_DATA;
  entry->description = g_strdup(description);
  entry->set = set;
  entry->data = data;

  undo->undo_stack = g_list_prepend(undo->undo_stack, entry);

  if(undo->group_add_mode == FALSE) {
    undo_change_len_undo(undo, 1);
    undo_clear_redo(undo);
    if((undo->maxlen != -1) && (undo->len_undo > undo->maxlen))
      undo_entry_free_last(undo);
  }
}

void claws_mail_undo_undo(ClawsMailUndo *undo)
{
  /* TODO */
  g_print("undo clicked\n");
}

void claws_mail_undo_redo(ClawsMailUndo *undo)
{
  /* TODO */
  g_print("redo clicked\n");
}

gboolean claws_mail_undo_can_undo(ClawsMailUndo *undo)
{
  g_return_val_if_fail(CLAWS_MAIL_IS_UNDO(undo), FALSE);

  if(undo->group_add_mode)
    return FALSE;

  return (undo->undo_stack != NULL);
}

gboolean claws_mail_undo_can_redo(ClawsMailUndo *undo)
{
  g_return_val_if_fail(CLAWS_MAIL_IS_UNDO(undo), FALSE);

  if(undo->group_add_mode)
    return FALSE;

  return (undo->redo_stack != NULL);
}

static GList* get_descriptions_from_stack(GList *stack)
{
  GList *list, *walk;

  list = NULL;
  for(walk = g_list_last(stack); walk; walk = walk->prev) {
    UndoEntry *entry;
    gchar *desc;
    entry = walk->data;
    if(entry->description)
      desc = entry->description;
    else if(entry->set->description)
      desc = entry->set->description;
    else
      desc = "<no description available>";
    list = g_list_prepend(list, desc);
  }
  return list;
}

/* The returned list must be freed, but not the list elements */
GList* claws_mail_undo_get_undo_descriptions(ClawsMailUndo *undo)
{
  return get_descriptions_from_stack(undo->undo_stack);
}

/* The returned list must be freed, but not the list elements */
GList* claws_mail_undo_get_redo_descriptions(ClawsMailUndo *undo)
{
  return get_descriptions_from_stack(undo->redo_stack);
}

void claws_mail_undo_print(ClawsMailUndo *undo)
{
  g_return_if_fail(CLAWS_MAIL_IS_UNDO(undo));
  claws_mail_undo_print_undo(undo);
  claws_mail_undo_print_redo(undo);
}

void claws_mail_undo_print_undo(ClawsMailUndo *undo)
{
  GList *walk;

  g_return_if_fail(CLAWS_MAIL_IS_UNDO(undo));
  g_print("\n=== Undo Stack: %d Elements ===\n", undo->len_undo);
  for(walk = undo->undo_stack; walk; walk = g_list_next(walk)) {
    UndoEntry *entry = walk->data;
    if(entry->type == UNDO_ENTRY_DATA) {
      gchar *description;
      if(entry->description)
        description = entry->description;
      else if(entry->set->description)
        description = entry->set->description;
      else
        description = "no description available";
      g_print("Data: %p: %s\n",entry->data, description);
    }
    else if(entry->type == UNDO_ENTRY_GROUP_START)
      g_print("Group start\n");
    else if(entry->type == UNDO_ENTRY_GROUP_END)
      g_print("Group end\n");
    else
      g_print("Unknown type: %d-%d\n", entry->type,UNDO_ENTRY_GROUP_START);
  }
  g_print("==============================\n\n");
}

void claws_mail_undo_print_redo(ClawsMailUndo *undo)
{
  GList *walk;

  g_return_if_fail(CLAWS_MAIL_IS_UNDO(undo));
  g_print("\n=== Redo Stack ===\n");
  for(walk = undo->redo_stack; walk; walk = g_list_next(walk)) {
    UndoEntry *entry = walk->data;
    if(entry->type == UNDO_ENTRY_DATA) {
      gchar *description;
      if(entry->description)
        description = entry->description;
      else if(entry->set->description)
        description = entry->set->description;
      else
        description = "no description available";
      g_print("Data: %p: %s\n",entry->data, description);
    }
    else if(entry->type == UNDO_ENTRY_GROUP_START)
      g_print("Group start\n");
    else if(entry->type == UNDO_ENTRY_GROUP_END)
      g_print("Group end\n");
    else
      g_print("Unknown type: %d-%d\n", entry->type,UNDO_ENTRY_GROUP_START);
  }
  g_print("==================\n\n");
}
