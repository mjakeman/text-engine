/* inline.c
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is dual-licensed under the terms of the Mozilla Public
 * License 2.0 and the Lesser General Public License 2.1 (or any
 * later version).
 *
 * SPDX-License-Identifier: MPL-2.0 OR LGPL-2.1-or-later
 */

#include "inline.h"

typedef struct
{
    int _padding;
} TextInlinePrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (TextInline, text_inline, TEXT_TYPE_ITEM)

enum {
    PROP_0,
    N_PROPS
};

static GParamSpec *properties [N_PROPS];

static void
text_inline_finalize (GObject *object)
{
    TextInline *self = (TextInline *)object;
    TextInlinePrivate *priv = text_inline_get_instance_private (self);

    G_OBJECT_CLASS (text_inline_parent_class)->finalize (object);
}

static void
text_inline_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
    TextInline *self = TEXT_INLINE (object);

    switch (prop_id)
    {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
text_inline_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
    TextInline *self = TEXT_INLINE (object);

    switch (prop_id)
    {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

int
text_inline_real_get_length (TextInline *self)
{
    return 1;
}

int
text_inline_get_length (TextInline *self)
{
    return TEXT_INLINE_CLASS (G_OBJECT_GET_CLASS (self))->get_length (self);
}

static void
text_inline_class_init (TextInlineClass *klass)
{
    klass->get_length = text_inline_real_get_length;

    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = text_inline_finalize;
    object_class->get_property = text_inline_get_property;
    object_class->set_property = text_inline_set_property;
}

static void
text_inline_init (TextInline *self)
{
}
