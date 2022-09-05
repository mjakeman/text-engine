/* insert.c
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is dual-licensed under the terms of the Mozilla Public
 * License 2.0 and the Lesser General Public License 2.1 (or any
 * later version).
 *
 * SPDX-License-Identifier: MPL-2.0 OR LGPL-2.1-or-later
 */

#include <glib.h>
#include <locale.h>
#include <model/document.h>
#include <model/paragraph.h>
#include <model/run.h>
#include <editor/editor.h>

typedef struct {
    TextDocument *doc;
    TextEditor *editor;

    TextRun *run1;
    TextRun *run2;
    TextRun *run3;
} InsertFixture;

#define RUN1 "Once upon a time there was a little dog, "
#define RUN2 "and his name was Rover."
#define RUN3 "By J. R. R. Tolkien"

static void
insert_fixture_set_up (InsertFixture *fixture,
                       gconstpointer  user_data)
{
    TextFrame *frame;
    TextParagraph *para1, *para2;
    TextRun *run1, *run2, *run3;

    frame = text_frame_new ();

    para1 = text_paragraph_new ();
    run1 = text_run_new (RUN1);
    run2 = text_run_new (RUN2);
    text_paragraph_append_inline(para1, run1);
    text_paragraph_append_inline(para1, run2);
    text_frame_append_block (frame, TEXT_BLOCK (para1));

    para2 = text_paragraph_new ();
    run3 = text_run_new (RUN3);
    text_paragraph_append_inline(para2, run3);
    text_frame_append_block (frame, TEXT_BLOCK (para2));

    fixture->doc = text_document_new ();
    fixture->doc->frame = frame;

    fixture->editor = text_editor_new (fixture->doc);

    text_editor_move_first (fixture->editor, TEXT_EDITOR_CURSOR);

    fixture->run1 = run1;
    fixture->run2 = run2;
    fixture->run3 = run3;
}

static void
insert_fixture_tear_down (InsertFixture *fixture,
                          gconstpointer  user_data)
{
    g_object_unref (fixture->editor);
    g_object_unref (fixture->doc);
}

static void
test_insert_test_start (InsertFixture *fixture,
                        gconstpointer  user_data)
{
    // test inserting at the start of a run

    gchar *text;

    text_editor_insert_text(fixture->editor, TEXT_EDITOR_CURSOR, "Alas! ");

    // changed
    g_object_get (fixture->run1, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "Alas! Once upon a time there was a little dog, ");

    // unchanged
    g_object_get (fixture->run2, "text", &text, NULL);
    g_assert_cmpstr (text, ==, RUN2);

    // unchanged
    g_object_get (fixture->run3, "text", &text, NULL);
    g_assert_cmpstr (text, ==, RUN3);

    // correct cursor index
    g_assert_cmpint (fixture->doc->cursor->index, ==, 6);
}

static void
test_insert_test_middle (InsertFixture *fixture,
                         gconstpointer  user_data)
{
    // test inserting in the middle of a run (common case)

    gchar *text;

    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 26);
    text_editor_insert_text(fixture->editor, TEXT_EDITOR_CURSOR, "n't");

    // changed
    g_object_get (fixture->run1, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "Once upon a time there wasn't a little dog, ");

    // unchanged
    g_object_get (fixture->run2, "text", &text, NULL);
    g_assert_cmpstr (text, ==, RUN2);

    // unchanged
    g_object_get (fixture->run3, "text", &text, NULL);
    g_assert_cmpstr (text, ==, RUN3);

    // correct cursor index
    g_assert_cmpint (fixture->doc->cursor->index, ==, 29);
}

static void
test_insert_test_end (InsertFixture *fixture,
                      gconstpointer  user_data)
{
    // test inserting at the end of a run

    gchar *text;

    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 41);
    text_editor_insert_text(fixture->editor, TEXT_EDITOR_CURSOR, "or at least I thought so...");

    // changed
    g_object_get (fixture->run1, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "Once upon a time there was a little dog, or at least I thought so...");

    // unchanged
    g_object_get (fixture->run2, "text", &text, NULL);
    g_assert_cmpstr (text, ==, RUN2);

    // unchanged
    g_object_get (fixture->run3, "text", &text, NULL);
    g_assert_cmpstr (text, ==, RUN3);

    // correct cursor index
    g_assert_cmpint (fixture->doc->cursor->index, ==, 68);
}

static void
test_insert_test_nothing (InsertFixture *fixture,
                          gconstpointer  user_data)
{
    gchar *text;

    text_editor_insert_text(fixture->editor, TEXT_EDITOR_CURSOR, "");

    // changed
    g_object_get (fixture->run1, "text", &text, NULL);
    g_assert_cmpstr (text, ==, RUN1);

    // unchanged
    g_object_get (fixture->run2, "text", &text, NULL);
    g_assert_cmpstr (text, ==, RUN2);

    // unchanged
    g_object_get (fixture->run3, "text", &text, NULL);
    g_assert_cmpstr (text, ==, RUN3);

    // correct cursor index
    g_assert_cmpint (fixture->doc->cursor->index, ==, 0);
}

int
main (int argc, char *argv[])
{
    setlocale (LC_ALL, "");

    g_test_init (&argc, &argv, NULL);

    // Define the tests.
    g_test_add ("/text-engine/editor/insert/test-start", InsertFixture, NULL,
                insert_fixture_set_up, test_insert_test_start,
                insert_fixture_tear_down);
    g_test_add ("/text-engine/editor/insert/test-middle", InsertFixture, NULL,
                insert_fixture_set_up, test_insert_test_middle,
                insert_fixture_tear_down);
    g_test_add ("/text-engine/editor/insert/test-end", InsertFixture, NULL,
                insert_fixture_set_up, test_insert_test_end,
                insert_fixture_tear_down);
    g_test_add ("/text-engine/editor/insert/test-nothing", InsertFixture, NULL,
                insert_fixture_set_up, test_insert_test_nothing,
                insert_fixture_tear_down);

    return g_test_run ();
}

