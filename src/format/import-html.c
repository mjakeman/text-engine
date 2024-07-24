/* import-html.c
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is dual-licensed under the terms of the Mozilla Public
 * License 2.0 and the Lesser General Public License 2.1 (or any
 * later version).
 *
 * SPDX-License-Identifier: MPL-2.0 OR LGPL-2.1-or-later
 */

#include "import.h"

#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>

#include "../model/paragraph.h"
#include "../model/block.h"
#include "../model/run.h"
#include "../model/image.h"

// Style Info
// TODO: Refactor this into a stylesheet module rather than setting it on runs directly
static gboolean is_bold = FALSE;
static gboolean is_underline = FALSE;
static gboolean is_italic = FALSE;

static void
build_text_frame_recursive (xmlNode        *nodes,
                            TextFrame      *frame,
                            TextParagraph **current)
{
    g_return_if_fail (TEXT_IS_FRAME (frame));
    g_return_if_fail (current);

    xmlNode *cur_node = NULL;

    if (nodes == NULL)
        return;

    for (cur_node = nodes; cur_node != NULL; cur_node = cur_node->next)
    {
        // ENTER NODE
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            if (g_str_equal (cur_node->name, "p") ||
                g_str_equal (cur_node->name, "br"))
            {
                *current = text_paragraph_new ();
                text_frame_append_block (frame, TEXT_BLOCK (*current));
            }
            if (g_str_equal (cur_node->name, "img"))
            {
                TextImage *image;
                xmlAttr *iter;
                char *img_src;

                *current = text_paragraph_new ();
                text_frame_append_block (frame, TEXT_BLOCK (*current));

                img_src = NULL;

                for (iter = cur_node->properties; iter != NULL; iter = iter->next)
                {
                    if (g_str_equal (iter->name, "src"))
                        img_src = g_strdup (iter->name);
                }

                image = text_image_new (img_src);
                text_paragraph_append_fragment(*current, TEXT_FRAGMENT(image));

            }
            else if (g_str_equal (cur_node->name, "b"))
                is_bold = TRUE;
            else if (g_str_equal (cur_node->name, "i"))
                is_italic = TRUE;
            else if (g_str_equal (cur_node->name, "u"))
                is_underline = TRUE;
            else
            {
                // Catch-all for not-yet implemented elements
                g_info ("Ignored element %s\n", cur_node->name);
            }
        }
        else if (cur_node->type == XML_TEXT_NODE)
        {
            // Append text as new run
            TextRun *new_run;

            const gchar *content = (gchar *)cur_node->content;
            new_run = text_run_new (content);
            text_run_set_style_bold (new_run, is_bold);
            text_run_set_style_italic (new_run, is_italic);
            text_run_set_style_underline (new_run, is_underline);
            text_paragraph_append_fragment(*current, TEXT_FRAGMENT (new_run));
        }

        // PROCESS CHILDREN
        build_text_frame_recursive (cur_node->children, frame, current);

        // EXIT NODE
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            if (g_str_equal (cur_node->name, "b"))
                is_bold = FALSE;
            else if (g_str_equal (cur_node->name, "i"))
                is_italic = FALSE;
            else if (g_str_equal (cur_node->name, "u"))
                is_underline = FALSE;
        }
    }
}

/**
 * build_text_frame:
 * @nodes: Array of #xmlNode structures to parse
 * @frame: An initialised #TextFrame to populate with data
 *
 * Recursively builds a new #TextFrame and populates it with data
 * from the provided normalised HTML tree.
 */
static void
build_text_frame (xmlNode   *nodes,
                  TextFrame *frame)
{
    TextParagraph *current = NULL;
    build_text_frame_recursive (nodes, frame, &current);
}

TextFrame *
format_parse_html (const gchar *html)
{
    htmlDocPtr doc;
    xmlNode *root;
    TextFrame *frame;

    g_info ("%s\n", html);

    doc = htmlParseDoc ((const guchar *)html, "utf-8");

    if (doc == NULL)
    {
        g_critical ("Could not parse HTML document.");
        return NULL;
    }

    root = xmlDocGetRootElement (doc);

    if (root == NULL)
    {
        g_warning ("Empty HTML document.");
        xmlFreeDoc (doc);
        xmlCleanupParser ();
        return NULL;
    }

    frame = text_frame_new ();

    g_info ("Root Node discovered: %s\n", root->name);

    build_text_frame (root, frame);

    xmlFreeDoc (doc);
    xmlCleanupParser ();

    return frame;
}
