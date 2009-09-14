#include <gtk/gtk.h>

#include "claws_mail_undo.h"
#include "claws_mail_undo_view.h"

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

GtkWidget* create_main_window(ClawsMailUndo *undo)
{
  GtkWidget *win;
  GtkWidget *vbox;
  GtkWidget *button;
  GtkWidget *hbox;
  GtkWidget *sep;
  GtkWidget *view;

  win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect(G_OBJECT(win), "destroy", G_CALLBACK(gtk_main_quit), NULL);

  vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(win), vbox);

  hbox = gtk_hbox_new(FALSE, 5);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

  /* undo button */
  button = gtk_button_new_from_stock(GTK_STOCK_UNDO);
  gtk_widget_set_sensitive(button, claws_mail_undo_can_undo(undo));
  g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK(claws_mail_undo_undo), undo);
  g_signal_connect_swapped(G_OBJECT(undo), "can-undo", G_CALLBACK(gtk_widget_set_sensitive), button);
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

  /* redo button */
  button = gtk_button_new_from_stock(GTK_STOCK_REDO);
  gtk_widget_set_sensitive(button, claws_mail_undo_can_redo(undo));
  g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK(claws_mail_undo_redo), undo);
  g_signal_connect_swapped(G_OBJECT(undo), "can-redo", G_CALLBACK(gtk_widget_set_sensitive), button);
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

  sep = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(vbox), sep, FALSE, FALSE, 0);
 
  view = claws_mail_undo_view_new(undo);
  gtk_box_pack_start(GTK_BOX(vbox), view, TRUE, TRUE, 0);

  gtk_widget_show_all(win);
  return win;
}

int main (int argc, char *argv[])
{
  ClawsMailUndo *undo;
  ClawsMailUndoSet set;

  gtk_init(&argc, &argv);

  undo = claws_mail_undo_new();
  claws_mail_undo_set_maxlen(undo, 10);

  set.do_undo = do_undo;
  set.do_redo = do_redo;
  set.do_free = do_free;
  set.description = "Set description";
  claws_mail_undo_register_set(undo, UNDO_SET_NAME, &set);

  add_entry(undo);
  add_entry_with_description(undo);
  add_entry(undo);

  create_main_window(undo);
  gtk_main();

  g_object_unref(G_OBJECT(undo));

  return 0;
}
