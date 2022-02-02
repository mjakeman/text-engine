/* paragraph.c
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

#include "paragraph.h"

struct _TextParagraph
{
    TextBlock parent_instance;
};

G_DEFINE_FINAL_TYPE (TextParagraph, text_paragraph, TEXT_TYPE_BLOCK)

enum {
    PROP_0,
    N_PROPS
};

static GParamSpec *properties [N_PROPS];

TextParagraph *
text_paragraph_new (void)
{
    return g_object_new (TEXT_TYPE_PARAGRAPH, NULL);
}

static void
text_paragraph_finalize (GObject *object)
{
    TextParagraph *self = (TextParagraph *)object;

    G_OBJECT_CLASS (text_paragraph_parent_class)->finalize (object);
}

static void
text_paragraph_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
    TextParagraph *self = TEXT_PARAGRAPH (object);

    switch (prop_id)
      {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      }
}

static void
text_paragraph_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
    TextParagraph *self = TEXT_PARAGRAPH (object);

    switch (prop_id)
      {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      }
}

void
text_paragraph_append_run (TextParagraph *self,
                           TextRun       *run)
{
    g_return_if_fail (TEXT_IS_PARAGRAPH (self));
    g_return_if_fail (TEXT_IS_RUN (run));

    text_node_append_child (TEXT_NODE (self), TEXT_NODE (run));
}

static void
text_paragraph_class_init (TextParagraphClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = text_paragraph_finalize;
    object_class->get_property = text_paragraph_get_property;
    object_class->set_property = text_paragraph_set_property;
}

static void
text_paragraph_init (TextParagraph *self)
{
}
