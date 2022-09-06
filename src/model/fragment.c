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
    gchar *text;
} TextFragmentPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (TextFragment, text_fragment, TEXT_TYPE_ITEM)

enum {
    PROP_0,
    PROP_TEXT,
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
        case PROP_TEXT:
            g_value_set_string (value, priv->text);
            break;
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
        case PROP_TEXT:
            if (priv->text)
                g_free (priv->text);
            priv->text = g_value_dup_string (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

int
text_fragment_real_get_length (TextFragment *self)
{
    g_return_val_if_fail (TEXT_IS_FRAGMENT (self), -1);

    TextFragmentPrivate *priv = text_fragment_get_instance_private (self);

    if (priv->text)
        return (int) strlen (priv->text);

    return 0;
}

const char *
text_fragment_real_get_text (TextFragment *self)
{
    TextFragmentPrivate *priv = text_fragment_get_instance_private (self);
    return priv->text;
}

int
text_fragment_get_length (TextFragment *self)
{
    return TEXT_FRAGMENT_CLASS (G_OBJECT_GET_CLASS (self))->get_length (self);
}

const char *
text_fragment_get_text (TextFragment *self)
{
    return TEXT_FRAGMENT_CLASS (G_OBJECT_GET_CLASS (self))->get_text (self);
}

static void
text_fragment_class_init (TextFragmentClass *klass)
{
    klass->get_length = text_fragment_real_get_length;
    klass->get_text = text_fragment_real_get_text;

    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = text_fragment_finalize;
    object_class->get_property = text_fragment_get_property;
    object_class->set_property = text_fragment_set_property;

    properties [PROP_TEXT]
            = g_param_spec_string ("text",
                                   "Text",
                                   "Text",
                                   NULL,
                                   G_PARAM_READWRITE|G_PARAM_CONSTRUCT);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
text_fragment_init (TextFragment *self)
{
}
