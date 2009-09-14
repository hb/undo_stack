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

#include "claws_mail_undo_view.h"

G_DEFINE_TYPE(ClawsMailUndoView, claws_mail_undo_view, GTK_TYPE_VBOX)

#define UNDO_VIEW_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), CLAWS_MAIL_TYPE_UNDO_VIEW, ClawsMailUndoViewPrivate))
struct _ClawsMailUndoViewPrivate
{
  ClawsMailUndo *undo;
  GtkWidget *undo_button;
  GtkWidget *redo_button;

  GtkWidget *undo_view;
  GtkWidget *redo_view;
};

enum {
  PROP_0,

  PROP_UNDO
};

#if 0
static gboolean update_view_traverse_node(GNode *node, gpointer data)
{
  GtkTreeIter iter;
  GtkTreeStore *store = data;

  gtk_tree_store_append(store, &iter, node->parent);
  gtk_tree_store_set(store, &iter, 0, node->data, -1);

  return FALSE;
}
#endif

static void update_view_add_node(GtkTreeStore *store, GNode *node, GtkTreeIter *parent_iter)
{
  GtkTreeIter iter;
  guint n_children, ii;

  gtk_tree_store_append(store, &iter, parent_iter);
  gtk_tree_store_set(store, &iter, 0, node->data, -1);

  n_children = g_node_n_children(node);
  for(ii = 0; ii < n_children; ii++) {
    GNode *child = g_node_nth_child(node, ii);
    update_view_add_node(store, child, &iter);
  }
}

static void update_view_with_stack(GtkWidget *view, GList *descriptions)
{
  GtkTreeStore *store;
  GList *walk;

  if(!view)
    return;
  store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(view)));
  g_return_if_fail(GTK_IS_TREE_STORE(store));

  gtk_tree_store_clear(store);

  for(walk = descriptions; walk; walk = walk->next) {
    update_view_add_node(store, walk->data, NULL);
    g_node_destroy(walk->data);
  }
  g_list_free(descriptions);
}

static void update_list_displays(ClawsMailUndoView *self)
{
  g_return_if_fail(self->priv->undo);

  update_view_with_stack(self->priv->undo_view, claws_mail_undo_get_undo_descriptions(self->priv->undo));
  update_view_with_stack(self->priv->redo_view, claws_mail_undo_get_redo_descriptions(self->priv->undo));
}

static void claws_mail_undo_view_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
  ClawsMailUndoView *self = CLAWS_MAIL_UNDO_VIEW(object);

  switch(property_id) {

  case PROP_UNDO:
    g_value_set_pointer(value, self->priv->undo);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void claws_mail_undo_view_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  ClawsMailUndoView *self = CLAWS_MAIL_UNDO_VIEW(object);

  switch(property_id) {

  case PROP_UNDO:
    /* construct-only property */
    if(self->priv->undo)
      g_object_unref(self->priv->undo);
    self->priv->undo = g_value_get_pointer(value);
    if(self->priv->undo) {
      g_object_ref(self->priv->undo);
      g_signal_connect_swapped(G_OBJECT(self->priv->undo_button), "clicked", G_CALLBACK(claws_mail_undo_undo), self->priv->undo);
      g_signal_connect_swapped(G_OBJECT(self->priv->undo), "can-undo", G_CALLBACK(gtk_widget_set_sensitive), self->priv->undo_button);
      gtk_widget_set_sensitive(GTK_WIDGET(self->priv->undo_button), claws_mail_undo_can_undo(self->priv->undo));
      g_signal_connect_swapped(G_OBJECT(self->priv->redo_button), "clicked", G_CALLBACK(claws_mail_undo_redo), self->priv->undo);
      g_signal_connect_swapped(G_OBJECT(self->priv->undo), "can-redo", G_CALLBACK(gtk_widget_set_sensitive), self->priv->redo_button);
      gtk_widget_set_sensitive(GTK_WIDGET(self->priv->redo_button), claws_mail_undo_can_redo(self->priv->undo));
      g_signal_connect_swapped(G_OBJECT(self->priv->undo), "changed", G_CALLBACK(update_list_displays), self);
      update_list_displays(self);
    }
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}


static void claws_mail_undo_view_dispose(GObject *object)
{
  ClawsMailUndoView *self = CLAWS_MAIL_UNDO_VIEW(object);
  
  if(self->priv->undo) {
    g_object_unref(self->priv->undo);
    self->priv->undo = NULL;
  }

  G_OBJECT_CLASS(claws_mail_undo_view_parent_class)->dispose(object);
}

static void claws_mail_undo_view_finalize(GObject *object)
{
  G_OBJECT_CLASS(claws_mail_undo_view_parent_class)->finalize (object);
}

static void claws_mail_undo_view_class_init(ClawsMailUndoViewClass *klass)
{
  GParamSpec *pspec;
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  g_type_class_add_private(klass, sizeof(ClawsMailUndoViewPrivate));

  object_class->get_property = claws_mail_undo_view_get_property;
  object_class->set_property = claws_mail_undo_view_set_property;
  object_class->dispose = claws_mail_undo_view_dispose;
  object_class->finalize = claws_mail_undo_view_finalize;

  pspec = g_param_spec_pointer("undo",                        
               "undoobject",
               "Pointer to ClawsMailUndo object",
               G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property(object_class, PROP_UNDO, pspec);
}

static GtkWidget* create_list_display(ClawsMailUndoView *self, gboolean undo_side)
{
  GtkWidget *button;
  GtkWidget *vbox;
  GtkTreeStore *model;
  GtkWidget *view;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkWidget *scrolledwin;
  gchar *title;
  GtkTreeSelection *selection;

  vbox = gtk_vbox_new(FALSE, 0);

  /* buttons */
  if(undo_side) {
    button = gtk_button_new_from_stock(GTK_STOCK_UNDO);
    self->priv->undo_button = button;
  }
  else {
    button = gtk_button_new_from_stock(GTK_STOCK_REDO);
    self->priv->redo_button = button;
  }
  gtk_widget_set_sensitive(button, FALSE);
  gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);

  /* scrolled window */
  scrolledwin = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwin), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start(GTK_BOX(vbox), scrolledwin, TRUE, TRUE, 0);

  model = gtk_tree_store_new(1, G_TYPE_STRING);
  view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
  gtk_tree_selection_set_mode(selection, GTK_SELECTION_NONE);
  renderer = gtk_cell_renderer_text_new();
  if(undo_side)
    title = "Undo stack";
  else
    title = "Redo stack";
  column = gtk_tree_view_column_new_with_attributes(title, renderer, "text", 0, NULL);
  gtk_tree_view_column_set_clickable(column, FALSE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  gtk_container_add(GTK_CONTAINER(scrolledwin), view);

  if(undo_side)
    self->priv->undo_view = view;
  else
    self->priv->redo_view = view;

  return vbox;
}

static void claws_mail_undo_view_init(ClawsMailUndoView *self)
{
  GtkWidget *paned;
  GtkWidget *list_display;

  self->priv = UNDO_VIEW_PRIVATE(self);

  self->priv->undo = NULL;
  self->priv->undo_button = NULL;
  self->priv->redo_button = NULL;
  self->priv->undo_view = NULL;
  self->priv->redo_view = NULL;

  paned = gtk_hpaned_new();

  /* first pane: undo list */
  list_display = create_list_display(self, TRUE);
  gtk_paned_add1(GTK_PANED(paned), list_display);

  /* second pane: redo list */
  list_display = create_list_display(self, FALSE);
  gtk_paned_add2(GTK_PANED(paned), list_display);

  gtk_box_pack_start(GTK_BOX(self), paned, TRUE, TRUE, 0);
  gtk_widget_show_all(paned);
}

GtkWidget* claws_mail_undo_view_new(ClawsMailUndo *undo)
{
  return g_object_new(CLAWS_MAIL_TYPE_UNDO_VIEW, "undo", undo, NULL);
}
