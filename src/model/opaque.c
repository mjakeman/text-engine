/* opaque.c
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is dual-licensed under the terms of the Mozilla Public
 * License 2.0 and the Lesser General Public License 2.1 (or any
 * later version).
 *
 * SPDX-License-Identifier: MPL-2.0 OR LGPL-2.1-or-later
 */

#include "opaque.h"

typedef struct
{
    int _padding;
} TextOpaquePrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (TextOpaque, text_opaque, TEXT_TYPE_FRAGMENT)

enum {
    PROP_0,
    N_PROPS
};

static GParamSpec *properties [N_PROPS];

static void
text_opaque_finalize (GObject *object)
{
    TextOpaque *self = (TextOpaque *)object;
    TextOpaquePrivate *priv = text_opaque_get_instance_private (self);

    G_OBJECT_CLASS (text_opaque_parent_class)->finalize (object);
}

static void
text_opaque_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
    TextOpaque *self = TEXT_OPAQUE (object);

    switch (prop_id)
    {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
text_opaque_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
    TextOpaque *self = TEXT_OPAQUE (object);

    switch (prop_id)
    {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

int
text_opaque_get_length (TextOpaque *self)
{
    return 1;
}

static void
text_opaque_class_init (TextOpaqueClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = text_opaque_finalize;
    object_class->get_property = text_opaque_get_property;
    object_class->set_property = text_opaque_set_property;

    TextFragmentClass *fragment_class = TEXT_FRAGMENT_CLASS (klass);

    fragment_class->get_length = text_opaque_get_length;
}

static void
text_opaque_init (TextOpaque *self)
{
}
