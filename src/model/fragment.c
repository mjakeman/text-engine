/* fragment.c
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is dual-licensed under the terms of the Mozilla Public
 * License 2.0 and the Lesser General Public License 2.1 (or any
 * later version).
 *
 * SPDX-License-Identifier: MPL-2.0 OR LGPL-2.1-or-later
 */

#include "fragment.h"

typedef struct
{
    int _padding;
} TextFragmentPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (TextFragment, text_fragment, TEXT_TYPE_ITEM)

enum {
    PROP_0,
    N_PROPS
};

static GParamSpec *properties [N_PROPS];

static void
text_fragment_finalize (GObject *object)
{
    TextFragment *self = (TextFragment *)object;
    TextFragmentPrivate *priv = text_fragment_get_instance_private (self);

    G_OBJECT_CLASS (text_fragment_parent_class)->finalize (object);
}

static void
text_fragment_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
    TextFragment *self = TEXT_FRAGMENT (object);
    TextFragmentPrivate *priv = text_fragment_get_instance_private (self);

    switch (prop_id)
    {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
text_fragment_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
    TextFragment *self = TEXT_FRAGMENT (object);
    TextFragmentPrivate *priv = text_fragment_get_instance_private (self);

    switch (prop_id)
    {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

const char *
text_fragment_real_get_text (TextFragment *self)
{
    TextFragmentPrivate *priv = text_fragment_get_instance_private (self);
    return "";
}

int
text_fragment_get_length (TextFragment *self)
{
    const char *text;

    g_return_val_if_fail (TEXT_IS_FRAGMENT (self), -1);

    text = text_fragment_get_text (self);
    // g_print ("Length of %s is %d\n", g_type_name_from_instance (self), (int) g_utf8_strlen (text, -1));
    return (int) g_utf8_strlen (text, -1);
}

int
text_fragment_get_size_bytes (TextFragment *self)
{
    const char *text;

    g_return_val_if_fail (TEXT_IS_FRAGMENT (self), -1);

    text = text_fragment_get_text (self);
    // g_print ("Size of %s is %d\n", g_type_name_from_instance (self), (int) strlen (text));
    return (int) strlen (text);
}

const char *
text_fragment_get_text (TextFragment *self)
{
    return TEXT_FRAGMENT_CLASS (G_OBJECT_GET_CLASS (self))->get_text (self);
}

static void
text_fragment_class_init (TextFragmentClass *klass)
{
    klass->get_text = text_fragment_real_get_text;

    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = text_fragment_finalize;
    object_class->get_property = text_fragment_get_property;
    object_class->set_property = text_fragment_set_property;
}

static void
text_fragment_init (TextFragment *self)
{
}
