#include <gtk/gtk.h>

#include "claws_mail_undo.h"



int main (int argc, char *argv[])
{
  ClawsMailUndo *undo;

  gtk_init(&argc, &argv);

  undo = claws_mail_undo_new();

  claws_mail_undo_print(undo);

  g_object_unref(G_OBJECT(undo));
  return 0;
}
