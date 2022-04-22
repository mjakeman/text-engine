/* inspector.c
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "inspector.h"

#include "display.h"
#include "../model/document.h"
#include "../model/run.h"

struct _TextInspector
{
    GtkWidget parent_instance;

    GObject *object;
    TextFrame *frame;

    GtkWidget *vbox;
    GtkWidget *colview;
};

G_DEFINE_FINAL_TYPE (TextInspector, text_inspector, GTK_TYPE_WIDGET)

enum {
    PROP_0,
    PROP_TITLE,
    PROP_OBJECT,
    N_PROPS
};

static GParamSpec *properties [N_PROPS];

#define TITLE "Text Engine"

static void
bind_frame (TextInspector *inspector,
            TextFrame     *frame);

TextInspector *
text_inspector_new (void)
{
    return g_object_new (TEXT_TYPE_INSPECTOR, NULL);
}

static void
text_inspector_finalize (GObject *object)
{
    TextInspector *self = (TextInspector *)object;

    gtk_widget_unparent (self->vbox);

    G_OBJECT_CLASS (text_inspector_parent_class)->finalize (object);
}

static void
text_inspector_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
    TextInspector *self = TEXT_INSPECTOR (object);

    switch (prop_id)
    {
    case PROP_TITLE:
        g_value_set_string (value, TITLE);
        break;
    case PROP_OBJECT:
        g_value_set_object (value, self->object);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
text_inspector_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
    TextInspector *self = TEXT_INSPECTOR (object);

    switch (prop_id)
    {
    case PROP_OBJECT:
        self->object = g_value_get_object (value);

        if (TEXT_IS_DISPLAY (self->object))
        {
            TextDocument *document;

            g_object_get (self->object, "document", &document, NULL);

            if (TEXT_IS_FRAME (document->frame))
                bind_frame (self, document->frame);
        }
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

GListModel *
create_list_model (TextItem *item)
{
    GListStore *store;
    TextNode *node;

    if (TEXT_IS_RUN (item))
        return NULL;

    store = g_list_store_new (TEXT_TYPE_ITEM);

    for (node = text_node_get_first_child (TEXT_NODE (item));
        node != NULL;
        node = text_node_get_next (node))
    {
        g_assert (TEXT_IS_ITEM (node));

        g_list_store_append (store, node);
    }

    return G_LIST_MODEL (store);
}

static void
bind_frame (TextInspector *self,
            TextFrame     *frame)
{
    GtkTreeListModel *tree_model;
    GtkSingleSelection *selection_model;
    GListStore *root;

    g_return_if_fail (TEXT_IS_INSPECTOR (self));
    g_return_if_fail (TEXT_IS_FRAME (frame));

    root = g_list_store_new (TEXT_TYPE_ITEM);
    g_list_store_append (root, TEXT_ITEM (frame));

    tree_model = gtk_tree_list_model_new (G_LIST_MODEL (root), FALSE, TRUE,
                                          (GtkTreeListModelCreateModelFunc) create_list_model,
                                          NULL, NULL);


    selection_model = gtk_single_selection_new (G_LIST_MODEL (tree_model));

    gtk_column_view_set_model (GTK_COLUMN_VIEW (self->colview),
                               GTK_SELECTION_MODEL (selection_model));
}

static void
text_inspector_class_init (TextInspectorClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = text_inspector_finalize;
    object_class->get_property = text_inspector_get_property;
    object_class->set_property = text_inspector_set_property;

    properties [PROP_TITLE]
        = g_param_spec_string ("title",
                               "Title",
                               "Title",
                               TITLE,
                               G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    properties [PROP_OBJECT]
        = g_param_spec_object ("object",
                               "Object",
                               "Object",
                               G_TYPE_OBJECT,
                               G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);


    g_object_class_install_properties (object_class, N_PROPS, properties);

    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
}

void
type_setup (GtkSignalListItemFactory *self,
            GtkListItem              *listitem,
            gpointer                  user_data)
{
    GtkWidget *label;
    GtkWidget *expander;

    label = gtk_label_new ("");
    expander = gtk_tree_expander_new ();

    gtk_tree_expander_set_child (GTK_TREE_EXPANDER (expander), label);

    gtk_list_item_set_child (listitem, expander);
}

void
type_bind (GtkSignalListItemFactory *self,
           GtkListItem              *listitem,
           gpointer                  user_data)
{
    GtkWidget *label;
    GtkTreeExpander *expander;
    GtkTreeListRow *row;
    TextItem *item;
    const gchar *type;

    expander = GTK_TREE_EXPANDER (gtk_list_item_get_child (listitem));
    row = GTK_TREE_LIST_ROW (gtk_list_item_get_item (listitem));

    item = gtk_tree_list_row_get_item (row);

    g_assert (GTK_IS_TREE_EXPANDER (expander));
    g_assert (GTK_IS_TREE_LIST_ROW (row));
    g_assert (TEXT_IS_ITEM (item));

    gtk_tree_expander_set_list_row (expander, row);

    label = gtk_tree_expander_get_child (expander);
    type = g_type_name_from_instance ((GTypeInstance *)item);
    gtk_label_set_text (GTK_LABEL (label), type);
}

void
text_setup (GtkSignalListItemFactory *self,
            GtkListItem              *listitem,
            gpointer                  user_data)
{
    GtkWidget *label;

    label = gtk_label_new ("");
    gtk_label_set_xalign (GTK_LABEL (label), 0);
    gtk_label_set_single_line_mode (GTK_LABEL (label), TRUE);

    gtk_list_item_set_child (listitem, label);
}

void
text_bind (GtkSignalListItemFactory *self,
           GtkListItem              *listitem,
           gpointer                  user_data)
{
    GtkWidget *label;
    GtkTreeListRow *row;
    TextItem *item;
    const gchar *type;

    label = gtk_list_item_get_child (listitem);
    row = GTK_TREE_LIST_ROW (gtk_list_item_get_item (listitem));

    item = gtk_tree_list_row_get_item (row);

    g_assert (GTK_IS_TREE_LIST_ROW (row));
    g_assert (TEXT_IS_ITEM (item));

    if (TEXT_IS_RUN (item))
    {
        const gchar *text;

        g_object_get (item, "text", &text, NULL);
        gtk_label_set_text (GTK_LABEL (label), text);
    }
    else
    {
        gtk_label_set_text (GTK_LABEL (label), NULL);
    }
}

static GtkWidget *
setup_colview ()
{
    GtkWidget *colview;
    GtkColumnViewColumn *column;
    GtkListItemFactory *factory;

    colview = gtk_column_view_new (NULL);
    gtk_column_view_set_reorderable (GTK_COLUMN_VIEW (colview), FALSE);
    gtk_column_view_set_show_column_separators (GTK_COLUMN_VIEW (colview), TRUE);
    gtk_widget_set_vexpand (colview, TRUE);
    gtk_widget_add_css_class (colview, "data-table");

    factory = gtk_signal_list_item_factory_new ();
    g_signal_connect (factory, "setup", G_CALLBACK (type_setup), NULL);
    g_signal_connect (factory, "bind", G_CALLBACK (type_bind), NULL);

    column = gtk_column_view_column_new ("Type", factory);
    gtk_column_view_column_set_expand (column, FALSE);
    gtk_column_view_column_set_resizable (column, TRUE);
    gtk_column_view_append_column (GTK_COLUMN_VIEW (colview), column);

    factory = gtk_signal_list_item_factory_new ();
    g_signal_connect (factory, "setup", G_CALLBACK (text_setup), NULL);
    g_signal_connect (factory, "bind", G_CALLBACK (text_bind), NULL);

    column = gtk_column_view_column_new ("Contents", factory);
    gtk_column_view_column_set_expand (column, TRUE);
    gtk_column_view_column_set_resizable (column, TRUE);
    gtk_column_view_append_column (GTK_COLUMN_VIEW (colview), column);

    return colview;
}

static void
text_inspector_init (TextInspector *self)
{
    GtkWidget *infobar;
    GtkWidget *label;

    self->vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_parent (self->vbox, GTK_WIDGET (self));

    label = gtk_label_new ("Select a TextDisplay widget to view its document");
    gtk_label_set_xalign (GTK_LABEL (label), 0);

    infobar = gtk_info_bar_new ();
    gtk_info_bar_add_child (GTK_INFO_BAR (infobar), label);
    gtk_box_append (GTK_BOX (self->vbox), infobar);

    self->colview = setup_colview ();
    gtk_box_append (GTK_BOX (self->vbox), self->colview);
}
