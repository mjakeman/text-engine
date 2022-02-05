/* display.c
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * SPDX-License-Identifier: MIT
 */

#include "display.h"

#include "../model/paragraph.h"

struct _TextDisplay
{
    GtkWidget parent_instance;

    TextFrame *frame;
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
        self->frame = g_value_get_object (value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
text_display_snapshot (GtkWidget   *widget,
                       GtkSnapshot *snapshot)
{
    g_return_if_fail (TEXT_IS_DISPLAY (widget));

    TextDisplay *self = TEXT_DISPLAY (widget);

    if (!self->frame)
        return;

    TextFrame *frame = self->frame;
    PangoContext *context = gtk_widget_get_pango_context (widget);

    GdkRGBA fg_color;
    gtk_style_context_get_color (gtk_widget_get_style_context (widget), &fg_color);

    // Two things need to happen here.
    //
    // Firstly, we need to layout the frame probably into some form of
    // layout tree. This can be cached between redraws so only the
    // changed block nodes need to be recalculated.
    //  -> Introduce an auxiliary TextLayout object
    //  -> Uses Pango (recursively?) for determining extents
    //
    // Secondly, the actual drawing is implementation-specific. The data
    // model provides stylistic information (data attributes) and semantic
    // structure, but the actual presentation depends on other factors such
    // as 'default stylesheet' and reflow.
    //  -> This is implemented by this widget

    for (TextNode *node = text_node_get_first_child (TEXT_NODE (frame));
         node != NULL;
         node = text_node_get_next (node))
    {
        g_assert (TEXT_IS_BLOCK (node));

        // Let's treat paragraphs opaquely for now. In the future, we need
        // to manually consider each text run in order for inline equations
        // and images.
        if (TEXT_IS_PARAGRAPH (node))
        {
            GString *str = g_string_new ("");
            for (TextNode *run = text_node_get_first_child (node);
                 run != NULL;
                 run = text_node_get_next (run))
            {
                const gchar *run_text;
                g_object_get (run, "text", &run_text, NULL);
                g_string_append (str, run_text);
            }

            int height;
            gchar *text = g_string_free (str, FALSE);

            PangoLayout *layout = pango_layout_new (context);
            pango_layout_set_text (layout, text, -1);
            pango_layout_set_wrap (layout, PANGO_WRAP_WORD_CHAR);
            pango_layout_set_width (layout, PANGO_SCALE * gtk_widget_get_width (widget));
            pango_layout_get_pixel_size (layout, NULL, &height);

            gtk_snapshot_append_layout (snapshot, layout, &fg_color);
            gtk_snapshot_translate (snapshot, &GRAPHENE_POINT_INIT (0, height));

            g_object_unref (layout);
            g_free (text);
        }
    }
}

static GtkSizeRequestMode
text_display_get_request_mode (GtkWidget* widget)
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
        int min_height = 0;
        TextFrame *frame = TEXT_DISPLAY (widget)->frame;
        PangoContext *context = gtk_widget_get_pango_context (widget);

        // TODO: Remove duplication between snapshot and measure
        for (TextNode *node = text_node_get_first_child (TEXT_NODE (frame));
             node != NULL;
             node = text_node_get_next (node))
        {
            g_assert (TEXT_IS_BLOCK (node));

            // Let's treat paragraphs opaquely for now. In the future, we need
            // to manually consider each text run in order for inline equations
            // and images.
            if (TEXT_IS_PARAGRAPH (node))
            {
                GString *str = g_string_new ("");
                for (TextNode *run = text_node_get_first_child (node);
                     run != NULL;
                     run = text_node_get_next (run))
                {
                    const gchar *run_text;
                    g_object_get (run, "text", &run_text, NULL);
                    g_string_append (str, run_text);
                }

                int height;
                gchar *text = g_string_free (str, FALSE);

                PangoLayout *layout = pango_layout_new (context);
                pango_layout_set_text (layout, text, -1);
                pango_layout_set_wrap (layout, PANGO_WRAP_WORD_CHAR);
                pango_layout_set_width (layout, PANGO_SCALE * for_size);
                pango_layout_get_pixel_size (layout, NULL, &height);

                min_height += height;

                g_object_unref (layout);
                g_free (text);
            }
        }

        *minimum = *natural = min_height;
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

static void
text_display_init (TextDisplay *self)
{
}
