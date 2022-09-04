/* image.c
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is dual-licensed under the terms of the Mozilla Public
 * License 2.0 and the Lesser General Public License 2.1 (or any
 * later version).
 *
 * SPDX-License-Identifier: MPL-2.0 OR LGPL-2.1-or-later
 */

#include "image.h"

struct _TextImage
{
    TextInline parent_instance;
    gchar *src;
};


G_DEFINE_FINAL_TYPE (TextImage, text_image, TEXT_TYPE_INLINE)

enum {
    PROP_0,
    PROP_SRC,
    N_PROPS
};

static GParamSpec *properties [N_PROPS];

TextImage *
text_image_new (const gchar *text)
{
    return g_object_new (TEXT_TYPE_IMAGE,
                         "text", text,
                         NULL);
}

static void
text_image_finalize (GObject *object)
{
    TextImage *self = (TextImage *)object;

    G_OBJECT_CLASS (text_image_parent_class)->finalize (object);
}

static void
text_image_get_property (GObject    *object,
                       guint       prop_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
    TextImage *self = TEXT_IMAGE (object);

    switch (prop_id)
    {
    case PROP_SRC:
        g_value_set_string (value, self->src);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
text_image_set_property (GObject      *object,
                       guint         prop_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
    TextImage *self = TEXT_IMAGE (object);

    switch (prop_id)
    {
    case PROP_SRC:
        if (self->src)
            g_free (self->src);
        self->src = g_value_dup_string (value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
text_image_class_init (TextImageClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = text_image_finalize;
    object_class->get_property = text_image_get_property;
    object_class->set_property = text_image_set_property;

    properties [PROP_SRC]
        = g_param_spec_string ("src",
                               "Source",
                               "Source",
                               NULL,
                               G_PARAM_READWRITE|G_PARAM_CONSTRUCT);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
text_image_init (TextImage *self)
{
}
