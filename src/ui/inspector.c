/* inspector.c
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is dual-licensed under the terms of the Mozilla Public
 * License 2.0 and the Lesser General Public License 2.1 (or any
 * later version).
 *
 * SPDX-License-Identifier: MPL-2.0 OR LGPL-2.1-or-later
 */

#include "inspector.h"

#include "display.h"
#include "../model/document.h"
#include "../model/run.h"
#include "../model/image.h"

struct _TextInspector
{
    GtkWidget parent_instance;

    GObject *object;
    TextDocument *document;

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

static void populate_data_from_frame (TextInspector *inspector);

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
            self->document = document;
            populate_data_from_frame (self);
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

    if (TEXT_IS_FRAGMENT (item))
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
populate_data_from_frame (TextInspector *self)
{
    GtkTreeListModel *tree_model;
    GtkSingleSelection *selection_model;
    GListStore *root;

    g_return_if_fail (TEXT_IS_INSPECTOR (self));
    g_return_if_fail (TEXT_IS_DOCUMENT (self->document));
    g_return_if_fail (TEXT_IS_FRAME (self->document->frame));

    root = g_list_store_new (TEXT_TYPE_ITEM);
    g_list_store_append (root, TEXT_ITEM (self->document->frame));

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
common_setup (GtkSignalListItemFactory *self,
              GtkListItem              *listitem,
              gpointer                  user_data)
{
    GtkWidget *label, *tag;
    GtkWidget *hbox;

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);

    tag = gtk_label_new ("");
    gtk_label_set_xalign (GTK_LABEL (tag), 0.5f);
    gtk_widget_add_css_class (tag, "inspector-tag");
    gtk_box_append (GTK_BOX (hbox), tag);
    gtk_widget_set_visible (tag, FALSE);

    label = gtk_label_new ("");
    gtk_label_set_xalign (GTK_LABEL (label), 0);
    gtk_label_set_single_line_mode (GTK_LABEL (label), TRUE);
    gtk_box_append (GTK_BOX (hbox), label);

    gtk_list_item_set_child (listitem, hbox);
}

void
text_bind (GtkSignalListItemFactory *self,
           GtkListItem              *listitem,
           gpointer                  user_data)
{
    GtkWidget *hbox, *tag, *label;
    GtkTreeListRow *row;
    TextItem *item;

    hbox = gtk_list_item_get_child (listitem);
    row = GTK_TREE_LIST_ROW (gtk_list_item_get_item (listitem));

    item = gtk_tree_list_row_get_item (row);

    tag = gtk_widget_get_first_child ( hbox);
    label = gtk_widget_get_next_sibling (tag);

    g_assert (GTK_IS_TREE_LIST_ROW (row));
    g_assert (TEXT_IS_ITEM (item));

    gtk_widget_set_visible (tag, FALSE);

    if (TEXT_IS_RUN (item))
    {
        const gchar *text;

        g_object_get (item, "text", &text, NULL);
        gtk_label_set_text (GTK_LABEL (label), text);
    }
    else if (TEXT_IS_IMAGE (item))
    {
        const gchar *src;

        g_object_get (item, "src", &src, NULL);
        gtk_label_set_text (GTK_LABEL (label), src);

        gtk_widget_set_visible (tag, TRUE);
        gtk_label_set_text (GTK_LABEL (tag), "image");
    }
    else
    {
        gtk_label_set_text (GTK_LABEL (label), NULL);
    }
}

void
style_bind (GtkSignalListItemFactory *self,
            GtkListItem              *listitem,
            gpointer                  user_data)
{
    GtkWidget *hbox, *tag, *label;
    GtkTreeListRow *row;
    TextItem *item;

    hbox = gtk_list_item_get_child (listitem);
    row = GTK_TREE_LIST_ROW (gtk_list_item_get_item (listitem));

    item = gtk_tree_list_row_get_item (row);

    tag = gtk_widget_get_first_child ( hbox);
    label = gtk_widget_get_next_sibling (tag);

    g_assert (GTK_IS_TREE_LIST_ROW (row));
    g_assert (TEXT_IS_ITEM (item));

    if (TEXT_IS_RUN (item))
    {
        GString *string;
        char *text;

        string = g_string_new ("");
        if (text_run_get_style_bold (TEXT_RUN (item)))
            string = g_string_append (string, "bold ");
        if (text_run_get_style_italic (TEXT_RUN (item)))
            string = g_string_append (string, "italic ");
        if (text_run_get_style_underline (TEXT_RUN (item)))
            string = g_string_append (string, "underline ");

        text = g_string_free (string, FALSE);
        gtk_label_set_text (GTK_LABEL (label), text);
        g_free (text);
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
    g_signal_connect (factory, "setup", G_CALLBACK (common_setup), NULL);
    g_signal_connect (factory, "bind", G_CALLBACK (text_bind), NULL);

    column = gtk_column_view_column_new ("Contents", factory);
    gtk_column_view_column_set_expand (column, TRUE);
    gtk_column_view_column_set_resizable (column, TRUE);
    gtk_column_view_append_column (GTK_COLUMN_VIEW (colview), column);

    factory = gtk_signal_list_item_factory_new ();
    g_signal_connect (factory, "setup", G_CALLBACK (common_setup), NULL);
    g_signal_connect (factory, "bind", G_CALLBACK (style_bind), NULL);

    column = gtk_column_view_column_new ("Style", factory);
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
    GtkWidget *scroll_area;

    self->vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_parent (self->vbox, GTK_WIDGET (self));

    label = gtk_label_new ("Select a TextDisplay widget to view its document");
    gtk_label_set_xalign (GTK_LABEL (label), 0);

    infobar = gtk_info_bar_new ();
    gtk_info_bar_add_child (GTK_INFO_BAR (infobar), label);
    gtk_info_bar_add_button (GTK_INFO_BAR (infobar), "Refresh Model", GTK_BUTTONS_OK);
    g_signal_connect_swapped (infobar, "response", G_CALLBACK (populate_data_from_frame), self);
    gtk_box_append (GTK_BOX (self->vbox), infobar);

    scroll_area = gtk_scrolled_window_new ();
    gtk_box_append (GTK_BOX (self->vbox), scroll_area);

    self->colview = setup_colview ();
    gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scroll_area), self->colview);
}
