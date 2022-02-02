/* widget.c
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

#include "widget.h"

#include <text-engine.h>

#include <model/node.h>
#include <model/block.h>
#include <model/frame.h>
#include <model/paragraph.h>

struct _RichTextWidget
{
    GtkWidget parent_instance;

    TextFrame *frame;
};

G_DEFINE_FINAL_TYPE (RichTextWidget, rich_text_widget, GTK_TYPE_WIDGET)

enum {
    PROP_0,
    N_PROPS
};

static GParamSpec *properties [N_PROPS];

RichTextWidget *
rich_text_widget_new (void)
{
    return g_object_new (RICH_TEXT_TYPE_WIDGET, NULL);
}

static void
rich_text_widget_finalize (GObject *object)
{
    RichTextWidget *self = (RichTextWidget *)object;

    G_OBJECT_CLASS (rich_text_widget_parent_class)->finalize (object);
}

static void
rich_text_widget_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
    RichTextWidget *self = RICH_TEXT_WIDGET (object);

    switch (prop_id)
      {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      }
}

static void
rich_text_widget_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
    RichTextWidget *self = RICH_TEXT_WIDGET (object);

    switch (prop_id)
      {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      }
}

static void
rich_text_widget_snapshot (GtkWidget   *widget,
                           GtkSnapshot *snapshot)
{
    RichTextWidget *self = RICH_TEXT_WIDGET (widget);

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
            pango_layout_get_pixel_size (layout, NULL, &height);

            gtk_snapshot_append_layout (snapshot, layout, &fg_color);
            gtk_snapshot_translate (snapshot, &GRAPHENE_POINT_INIT (0, height));

            g_object_unref (layout);
            g_free (text);
        }
    }
}

static void
rich_text_widget_class_init (RichTextWidgetClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = rich_text_widget_finalize;
    object_class->get_property = rich_text_widget_get_property;
    object_class->set_property = rich_text_widget_set_property;

    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    widget_class->snapshot = rich_text_widget_snapshot;
}

static void
rich_text_widget_init (RichTextWidget *self)
{
    self->frame = text_frame_new ();

    TextParagraph *block1 = text_paragraph_new ();
    text_paragraph_append_run (block1, text_run_new ("This is some text."));
    text_paragraph_append_run (block1, text_run_new (" Comprised of multiple runs"));
    text_paragraph_append_run (block1, text_run_new (" which could each have their own..."));
    text_paragraph_append_run (block1, text_run_new (" FORMATTING!"));
    text_paragraph_append_run (block1, text_run_new (" (yay)"));
    text_frame_append_block (self->frame, TEXT_BLOCK (block1));

    TextParagraph *block2 = text_paragraph_new ();
    text_paragraph_append_run (block2, text_run_new ("Some more text!"));
    text_frame_append_block (self->frame, TEXT_BLOCK (block2));

    TextParagraph *block3 = text_paragraph_new ();
    text_paragraph_append_run (block3, text_run_new ("One final paragraph :D"));
    text_frame_append_block (self->frame, TEXT_BLOCK (block3));
}
