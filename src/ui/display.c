/* display.c
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is dual-licensed under the terms of the Mozilla Public
 * License 2.0 and the Lesser General Public License 2.1 (or any
 * later version).
 *
 * SPDX-License-Identifier: MPL-2.0 OR LGPL-2.1-or-later
 */

#include "display.h"

#include "../model/mark.h"
#include "../model/paragraph.h"
#include "../layout/layout.h"
#include "../model/document.h"
#include "../editor/editor.h"

struct _TextDisplay
{
    GtkWidget parent_instance;

    TextDocument *document;
    TextEditor *editor;
    TextLayout *layout;
    TextLayoutBox *layout_tree;

    GtkIMContext *context;

    TextMark *cursor;
};

G_DEFINE_FINAL_TYPE (TextDisplay, text_display, GTK_TYPE_WIDGET)

enum {
    PROP_0,
    PROP_DOCUMENT,
    N_PROPS
};

static GParamSpec *properties [N_PROPS];

/**
 * text_display_new:
 * @document: The #TextDocument to display or %NULL
 *
 * Creates a new #TextDisplay widget which displays the rich text
 * document stored inside @document.
 *
 * Returns: A new #TextDisplay widget
 */
TextDisplay *
text_display_new (TextDocument *document)
{
    return g_object_new (TEXT_TYPE_DISPLAY,
                         "document", document,
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
    case PROP_DOCUMENT:
        g_value_set_object (value, self->document);
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
    case PROP_DOCUMENT:
        text_node_clear (&self->layout_tree);
        self->document = g_value_get_object (value);

        if (self->document)
        {
            if (self->editor)
                g_object_unref (self->editor);

            self->editor = text_editor_new (self->document);
            text_editor_move_first (self->editor, TEXT_EDITOR_CURSOR);
        }
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
        layout_snapshot_recursive (widget, TEXT_LAYOUT_BOX (node), snapshot, fg_color, &delta_height);
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

    const TextDimensions *cursor;
    if (gtk_widget_has_focus (widget) &&
        text_layout_box_get_cursor (layout_box, &cursor))
    {
        gtk_snapshot_append_color (snapshot, fg_color, &GRAPHENE_RECT_INIT (cursor->x, cursor->y, 1, cursor->height));
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

    if (!self->document)
        return;

    if (!self->layout_tree)
        return;

    // TODO: Don't recreate this each time - do in size allocate instead?
    text_node_clear (&self->layout_tree);
    self->layout_tree = text_layout_build_layout_tree (self->layout,
                                                       gtk_widget_get_pango_context (GTK_WIDGET (self)),
                                                       self->document->cursor,
                                                       self->document->frame,
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

        text_node_clear (&self->layout_tree);
        self->layout_tree = text_layout_build_layout_tree (self->layout,
                                                           context,
                                                           self->document->cursor,
                                                           self->document->frame,
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

    properties [PROP_DOCUMENT]
        = g_param_spec_object ("document",
                               "Document",
                               "Document",
                               TEXT_TYPE_DOCUMENT,
                               G_PARAM_READWRITE|G_PARAM_CONSTRUCT);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    widget_class->snapshot = text_display_snapshot;
    widget_class->get_request_mode = text_display_get_request_mode;
    widget_class->measure = text_display_measure;

    gtk_widget_class_set_css_name (widget_class, "textdisplay");
}

void
_unset_selection (TextDocument *doc)
{
    if (doc->selection)
        g_clear_pointer (&doc->selection, text_mark_free);
}

void
commit (GtkIMContext *context,
        gchar        *str,
        TextDisplay  *self)
{
    g_return_if_fail (TEXT_IS_DISPLAY (self));
    g_return_if_fail (GTK_IS_IM_CONTEXT (context));

    g_assert (context == self->context);

    if (!TEXT_IS_DOCUMENT (self->document))
        return;

    self->document->selection != NULL
        ? text_editor_replace (self->editor, TEXT_EDITOR_CURSOR, TEXT_EDITOR_SELECTION, str)
        : text_editor_insert (self->editor, TEXT_EDITOR_CURSOR, str);

    _unset_selection (self->document);

    // Queue redraw for now
    // Later on, we should invalidate the model which
    // then bubbles up and invalidates the style
    // which then bubbles up and invalidates the layout
    // which then causes a partial redraw - simple right?
    gtk_widget_queue_draw (GTK_WIDGET (self));

    g_print ("commit: %s\n", str);
}

gboolean
key_pressed (GtkEventControllerKey *controller,
             guint                  keyval,
             guint                  keycode,
             GdkModifierType        state,
             TextDisplay           *self)
{
    TextMark *cursor;
    TextMark *selection;

    gboolean ctrl_pressed;
    gboolean shift_pressed;

    cursor = self->document->cursor;
    selection = self->document->selection;

    ctrl_pressed = state & GDK_CONTROL_MASK;
    shift_pressed = state & GDK_SHIFT_MASK;

    // Setup selection
    if (shift_pressed && !selection)
    {
        g_print ("No selection!\n");
        selection = text_mark_copy (cursor);
        self->document->selection = selection;
    }
    else if (!shift_pressed && selection)
    {
        _unset_selection (self->document);
        selection = NULL;
    }

    // Handle Save
    if (ctrl_pressed && keyval == GDK_KEY_s)
    {
        gchar *text;
        GdkDisplay *display;
        GdkClipboard *clipboard;

        display = gdk_display_get_default ();
        clipboard = gdk_display_get_clipboard (display);

        text = text_editor_dump_plain_text (self->editor);
        g_print ("Saving to clipboard:\nSTART\n%s\nEND\n", text);

        // "Save" to clipboard for now
        gdk_clipboard_set_text (clipboard, text);
        g_free (text);
    }

    // Handle Home/End
    if (ctrl_pressed && keyval == GDK_KEY_Home)
    {
        text_editor_move_first (self->editor, shift_pressed ? TEXT_EDITOR_SELECTION : TEXT_EDITOR_CURSOR);
        goto handled;
    }

    if (ctrl_pressed && keyval == GDK_KEY_End)
    {
        text_editor_move_last (self->editor, shift_pressed ? TEXT_EDITOR_SELECTION : TEXT_EDITOR_CURSOR);
        goto handled;
    }

    // Handle directional movemenent
    // TODO: Can we draw cursors/selections on another layer?
    if (keyval == GDK_KEY_Left)
    {
        text_editor_move_left (self->editor, shift_pressed ? TEXT_EDITOR_SELECTION : TEXT_EDITOR_CURSOR, 1);
        goto handled;
    }

    if (keyval == GDK_KEY_Right)
    {
        text_editor_move_right (self->editor, shift_pressed ? TEXT_EDITOR_SELECTION : TEXT_EDITOR_CURSOR, 1);
        goto handled;
    }

    // Handle deletion
    if (keyval == GDK_KEY_Delete)
    {
        if (selection)
        {
            text_editor_replace (self->editor, TEXT_EDITOR_CURSOR, TEXT_EDITOR_SELECTION, "");
            _unset_selection (self->document);
        }
        else
            text_editor_delete (self->editor, TEXT_EDITOR_CURSOR, 1);

        goto handled;
    }

    if (keyval == GDK_KEY_BackSpace)
    {
        if (selection)
        {
            text_editor_replace (self->editor, TEXT_EDITOR_CURSOR, TEXT_EDITOR_SELECTION, "");
            _unset_selection (self->document);
        }
        else
            text_editor_delete (self->editor, TEXT_EDITOR_CURSOR, -1);

        goto handled;
    }

    if (keyval == GDK_KEY_Return)
    {
        text_editor_split (self->editor, TEXT_EDITOR_CURSOR);
        goto handled;
    }

    return FALSE;

handled:
    gtk_widget_queue_draw (GTK_WIDGET (self));
    return TRUE;
}

void
pointer_pressed (GtkGestureClick *gesture,
                 gint             n_press,
                 gdouble          x,
                 gdouble          y,
                 TextDisplay     *self)
{
    gtk_widget_grab_focus (GTK_WIDGET (self));
    gtk_widget_queue_draw (GTK_WIDGET (self));
}

static void
text_display_init (TextDisplay *self)
{
    GtkEventController *controller;
    GtkGesture *gesture;

    self->layout = text_layout_new ();

    self->context = gtk_im_context_simple_new ();
    gtk_im_context_set_client_widget (self->context, GTK_WIDGET (self));

    g_signal_connect (self->context, "commit", G_CALLBACK (commit), self);

    controller = gtk_event_controller_key_new ();
    gtk_event_controller_key_set_im_context (GTK_EVENT_CONTROLLER_KEY (controller), self->context);
    g_signal_connect (controller, "key-pressed", G_CALLBACK (key_pressed), self);
    gtk_widget_add_controller (GTK_WIDGET (self), controller);

    gesture = gtk_gesture_click_new ();
    g_signal_connect (gesture, "pressed", G_CALLBACK (pointer_pressed), self);
    gtk_widget_add_controller (GTK_WIDGET (self), GTK_EVENT_CONTROLLER (gesture));

    gtk_widget_set_focusable (GTK_WIDGET (self), TRUE);
}
