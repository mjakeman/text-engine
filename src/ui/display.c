/* display.c
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

#include "display.h"

#include "../model/paragraph.h"
#include "../layout/layout.h"

struct _TextDisplay
{
    GtkWidget parent_instance;

    TextFrame *frame;
    TextLayout *layout;
    TextLayoutBox *layout_tree;

    GtkIMContext *context;

    int index;
    TextRun *run;
};

G_DEFINE_FINAL_TYPE (TextDisplay, text_display, GTK_TYPE_WIDGET)

enum {
    PROP_0,
    PROP_FRAME,
    N_PROPS
};

static GParamSpec *properties [N_PROPS];

/**
 * text_display_new:
 * @frame: The #TextFrame to display or %NULL
 *
 * Creates a new #TextDisplay widget which displays the rich text
 * document stored inside @frame.
 *
 * Returns: A new #TextDisplay widget
 */
TextDisplay *
text_display_new (TextFrame *frame)
{
    return g_object_new (TEXT_TYPE_DISPLAY,
                         "frame", frame,
                         NULL);
}

static void
text_display_finalize (GObject *object)
{
    TextDisplay *self = (TextDisplay *)object;

    G_OBJECT_CLASS (text_display_parent_class)->finalize (object);
}

static void
text_display_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
    TextDisplay *self = TEXT_DISPLAY (object);

    switch (prop_id)
    {
    case PROP_FRAME:
        g_value_set_object (value, self->frame);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
text_display_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
    TextDisplay *self = TEXT_DISPLAY (object);

    switch (prop_id)
    {
    case PROP_FRAME:
        g_clear_object (&self->layout_tree);
        self->frame = g_value_get_object (value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
layout_snapshot_recursive (GtkWidget     *widget,
                           TextLayoutBox *layout_box,
                           GtkSnapshot   *snapshot,
                           GdkRGBA       *fg_color,
                           int           *delta_height)
{
    int offset = 0;

    for (TextNode *node = text_node_get_first_child (TEXT_NODE (layout_box));
         node != NULL;
         node = text_node_get_next (node))
    {
        g_assert (TEXT_IS_LAYOUT_BOX (node));

        int delta_height;
        layout_snapshot_recursive (widget, node, snapshot, fg_color, &delta_height);
        offset += delta_height;
    }

    PangoLayout *layout = text_layout_box_get_pango_layout (layout_box);
    const TextDimensions *bbox = text_layout_box_get_bbox (layout_box);

    if (layout)
    {
        gtk_snapshot_save (snapshot);
        gtk_snapshot_translate (snapshot, &GRAPHENE_POINT_INIT (0, offset));
        gtk_snapshot_append_layout (snapshot, layout, fg_color);
        gtk_snapshot_restore (snapshot);

        offset = bbox->height;
    }

    gtk_snapshot_translate (snapshot, &GRAPHENE_POINT_INIT (0, bbox->height));

    *delta_height = bbox->height;
}

static void
text_display_snapshot (GtkWidget   *widget,
                       GtkSnapshot *snapshot)
{
    g_return_if_fail (TEXT_IS_DISPLAY (widget));

    TextDisplay *self = TEXT_DISPLAY (widget);

    if (!self->frame)
        return;

    if (!self->layout_tree)
        return;

    // TODO: Don't recreate this each time - do in size allocate instead?
    g_clear_object (&self->layout_tree);
    self->layout_tree = text_layout_build_layout_tree (self->layout,
                                                       gtk_widget_get_pango_context (GTK_WIDGET (self)),
                                                       self->frame,
                                                       gtk_widget_get_width (GTK_WIDGET (self)));

    GdkRGBA fg_color;
    gtk_style_context_get_color (gtk_widget_get_style_context (widget), &fg_color);

    // Display the layout tree
    int delta_height;
    layout_snapshot_recursive (widget, self->layout_tree, snapshot, &fg_color, &delta_height);
}

static GtkSizeRequestMode
text_display_get_request_mode (GtkWidget *widget)
{
    return GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}

static void
text_display_measure (GtkWidget      *widget,
                      GtkOrientation  orientation,
                      int             for_size,
                      int            *minimum,
                      int            *natural,
                      int            *minimum_baseline,
                      int            *natural_baseline)
{
    if (orientation == GTK_ORIENTATION_VERTICAL)
    {
        TextDisplay *self = TEXT_DISPLAY (widget);
        PangoContext *context = gtk_widget_get_pango_context (widget);

        g_clear_object (&self->layout_tree);
        self->layout_tree = text_layout_build_layout_tree (self->layout,
                                                           context,
                                                           self->frame,
                                                           for_size);

        *minimum = *natural = text_layout_box_get_bbox (self->layout_tree)->height;

        g_debug ("Height: %d\n", *minimum);
    }
    else if (orientation == GTK_ORIENTATION_HORIZONTAL)
    {
        GTK_WIDGET_CLASS (text_display_parent_class)->measure (widget,
                                                               orientation,
                                                               for_size,
                                                               minimum,
                                                               natural,
                                                               minimum_baseline,
                                                               natural_baseline);
    }
}

static void
text_display_class_init (TextDisplayClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = text_display_finalize;
    object_class->get_property = text_display_get_property;
    object_class->set_property = text_display_set_property;

    properties [PROP_FRAME]
        = g_param_spec_object ("frame",
                               "Frame",
                               "Frame",
                               TEXT_TYPE_FRAME,
                               G_PARAM_READWRITE|G_PARAM_CONSTRUCT);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    widget_class->snapshot = text_display_snapshot;
    widget_class->get_request_mode = text_display_get_request_mode;
    widget_class->measure = text_display_measure;
}

TextItem *
go_up (TextItem *item)
{
    TextNode *parent;
    TextNode *sibling;

    parent = text_node_get_parent (TEXT_NODE (item));

    if (parent && TEXT_IS_ITEM (parent))
    {
        sibling = text_node_get_next (parent);
        if (sibling && TEXT_IS_ITEM (sibling))
        {
            return TEXT_ITEM (sibling);
        }
        else
        {
            return go_up (TEXT_ITEM (parent));
        }
    }

    return NULL;
}

TextRun *
walk_until_next_run (TextItem *item)
{
    TextNode *child;
    TextNode *sibling;
    TextItem *parent;

    child = text_node_get_first_child (TEXT_NODE (item));

    if (child && TEXT_IS_ITEM (child)) {
        if (TEXT_IS_RUN (child)) {
            return TEXT_RUN (child);
        }

        return walk_until_next_run (TEXT_ITEM (child));
    }

    sibling = text_node_get_next (TEXT_NODE (item));

    if (sibling && TEXT_IS_ITEM (sibling)) {
        if (TEXT_IS_RUN (sibling)) {
            return TEXT_RUN (sibling);
        }

        return walk_until_next_run (TEXT_ITEM (sibling));
    }

    parent = go_up (item);

    if (parent) {
        if (TEXT_IS_RUN (parent)) {
            return TEXT_RUN (parent);
        }

        return walk_until_next_run (parent);
    }

    return NULL;
}

void
commit (GtkIMContext *context,
        gchar        *str,
        TextDisplay  *self)
{
    g_return_if_fail (TEXT_IS_DISPLAY (self));
    g_return_if_fail (GTK_IS_IM_CONTEXT (context));

    g_assert (context == self->context);

    if (!TEXT_IS_FRAME (self->frame))
        return;

    if (!TEXT_IS_RUN (self->run))
        self->run = walk_until_next_run (TEXT_ITEM (self->frame));

    char *text;
    g_object_get (self->run, "text", &text, NULL);
    g_object_set (self->run, "text", str, NULL);

    // Queue redraw for now
    // Later on, we should invalidate the model which
    // then bubbles up and invalidates the style
    // which then bubbles up and invalidates the layout
    // which then causes a partial redraw - simple right?
    gtk_widget_queue_draw (GTK_WIDGET (self));

    g_print ("commit: %s\n", str);
}

static void
text_display_init (TextDisplay *self)
{
    GtkEventController *controller;

    self->layout = text_layout_new ();

    self->context = gtk_im_context_simple_new ();
    gtk_im_context_set_client_widget (self->context, GTK_WIDGET (self));

    g_signal_connect (self->context, "commit", G_CALLBACK (commit), self);

    controller = gtk_event_controller_key_new ();
    gtk_event_controller_key_set_im_context (GTK_EVENT_CONTROLLER_KEY (controller), self->context);

    gtk_widget_set_focusable (GTK_WIDGET (self), TRUE);
    gtk_widget_add_controller (GTK_WIDGET (self), controller);
}
