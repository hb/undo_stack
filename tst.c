#include <gtk/gtk.h>

#include "claws_mail_undo.h"
#include "claws_mail_undo_view.h"

#define UNDO_SET_NAME_OK "tst1"
#define UNDO_SET_NAME_FAIL "tst2"

typedef struct _UndoDataTst1 UndoDataTst1;
struct _UndoDataTst1 {
};

gboolean do_undo_ok(gpointer data)
{
  g_print("do undo called\n");
  return TRUE;
}

gboolean do_redo_ok(gpointer data)
{
  g_print("do redo called\n");
  return TRUE;
}

gboolean do_undo_fail(gpointer data)
{
  g_print("do undo called\n");
  return FALSE;
}

gboolean do_redo_fail(gpointer data)
{
  g_print("do redo called\n");
  return FALSE;
}

void do_free(gpointer data) {
  g_print("do free called\n");
  g_free(data);
}

static void add_entry(ClawsMailUndo *undo)
{
  UndoDataTst1 *data;
  data = g_new0(UndoDataTst1, 1);
  claws_mail_undo_add(undo, UNDO_SET_NAME_OK, data, NULL);
}

static void add_entry_with_description(ClawsMailUndo *undo)
{
  UndoDataTst1 *data;
  data = g_new0(UndoDataTst1, 1);
  claws_mail_undo_add(undo, UNDO_SET_NAME_FAIL, data, "jo man!");
}

static void start_group_with_description(ClawsMailUndo *undo)
{
  claws_mail_undo_start_group(undo, "group description");
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
  gtk_widget_set_size_request(win, 600, 600);
  g_signal_connect(G_OBJECT(win), "destroy", G_CALLBACK(gtk_main_quit), NULL);

  vbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(vbox);
  gtk_container_add(GTK_CONTAINER(win), vbox);

  hbox = gtk_hbox_new(FALSE, 5);
  gtk_widget_show(hbox);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

  /* undo button */
  button = gtk_button_new_from_stock(GTK_STOCK_UNDO);
  gtk_widget_show(button);
  gtk_widget_set_sensitive(button, claws_mail_undo_can_undo(undo));
  g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK(claws_mail_undo_undo), undo);
  g_signal_connect_swapped(G_OBJECT(undo), "can-undo", G_CALLBACK(gtk_widget_set_sensitive), button);
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

  /* redo button */
  button = gtk_button_new_from_stock(GTK_STOCK_REDO);
  gtk_widget_show(button);
  gtk_widget_set_sensitive(button, claws_mail_undo_can_redo(undo));
  g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK(claws_mail_undo_redo), undo);
  g_signal_connect_swapped(G_OBJECT(undo), "can-redo", G_CALLBACK(gtk_widget_set_sensitive), button);
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

  /* add buttons */
  button = gtk_button_new_with_label("add");
  gtk_widget_show(button);
  g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK(add_entry), undo);
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
  button = gtk_button_new_with_label("add desc");
  gtk_widget_show(button);
  g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK(add_entry_with_description), undo);
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
  button = gtk_button_new_with_label("start group");
  gtk_widget_show(button);
  g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK(start_group_with_description), undo);
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
  button = gtk_button_new_with_label("end group");
  gtk_widget_show(button);
  g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK(claws_mail_undo_end_group), undo);
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

  sep = gtk_hseparator_new();
  gtk_widget_show(sep);
  gtk_box_pack_start(GTK_BOX(vbox), sep, FALSE, FALSE, 0);
 
  view = claws_mail_undo_view_new(undo);
  claws_mail_undo_view_show_clear_button(CLAWS_MAIL_UNDO_VIEW(view), TRUE);
  gtk_widget_show(view);
  gtk_box_pack_start(GTK_BOX(vbox), view, TRUE, TRUE, 0);

  gtk_widget_show(win);
  return win;
}

int main (int argc, char *argv[])
{
  ClawsMailUndo *undo;
  ClawsMailUndoSet set_ok, set_fail;

  gtk_init(&argc, &argv);

  undo = claws_mail_undo_new();
  claws_mail_undo_set_maxlen(undo, 10);

  set_ok.do_undo = do_undo_ok;
  set_ok.do_redo = do_redo_ok;
  set_ok.do_free = do_free;
  set_ok.description = "OK Set description";
  claws_mail_undo_register_set(undo, UNDO_SET_NAME_OK, &set_ok);

  set_fail.do_undo = do_undo_ok;
  set_fail.do_redo = do_redo_fail;
  set_fail.do_free = do_free;
  set_fail.description = "FAIL Set description";
  claws_mail_undo_register_set(undo, UNDO_SET_NAME_FAIL, &set_fail);

  add_entry(undo);
  add_entry_with_description(undo);
  add_entry(undo);

  create_main_window(undo);
  gtk_main();

  g_object_unref(G_OBJECT(undo));

  return 0;
}
