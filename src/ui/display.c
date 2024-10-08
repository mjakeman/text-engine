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
    TextNode *layout_tree;

    GtkIMContext *context;

    TextMark *cursor;

    // Margins
    int margin_start;
    int margin_end;
    int margin_top;
    int margin_bottom;

    // Scrollable
    GtkAdjustment *hadjustment;
    GtkAdjustment *vadjustment;
    GtkScrollablePolicy hscroll_policy;
    GtkScrollablePolicy vscroll_policy;
};

G_DEFINE_FINAL_TYPE_WITH_CODE (TextDisplay, text_display, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_SCROLLABLE, NULL))

enum {
    PROP_0,
    PROP_DOCUMENT,
    PROP_MARGIN_START,
    PROP_MARGIN_END,
    PROP_MARGIN_TOP,
    PROP_MARGIN_BOTTOM,
    N_PROPS,

    // Overridden Properties
    PROP_HADJUSTMENT,
    PROP_VADJUSTMENT,
    PROP_HSCROLL_POLICY,
    PROP_VSCROLL_POLICY
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

    case PROP_MARGIN_START:
        g_value_set_int (value, self->margin_start);
        break;

    case PROP_MARGIN_END:
        g_value_set_int (value, self->margin_end);
        break;

    case PROP_MARGIN_TOP:
        g_value_set_int (value, self->margin_top);
        break;

    case PROP_MARGIN_BOTTOM:
        g_value_set_int (value, self->margin_bottom);
        break;

    case PROP_HADJUSTMENT:
        g_value_set_object (value, self->hadjustment);
        break;

    case PROP_VADJUSTMENT:
        g_value_set_object (value, self->vadjustment);
        break;

    case PROP_HSCROLL_POLICY:
        g_value_set_enum (value, self->hscroll_policy);
        break;

    case PROP_VSCROLL_POLICY:
        g_value_set_enum (value, self->vscroll_policy);
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

    GtkAdjustment *adj;

    switch (prop_id)
    {
    case PROP_DOCUMENT:
        if (self->layout_tree)
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

    case PROP_MARGIN_START:
        self->margin_start = g_value_get_int (value);
        gtk_widget_queue_allocate (GTK_WIDGET (self));
        break;

    case PROP_MARGIN_END:
        self->margin_end = g_value_get_int (value);
        gtk_widget_queue_allocate (GTK_WIDGET (self));
        break;

    case PROP_MARGIN_TOP:
        self->margin_top = g_value_get_int (value);
        gtk_widget_queue_allocate (GTK_WIDGET (self));
        break;

    case PROP_MARGIN_BOTTOM:
        self->margin_bottom = g_value_get_int (value);
        gtk_widget_queue_allocate (GTK_WIDGET (self));
        break;

    case PROP_HADJUSTMENT:
        adj = g_value_get_object (value);
        if (adj)
        {
            self->hadjustment = g_object_ref_sink (adj);
            g_signal_connect_swapped (self->hadjustment, "value-changed", G_CALLBACK (gtk_widget_queue_draw), self);
        }
        gtk_widget_queue_allocate (GTK_WIDGET (self));
        break;

    case PROP_VADJUSTMENT:
        adj = g_value_get_object (value);
        if (adj)
        {
            self->vadjustment = g_object_ref_sink (adj);
            g_signal_connect_swapped (self->vadjustment, "value-changed", G_CALLBACK (gtk_widget_queue_draw), self);
        }
        gtk_widget_queue_allocate (GTK_WIDGET (self));
        break;

    case PROP_HSCROLL_POLICY:
        self->hscroll_policy = g_value_get_enum (value);
        gtk_widget_queue_allocate (GTK_WIDGET (self));
        break;

    case PROP_VSCROLL_POLICY:
        self->vscroll_policy = g_value_get_enum (value);
        gtk_widget_queue_allocate (GTK_WIDGET (self));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
_rebuild_layout_tree (TextDisplay *self, int width)
{
    g_info ("Rebuilding layout tree\n");

    if (self->layout_tree)
        text_node_clear (&self->layout_tree);

    self->layout_tree = TEXT_NODE (text_layout_build_layout_tree (self->layout,
                                                                  gtk_widget_get_pango_context (GTK_WIDGET (self)),
                                                                  self->document->frame,
                                                                  width));
}

static void
draw_box_recursive (GtkWidget     *widget,
                    TextLayoutBox *layout_box,
                    GtkSnapshot   *snapshot,
                    GdkRGBA       *fg_color,
                    int           *delta_height);

static void
draw_block (GtkWidget     *widget,
            TextLayoutBox *layout_box,
            GtkSnapshot   *snapshot,
            GdkRGBA       *fg_color,
            int           *delta_height)
{
    int offset = 0;
    TextItem *item;
    const TextDimensions *bbox;

    // Get bounding box
    bbox = text_layout_box_get_bbox (layout_box);

    // Draw children first
    gtk_snapshot_save (snapshot);
    for (TextNode *node = text_node_get_first_child (TEXT_NODE (layout_box));
         node != NULL;
         node = text_node_get_next (node))
    {
        g_assert (TEXT_IS_LAYOUT_BOX (node));

        int child_delta_height;

        draw_box_recursive(widget, TEXT_LAYOUT_BOX(node), snapshot, fg_color, &child_delta_height);
        offset += child_delta_height;
    }
    gtk_snapshot_restore (snapshot);

    // Draw Text (if applicable)
    if (TEXT_IS_LAYOUT_BLOCK (layout_box))
    {
        PangoLayout *layout;

        layout = text_layout_block_get_pango_layout (TEXT_LAYOUT_BLOCK (layout_box));

        if (layout)
        {
            gtk_snapshot_save (snapshot);
            gtk_snapshot_translate (snapshot, &GRAPHENE_POINT_INIT (0, offset));
            gtk_snapshot_append_layout (snapshot, layout, fg_color);
            // item = text_layout_box_get_item (layout_box);
            // draw_inline_elements (snapshot, layout, item, bbox->x, bbox->y);
            gtk_snapshot_restore (snapshot);
        }
    }

    gtk_snapshot_translate (snapshot, &GRAPHENE_POINT_INIT (0, bbox->height));

    *delta_height = (int) bbox->height;
}

static void
draw_inline (GtkWidget    *widget,
            TextLayoutBox *layout_box,
            GtkSnapshot   *snapshot,
            GdkRGBA       *fg_color)
{
    GdkRGBA placeholder;
    const TextDimensions *bbox;

    gdk_rgba_parse (&placeholder, "red");

    // Get bounding box
    bbox = text_layout_box_get_bbox (layout_box);

    // Draw bounding box as solid colour (for now)
    gtk_snapshot_save (snapshot);
    gtk_snapshot_append_color (snapshot, &placeholder,
                               &GRAPHENE_RECT_INIT (
                                       bbox->x,
                                       bbox->y,
                                       bbox->width,
                                       bbox->height));
    gtk_snapshot_restore (snapshot);
}

static void
draw_box_recursive (GtkWidget     *widget,
                    TextLayoutBox *layout_box,
                    GtkSnapshot   *snapshot,
                    GdkRGBA       *fg_color,
                    int           *delta_height)
{
    int offset = 0;
    TextItem *item;
    const TextDimensions *bbox;

    *delta_height = 0;

    // For block elements, draw content and children
    if (TEXT_IS_LAYOUT_BLOCK (layout_box))
        draw_block (widget, layout_box, snapshot, fg_color, delta_height);
    else if (TEXT_IS_LAYOUT_INLINE (layout_box))
        draw_inline (widget, layout_box, snapshot, fg_color);
}

static void
draw_cursor_snapshot (GtkSnapshot *snapshot,
                      TextMark *cursor,
                      GdkRGBA *color)
{
    TextLayoutBlock *block;
    TextParagraph *item;
    TextFragment *inline_item;
    int index;

    item = cursor->paragraph;
    index = cursor->index;

    inline_item = text_paragraph_get_item_at_index (item, index, NULL);
    block = TEXT_LAYOUT_BLOCK (text_item_get_attachment (TEXT_ITEM (item)));

    if (TEXT_IS_LAYOUT_BOX (block))
    {
        int x, y, height, width;
        const TextDimensions *bbox;
        PangoLayout *layout;

        bbox = text_layout_box_get_bbox (TEXT_LAYOUT_BOX (block));

        // if (TEXT_IS_RUN (inline_item))
        {
            layout = text_layout_block_get_pango_layout (block);

            PangoRectangle cursor_rect;
            pango_layout_index_to_pos (layout,
                                       index,
                                       &cursor_rect);

            // Hardcode width to 1
            x = cursor_rect.x / PANGO_SCALE;
            y = cursor_rect.y / PANGO_SCALE;
            height = cursor_rect.height / PANGO_SCALE;
            width = 1;
        }
        /*else if (inline_item != NULL)
        {
            // Treat object as opaque with start and end cursor positions
            // TODO: Make this far more robust e.g. for editable inlines
            // TODO: What even is an inline_box? Give this a proper name
            TextLayoutBox *inline_box;
            const TextDimensions *inline_bbox;
            inline_box = TEXT_LAYOUT_BOX (text_item_get_attachment (inline_item));
            inline_bbox = text_layout_box_get_bbox (inline_box);

            if (index == 0)
            {
                x = 0;
                y = 0;
                height = (int) inline_bbox->height;
                width = 1;
            }
            else
            {
                x = (int) inline_bbox->width;
                y = 0;
                height = (int) inline_bbox->height;
                width = 1;
            }
        }*/

        gtk_snapshot_append_color (snapshot, color, &GRAPHENE_RECT_INIT (bbox->x + x, bbox->y + y, width, height));
    }
}

static void
draw_selection_partial_layout_snapshot (GtkSnapshot *snapshot,
                                        PangoLayout *layout,
                                        int          start_index,
                                        int          end_index,
                                        GdkRGBA     *color)
{
    PangoLayoutIter *iter;
    gboolean iter_next;

    iter = pango_layout_get_iter (layout);
    iter_next = TRUE;

    // Make sure start index is actually smaller than the end index
    if (start_index > end_index) {
        int tmp;
        tmp = start_index;
        start_index = end_index;
        end_index = tmp;
    }

    while (iter_next)
    {
        PangoRectangle rect;
        PangoLayoutLine *line;
        int line_start_index;
        int line_end_index;

        // Get line indices
        line = pango_layout_iter_get_line_readonly (iter);
        line_start_index = pango_layout_line_get_start_index (line);
        line_end_index = line_start_index + pango_layout_line_get_length (line);

        // Get extents of current line
        pango_layout_iter_get_line_extents (iter, &rect, NULL);

        // Advance iter to next line
        iter_next = pango_layout_iter_next_line (iter);

        // Skip lines outside the range
        if (line_end_index < start_index || line_start_index > end_index)
            continue;

        // Handle case where start and end are within the same line
        if (start_index >= line_start_index &&
            start_index <= line_end_index &&
            end_index >= line_start_index &&
            end_index <= line_end_index)
        {
            int selection_start_x;
            int selection_end_x;
            int width;

            pango_layout_line_index_to_x (line, start_index, FALSE, &selection_start_x);
            pango_layout_line_index_to_x (line, end_index, FALSE, &selection_end_x);

            width = selection_end_x - selection_start_x;

            // Draw selection box for current line
            gtk_snapshot_append_color (snapshot, color,&GRAPHENE_RECT_INIT (
                    (float) (rect.x + selection_start_x) / PANGO_SCALE,
                    (float) rect.y / PANGO_SCALE,
                    (float) width / PANGO_SCALE,
                    (float) rect.height / PANGO_SCALE));

            return;
        }

        // Handle starting line
        if (start_index >= line_start_index && start_index <= line_end_index)
        {
            int selection_x;
            int line_end_x;
            int width;

            // Get the horizontal starting point of the selection and the ending
            // index of the line and draw between them. We need to do this to
            // support both left-to-right and right-to-left languages.
            pango_layout_line_index_to_x (line, start_index, FALSE, &selection_x);
            pango_layout_line_index_to_x (line, line_end_index, FALSE, &line_end_x);

            width = selection_x - line_end_x;

            // Draw selection box for current line
            gtk_snapshot_append_color (snapshot, color,&GRAPHENE_RECT_INIT (
                    (float) (rect.x + line_end_x) / PANGO_SCALE,
                    (float) rect.y / PANGO_SCALE,
                    (float) width / PANGO_SCALE,
                    (float) rect.height / PANGO_SCALE));

            continue;
        }

        // Handle ending line
        if (end_index >= line_start_index && end_index <= line_end_index)
        {
            int selection_x;
            int line_start_x;
            int width;

            // Get the horizontal ending point of the selection and the starting
            // index of the line and draw between them. We need to do this to
            // support both left-to-right and right-to-left languages.
            pango_layout_line_index_to_x (line, end_index, FALSE, &selection_x);
            pango_layout_line_index_to_x (line, line_start_index, FALSE, &line_start_x);

            width = line_start_x - selection_x;

            // Draw selection box for current line
            gtk_snapshot_append_color (snapshot, color,&GRAPHENE_RECT_INIT (
                    (float) (rect.x + selection_x) / PANGO_SCALE,
                    (float) rect.y / PANGO_SCALE,
                    (float) width / PANGO_SCALE,
                    (float) rect.height / PANGO_SCALE));

            continue;
        }

        // Draw selection box for current line
        gtk_snapshot_append_color (snapshot, color,&GRAPHENE_RECT_INIT (
                (float) rect.x / PANGO_SCALE,
                (float) rect.y / PANGO_SCALE,
                (float) rect.width / PANGO_SCALE,
                (float) rect.height / PANGO_SCALE));
    }
}

static void
draw_selection_layout_snapshot (GtkSnapshot *snapshot,
                                PangoLayout *layout,
                                GdkRGBA     *color)
{
    PangoLayoutIter *iter;
    gboolean iter_next;

    iter = pango_layout_get_iter (layout);
    iter_next = TRUE;

    while (iter_next)
    {
        PangoRectangle rect;

        // Get extents of current line
        pango_layout_iter_get_line_extents (iter, &rect, NULL);

        // Advance iter to next line
        iter_next = pango_layout_iter_next_line (iter);

        // Draw selection box for current line
        gtk_snapshot_append_color (snapshot, color,&GRAPHENE_RECT_INIT (
                (float) rect.x / PANGO_SCALE,
                (float) rect.y / PANGO_SCALE,
                (float) rect.width / PANGO_SCALE,
                (float) rect.height / PANGO_SCALE));
    }
}

static void
draw_selection_snapshot (GtkSnapshot *snapshot,
                         GdkRGBA     *selection_color,
                         TextMark    *cursor,
                         TextMark    *selection)
{
    TextLayoutBlock *layout;
    TextParagraph *current;
    const TextDimensions *bbox;
    gboolean draw_selection;

    // Check if cursor and selection are within the same paragraph
    if (cursor->paragraph == selection->paragraph)
    {
        layout = TEXT_LAYOUT_BLOCK (text_item_get_attachment (TEXT_ITEM (cursor->paragraph)));
        bbox = text_layout_box_get_bbox (TEXT_LAYOUT_BOX (layout));

        gtk_snapshot_translate (snapshot, &GRAPHENE_POINT_INIT (0, bbox->y));
        draw_selection_partial_layout_snapshot (snapshot, text_layout_block_get_pango_layout (layout),
                                                cursor->index, selection->index,
                                                selection_color);

        return;
    }

    // Ensure cursor comes before selection in order. The actual meaning
    // doesn't matter as long as cursor comes first.
    text_editor_sort_marks (cursor, selection, &cursor, &selection);

    // Iterate over all paragraphs between the cursor and selection marks
    draw_selection = TRUE;
    current = cursor->paragraph;

    while (draw_selection)
    {
        if (!current)
            break;

        layout = TEXT_LAYOUT_BLOCK (text_item_get_attachment (TEXT_ITEM (current)));
        bbox = text_layout_box_get_bbox (TEXT_LAYOUT_BOX (layout));

        gtk_snapshot_save (snapshot);
        gtk_snapshot_translate (snapshot, &GRAPHENE_POINT_INIT (0, bbox->y));

        if (current == cursor->paragraph)
        {
            // Draw partial start segment
            draw_selection_partial_layout_snapshot (snapshot, text_layout_block_get_pango_layout (layout),
                                                    cursor->index,
                                                    text_paragraph_get_size_bytes (cursor->paragraph),
                                                    selection_color);
        }
        else if (current == selection->paragraph)
        {
            // Draw partial end segment
            draw_selection_partial_layout_snapshot (snapshot, text_layout_block_get_pango_layout (layout),
                                                    0,
                                                    selection->index,
                                                    selection_color);

            // Finished drawing, break out of loop
            draw_selection = FALSE;
        }
        else
        {
            // Draw full segment
            draw_selection_layout_snapshot (snapshot,
                                            text_layout_block_get_pango_layout (layout),
                                            selection_color);
        }

        gtk_snapshot_restore (snapshot);

        current = text_editor_next_paragraph (current);
    }
}

static void
text_display_snapshot (GtkWidget   *widget,
                       GtkSnapshot *snapshot)
{
    double displacement;
    int delta_height;
    TextDisplay *self;
    GtkStyleContext *context;
    GdkRGBA fg_color;
    GdkRGBA selection_color;
    GdkRGBA unfocused_selection_color;

    g_return_if_fail (TEXT_IS_DISPLAY (widget));

    self = TEXT_DISPLAY (widget);

    if (!self->document)
        return;

    if (!self->layout_tree)
        return;

    // Get default colours
    context = gtk_widget_get_style_context (widget);
    gtk_style_context_get_color (context, &fg_color);
    gtk_style_context_lookup_color (context, "theme_selected_bg_color", &selection_color);
    gtk_style_context_lookup_color (context, "theme_unfocused_selected_bg_color", &unfocused_selection_color);
    selection_color.alpha = 0.3f;
    unfocused_selection_color.alpha = 0.3f;

    // Set vertical displacement (horizontal not supported)
    displacement = self->vadjustment
                   ? -gtk_adjustment_get_value (self->vadjustment)
                   : 0;

    gtk_snapshot_translate (snapshot, &GRAPHENE_POINT_INIT (self->margin_start, self->margin_top + displacement));

    // Draw selection
    if (self->document->selection) {
        gtk_snapshot_save (snapshot);
        draw_selection_snapshot (snapshot,
                                 &selection_color,
                                 self->document->cursor,
                                 self->document->selection);
        gtk_snapshot_restore (snapshot);
    }

    // Draw layout tree
    gtk_snapshot_save (snapshot);
    draw_box_recursive (widget, TEXT_LAYOUT_BOX (self->layout_tree), snapshot, &fg_color, &delta_height);
    gtk_snapshot_restore (snapshot);

    // Draw cursors
    if (gtk_widget_has_focus (widget)) {
        gtk_snapshot_save (snapshot);
        draw_cursor_snapshot (snapshot, self->document->cursor, &fg_color);
        gtk_snapshot_restore (snapshot);
    }
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

        // Account for start/end margins
        for_size -= self->margin_start + self->margin_end;

        if (self->layout_tree)
            text_node_clear (&self->layout_tree);

        self->layout_tree = TEXT_NODE (text_layout_build_layout_tree (self->layout,
                                                                      context,
                                                                      self->document->frame,
                                                                      for_size));

        *minimum = *natural = text_layout_box_get_bbox (TEXT_LAYOUT_BOX (self->layout_tree))->height;

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

void
text_display_size_allocate (GtkWidget *widget,
                            int        widget_width,
                            int        widget_height,
                            int        baseline)
{
    TextDisplay *self;
    const TextDimensions *bbox;
    int cur_value;
    int content_height;
    int content_width;

    self = TEXT_DISPLAY (widget);

    _rebuild_layout_tree (self, widget_width - self->margin_start - self->margin_end);

    bbox = text_layout_box_get_bbox (TEXT_LAYOUT_BOX (self->layout_tree));

    content_height = bbox->height + self->margin_top + self->margin_bottom;
    content_height = MAX (content_height, widget_height);

    content_width = bbox->width + self->margin_start + self->margin_end;
    content_width = MAX (content_width, widget_width);

    if (self->vadjustment)
    {
        cur_value = gtk_adjustment_get_value (self->vadjustment);

        // only emit notify once for the whole block
        g_object_freeze_notify (G_OBJECT (self->vadjustment));

        gtk_adjustment_set_value (self->vadjustment, cur_value);
        gtk_adjustment_set_upper (self->vadjustment, content_height);
        gtk_adjustment_set_step_increment (self->vadjustment, widget_height * 0.1);
        gtk_adjustment_set_page_increment (self->vadjustment, widget_height * 0.9);
        gtk_adjustment_set_page_size (self->vadjustment, widget_height);

        g_object_thaw_notify (G_OBJECT (self->vadjustment));
    }

    if (self->hadjustment)
    {
        cur_value = gtk_adjustment_get_value (self->hadjustment);

        // only emit notify once for the whole block
        g_object_freeze_notify (G_OBJECT (self->hadjustment));

        gtk_adjustment_set_value (self->hadjustment, cur_value);
        gtk_adjustment_set_upper (self->hadjustment, content_width);
        gtk_adjustment_set_step_increment (self->hadjustment, widget_width * 0.1);
        gtk_adjustment_set_page_increment (self->hadjustment, widget_width * 0.9);
        gtk_adjustment_set_page_size (self->hadjustment, widget_width);

        g_object_thaw_notify (G_OBJECT (self->hadjustment));
    }
}

static void
text_display_class_init (TextDisplayClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = text_display_finalize;
    object_class->get_property = text_display_get_property;
    object_class->set_property = text_display_set_property;

    g_object_class_override_property (object_class, PROP_HADJUSTMENT, "hadjustment");
    g_object_class_override_property (object_class, PROP_VADJUSTMENT, "vadjustment");
    g_object_class_override_property (object_class, PROP_HSCROLL_POLICY, "hscroll-policy");
    g_object_class_override_property (object_class, PROP_VSCROLL_POLICY, "vscroll-policy");

    properties [PROP_DOCUMENT]
        = g_param_spec_object ("document",
                               "Document",
                               "Document",
                               TEXT_TYPE_DOCUMENT,
                               G_PARAM_READWRITE|G_PARAM_CONSTRUCT);

    properties [PROP_MARGIN_START]
        = g_param_spec_int ("margin-start",
                            "Margin Start",
                            "Margin Start",
                            0, G_MAXINT, 0,
                            G_PARAM_READWRITE|G_PARAM_CONSTRUCT);

    properties [PROP_MARGIN_END]
        = g_param_spec_int ("margin-end",
                            "Margin End",
                            "Margin End",
                            0, G_MAXINT, 0,
                            G_PARAM_READWRITE|G_PARAM_CONSTRUCT);

    properties [PROP_MARGIN_TOP]
        = g_param_spec_int ("margin-top",
                            "Margin Top",
                            "Margin Top",
                            0, G_MAXINT, 0,
                            G_PARAM_READWRITE|G_PARAM_CONSTRUCT);

    properties [PROP_MARGIN_BOTTOM]
        = g_param_spec_int ("margin-bottom",
                            "Margin Bottom",
                            "Margin Bottom",
                            0, G_MAXINT, 0,
                            G_PARAM_READWRITE|G_PARAM_CONSTRUCT);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    widget_class->snapshot = text_display_snapshot;
    widget_class->get_request_mode = text_display_get_request_mode;
    widget_class->measure = text_display_measure;
    widget_class->size_allocate = text_display_size_allocate;

    gtk_widget_class_set_css_name (widget_class, "textdisplay");
}

TextMark *
_set_selection (TextDocument *doc)
{
    TextMark *selection;

    selection = text_mark_copy (doc->cursor);
    doc->selection = selection;

    return selection;
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
        : text_editor_insert_text(self->editor, TEXT_EDITOR_CURSOR, str);

    _unset_selection (self->document);

    // Queue redraw for now
    // Later on, we should invalidate the model which
    // then bubbles up and invalidates the style
    // which then bubbles up and invalidates the layout
    // which then causes a partial redraw - simple right?
    gtk_widget_queue_allocate (GTK_WIDGET (self));
    gtk_widget_queue_draw (GTK_WIDGET (self));

    g_info ("commit: %s\n", str);
}

static gboolean
_move_cursor_home (TextMark *cursor)
{
    TextParagraph *para;
    TextLayoutBlock *layout;
    int index;

    index = cursor->index;
    para = cursor->paragraph;
    layout = TEXT_LAYOUT_BLOCK (text_item_get_attachment (TEXT_ITEM (para)));

    if (layout) {
        PangoLayout *pango;
        GSList *iter;
        pango = text_layout_block_get_pango_layout (layout);
        int base_index = 0;

        for (iter = pango_layout_get_lines (pango);
             iter != NULL;
             iter = iter->next)
        {
            PangoLayoutLine *line;
            gboolean is_last_line;

            is_last_line = (iter->next == NULL);
            line = iter->data;

            // For the last line in the paragraph, there is an imaginary 'paragraph break'
            // character to account for the traversal between paragraphs. Therefore we check
            // whether index is contained within the 'length + 1' of the last line.
            if (is_last_line && base_index + pango_layout_line_get_length (line) + 1 > index) {
                cursor->index = base_index;
                return TRUE;
            }

            // Otherwise check if the index is contained within the line length, then
            // go to the starting index.
            if (base_index + pango_layout_line_get_length (line) > index) {
                cursor->index = base_index;
                return TRUE;
            }

            // Fine the base index of the next line
            base_index += pango_layout_line_get_length (line);
        }
    }

    return FALSE;
}

static gboolean
_move_cursor_end (TextMark *cursor)
{
    TextParagraph *para;
    TextLayoutBlock *layout;
    int index;

    index = cursor->index;
    para = cursor->paragraph;
    layout = TEXT_LAYOUT_BLOCK (text_item_get_attachment (TEXT_ITEM (para)));

    if (layout) {
        PangoLayout *pango;
        GSList *iter;
        pango = text_layout_block_get_pango_layout (layout);
        iter = pango_layout_get_lines (pango);
        int base_index = 0;

        for (iter = pango_layout_get_lines (pango);
             iter != NULL;
             iter = iter->next)
        {
            PangoLayoutLine *line;
            gboolean is_last_line;

            line = iter->data;
            is_last_line = (iter->next == NULL);

            // For the last line in the paragraph, there is an imaginary 'paragraph break'
            // character to account for the traversal between paragraphs. Therefore we check
            // whether index is contained within the 'length + 1' of the last line.
            if (is_last_line && base_index + pango_layout_line_get_length (line) + 1 > index) {
                cursor->index = base_index + pango_layout_line_get_length (line);
                return TRUE;
            }

            // Otherwise check if the index is contained within the line length, then
            // go to the index before the final character on the line.
            if (base_index + pango_layout_line_get_length (line) > index) {
                cursor->index = base_index + pango_layout_line_get_length (line) - 1;
                return TRUE;
            }

            base_index += pango_layout_line_get_length (line);
        }
    }

    return FALSE;
}

static gboolean
_move_cursor_vertically (TextMark *cursor,
                         gboolean  up)
{
    TextParagraph *para;
    TextLayoutBlock *block_layout;
    PangoLayout *pango_layout;
    PangoLayoutLine *line;
    PangoRectangle position;
    int cur_line_index;
    int index;
    int x_pos;

    index = cursor->index;
    para = cursor->paragraph;
    block_layout = TEXT_LAYOUT_BLOCK (text_item_get_attachment (TEXT_ITEM (para)));
    pango_layout = text_layout_block_get_pango_layout(block_layout);

    // First try move within the paragraph
    pango_layout_index_to_line_x (pango_layout, index, FALSE, &cur_line_index, &x_pos);

    // Try get line +/- 1
    line = pango_layout_get_line (pango_layout, up ? cur_line_index - 1 : cur_line_index + 1);

    if (line) {
        pango_layout_line_x_to_index(line, x_pos, &index, FALSE);
        cursor->index = index;
        return TRUE;
    }

    // If there are no more lines left, move to above or below box_layout
    pango_layout_index_to_pos(pango_layout, index, &position);

    block_layout = up
        ? TEXT_LAYOUT_BLOCK (text_layout_find_above (TEXT_LAYOUT_BOX (block_layout)))
        : TEXT_LAYOUT_BLOCK (text_layout_find_below (TEXT_LAYOUT_BOX (block_layout)));

    if (!block_layout)
        return FALSE;

    pango_layout = text_layout_block_get_pango_layout (block_layout);
    line = pango_layout_get_line(pango_layout, up ? pango_layout_get_line_count (pango_layout) - 1 : 0);
    pango_layout_line_x_to_index (line, position.x, &index, NULL);

    para = TEXT_PARAGRAPH (text_layout_box_get_item (TEXT_LAYOUT_BOX (block_layout)));

    cursor->index = index;
    cursor->paragraph = para;

    return TRUE;
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
        selection = _set_selection (self->document);
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
        g_info ("Saving to clipboard:\nSTART\n%s\nEND\n", text);

        // "Save" to clipboard for now
        gdk_clipboard_set_text (clipboard, text);
        g_free (text);
        return TRUE;
    }

    // Handle Home/End
    if (keyval == GDK_KEY_Home)
    {
        if (!shift_pressed && selection)
            _unset_selection (self->document);

        if (ctrl_pressed) {
            text_editor_move_first (self->editor, TEXT_EDITOR_CURSOR);
            goto redraw;
        }
        else if (_move_cursor_home (self->document->cursor)) {
            goto redraw;
        }

        return TRUE;
    }

    if (keyval == GDK_KEY_End)
    {
        if (!shift_pressed && selection)
            _unset_selection (self->document);

        if (ctrl_pressed) {
            text_editor_move_last (self->editor, TEXT_EDITOR_CURSOR);
        }
        else if (_move_cursor_end (self->document->cursor)) {
            goto redraw;
        }

        return TRUE;
    }

    // Handle directional movemenent
    if (keyval == GDK_KEY_Left)
    {
        if (!shift_pressed && selection)
            _unset_selection (self->document);

        text_editor_move_left (self->editor, TEXT_EDITOR_CURSOR, 1);
        goto redraw;
    }

    if (keyval == GDK_KEY_Right)
    {
        if (!shift_pressed && selection)
            _unset_selection (self->document);

        text_editor_move_right (self->editor, TEXT_EDITOR_CURSOR, 1);
        goto redraw;
    }

    if (keyval == GDK_KEY_Up)
    {
        if (!shift_pressed && selection)
            _unset_selection (self->document);

        if (_move_cursor_vertically (self->document->cursor, TRUE))
            goto redraw;
        return TRUE;
    }

    if (keyval == GDK_KEY_Down)
    {
        if (!shift_pressed && selection)
            _unset_selection (self->document);

        if (_move_cursor_vertically (self->document->cursor, FALSE))
            goto redraw;
        return TRUE;
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

        goto reallocate;
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

        goto reallocate;
    }

    if (keyval == GDK_KEY_Return)
    {
        if (selection)
        {
            text_editor_replace (self->editor, TEXT_EDITOR_CURSOR, TEXT_EDITOR_SELECTION, "");
            _unset_selection (self->document);
        }

        text_editor_split (self->editor, TEXT_EDITOR_CURSOR);
        goto reallocate;
    }

    // Select all
    if (keyval == GDK_KEY_a && ctrl_pressed)
    {
        if (!selection)
            _set_selection (self->document);

        text_editor_move_first (self->editor, TEXT_EDITOR_SELECTION);
        text_editor_move_last (self->editor, TEXT_EDITOR_CURSOR);

        goto reallocate;
    }

    // Handle formatting
    if (keyval == GDK_KEY_b && ctrl_pressed)
    {
        gboolean is_bold;

        is_bold = text_editor_get_format_bold_at_mark (self->editor, self->document->cursor);
        text_editor_apply_format_bold (self->editor,
                                       self->document->cursor,
                                       self->document->selection,
                                       !is_bold);
        goto reallocate;
    }

    if (keyval == GDK_KEY_i && ctrl_pressed)
    {
        gboolean is_italic;

        is_italic = text_editor_get_format_italic_at_mark (self->editor, self->document->cursor);
        text_editor_apply_format_italic (self->editor,
                                         self->document->cursor,
                                         self->document->selection,
                                         !is_italic);
        goto reallocate;
    }

    if (keyval == GDK_KEY_u && ctrl_pressed)
    {
        gboolean is_underline;

        is_underline = text_editor_get_format_underline_at_mark (self->editor, self->document->cursor);
        text_editor_apply_format_underline (self->editor,
                                            self->document->cursor,
                                            self->document->selection,
                                            !is_underline);
        goto reallocate;
    }

    // Insert image
    if (keyval == GDK_KEY_1 && ctrl_pressed)
    {
        TextImage *img;
        img = text_image_new ("placeholder.png");
        text_editor_insert_fragment(self->editor, TEXT_EDITOR_CURSOR, TEXT_FRAGMENT(img));

        goto reallocate;
    }

    return FALSE;

reallocate:
    gtk_widget_queue_allocate (GTK_WIDGET (self));

redraw:
    gtk_widget_queue_draw (GTK_WIDGET (self));
    return TRUE;
}

void
set_mark_from_cursor (TextDisplay *self,
                      double       x,
                      double       y,
                      TextMark    *mark)
{
    double displacement;

    if (self->layout_tree) {
        TextLayoutBox *box;

        // Get vertical displacement (horizontal not supported)
        displacement = self->vadjustment
                       ? -gtk_adjustment_get_value (self->vadjustment)
                       : 0;

        y -= displacement;

        box = text_layout_pick (TEXT_LAYOUT_BOX (self->layout_tree), x - self->margin_start, y - self->margin_top);

        if (box) {
            TextItem *item;
            const TextDimensions *bbox;

            item = text_layout_box_get_item (box);
            bbox = text_layout_box_get_bbox (box);

            // TODO: Properly find the nearest leaf node
            // when we have more complex renderers

            if (TEXT_IS_PARAGRAPH (item))
            {
                int index, trailing;

                // Pango automatically clamps the coordinates to the layout for us
                pango_layout_xy_to_index (text_layout_block_get_pango_layout (TEXT_LAYOUT_BLOCK (box)),
                                          (int)((x - bbox->x) * (double)PANGO_SCALE),
                                          (int)((y - bbox->y) * (double)PANGO_SCALE),
                                          &index, &trailing);

                mark->paragraph = TEXT_PARAGRAPH (item);
                mark->index = index;
            }
            else if (TEXT_IS_IMAGE (item))
            {
                // Treat bounding box opaquely
                mark->index = 0;
                mark->paragraph = TEXT_PARAGRAPH (text_node_get_parent (TEXT_NODE (item)));
                if ((x - bbox->x) > (bbox->width / 2))
                    mark->index = 1;

            }
        }
    }
}

void
drag_begin (GtkGestureDrag *gesture,
            gdouble         start_x,
            gdouble         start_y,
            TextDisplay    *self)
{
    set_mark_from_cursor (self, start_x, start_y, self->document->cursor);

    _unset_selection (self->document);

    gtk_widget_grab_focus (GTK_WIDGET (self));
    gtk_widget_queue_draw (GTK_WIDGET (self));
}

void
drag_update (GtkGestureDrag *gesture,
             gdouble         offset_x,
             gdouble         offset_y,
             TextDisplay    *self)
{
    double start_x, start_y;
    gtk_gesture_drag_get_start_point (gesture, &start_x, &start_y);

    if (!self->document->selection)
        _set_selection (self->document);

    set_mark_from_cursor (self, start_x + offset_x, start_y + offset_y, self->document->cursor);

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

    gtk_widget_set_cursor_from_name (GTK_WIDGET (self), "text");

    controller = gtk_event_controller_key_new ();
    gtk_event_controller_key_set_im_context (GTK_EVENT_CONTROLLER_KEY (controller), self->context);
    g_signal_connect (controller, "key-pressed", G_CALLBACK (key_pressed), self);
    gtk_widget_add_controller (GTK_WIDGET (self), controller);

    // Handles clicks and drags
    gesture = gtk_gesture_drag_new ();
    g_signal_connect (gesture, "drag-begin", G_CALLBACK (drag_begin), self);
    g_signal_connect (gesture, "drag-update", G_CALLBACK (drag_update), self);
    gtk_widget_add_controller (GTK_WIDGET (self), GTK_EVENT_CONTROLLER (gesture));

    gtk_widget_set_focusable (GTK_WIDGET (self), TRUE);
    gtk_widget_set_overflow (GTK_WIDGET (self), GTK_OVERFLOW_HIDDEN);
}
