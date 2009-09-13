#include <gtk/gtk.h>

#include "claws_mail_undo.h"

#define UNDO_SET_NAME "tst1"

typedef struct _UndoDataTst1 UndoDataTst1;
struct _UndoDataTst1 {
};

void do_undo(gpointer data) {
  g_print("do undo called\n");
}

void do_redo(gpointer data) {
  g_print("do redo called\n");
}

void do_free(gpointer data) {
  g_print("do free called\n");
  g_free(data);
}

static void add_entry(ClawsMailUndo *undo)
{
  UndoDataTst1 *data;
  data = g_new0(UndoDataTst1, 1);
  claws_mail_undo_add(undo, UNDO_SET_NAME, data, NULL);
}

static void add_entry_with_description(ClawsMailUndo *undo)
{
  UndoDataTst1 *data;
  data = g_new0(UndoDataTst1, 1);
  claws_mail_undo_add(undo, UNDO_SET_NAME, data, "jo man!");
}

int main (int argc, char *argv[])
{
  ClawsMailUndo *undo;
  ClawsMailUndoSet set;

  gtk_init(&argc, &argv);

  undo = claws_mail_undo_new();
  claws_mail_undo_set_maxlen(undo, 2);

  set.do_undo = do_undo;
  set.do_redo = do_redo;
  set.do_free = do_free;
  set.description = "Set description";
  claws_mail_undo_register_set(undo, UNDO_SET_NAME, &set);

  claws_mail_undo_print(undo);
  add_entry(undo);
  add_entry_with_description(undo);
  add_entry(undo);
  claws_mail_undo_print(undo);

  g_object_unref(G_OBJECT(undo));
  return 0;
}
