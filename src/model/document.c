/* document.c
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

#include "document.h"

G_DEFINE_FINAL_TYPE (TextDocument, text_document, G_TYPE_OBJECT)

enum {
    PROP_0,
    N_PROPS
};

static GParamSpec *properties [N_PROPS];

TextDocument *
text_document_new (void)
{
    return g_object_new (TEXT_TYPE_DOCUMENT, NULL);
}

static void
text_document_finalize (GObject *object)
{
    TextDocument *self = (TextDocument *)object;

    G_OBJECT_CLASS (text_document_parent_class)->finalize (object);
}

static void
text_document_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
    TextDocument *self = TEXT_DOCUMENT (object);

    switch (prop_id)
      {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      }
}

static void
text_document_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
    TextDocument *self = TEXT_DOCUMENT (object);

    switch (prop_id)
      {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      }
}

static void
text_document_class_init (TextDocumentClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = text_document_finalize;
    object_class->get_property = text_document_get_property;
    object_class->set_property = text_document_set_property;
}

GSList *
text_document_get_all_marks (TextDocument *doc)
{
    GSList *marks;

    marks = g_slist_copy (doc->marks);
    marks = g_slist_append (doc->marks, doc->cursor);

    if (doc->selection)
        marks = g_slist_append (doc->marks, doc->selection);

    return marks;
}

TextMark *
text_document_create_mark (TextDocument  *doc,
                           TextParagraph *paragraph,
                           int            index,
                           TextGravity    gravity)
{
    TextMark *new;

    g_return_val_if_fail (TEXT_IS_DOCUMENT (doc), NULL);
    g_return_val_if_fail (TEXT_IS_PARAGRAPH (paragraph), NULL);

    new = text_mark_new (paragraph, index, gravity);
    doc->marks = g_slist_append (doc->marks, new);

    return new;
}

TextMark *
text_document_copy_mark (TextDocument *doc,
                         TextMark     *mark)
{
    TextMark *new;

    g_return_val_if_fail (TEXT_IS_DOCUMENT (doc), NULL);
    g_return_val_if_fail (mark != NULL, NULL);

    new = text_mark_copy (mark);
    doc->marks = g_slist_append (doc->marks, new);

    return new;
}

void
text_document_delete_mark (TextDocument *doc,
                           TextMark     *mark)
{
    g_return_if_fail (TEXT_IS_DOCUMENT (doc));
    g_return_if_fail (mark != NULL);

    doc->marks = g_slist_remove (doc->marks, mark);
}

void
text_document_clear_mark (TextDocument *doc,
                          TextMark     **mark)
{
    g_return_if_fail (TEXT_IS_DOCUMENT (doc));
    g_return_if_fail (mark != NULL);

    doc->marks = g_slist_remove (doc->marks, *mark);
    *mark = NULL;
}

static void
text_document_init (TextDocument *self)
{
    self->cursor = text_mark_new (NULL, 0, TEXT_GRAVITY_RIGHT);
}
