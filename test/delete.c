/* delete.c
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
    TextRun *run4;
    TextRun *run5;
    TextParagraph *para1;
    TextParagraph *para2;
    TextParagraph *para3;
} DeleteFixture;

#define RUN1 "abcdefghij"
#define RUN2 "1234567890"
#define RUN3 "!@#$%^&*()"
#define RUN4 "zxcvbnm,./"
#define RUN5 "0987654321"

static void
delete_fixture_set_up (DeleteFixture *fixture,
                       gconstpointer  user_data)
{
    TextFrame *frame;
    TextParagraph *para1, *para2, *para3;
    TextRun *run1, *run2, *run3, *run4, *run5;

    frame = text_frame_new ();

    para1 = text_paragraph_new ();
    run1 = text_run_new (RUN1);
    run2 = text_run_new (RUN2);
    run3 = text_run_new (RUN3);
    text_paragraph_append_inline(para1, run1);
    text_paragraph_append_inline(para1, run2);
    text_paragraph_append_inline(para1, run3);
    text_frame_append_block (frame, TEXT_BLOCK (para1));

    para2 = text_paragraph_new ();
    run4 = text_run_new (RUN4);
    text_paragraph_append_inline(para2, run4);
    text_frame_append_block (frame, TEXT_BLOCK (para2));

    para3 = text_paragraph_new ();
    run5 = text_run_new (RUN5);
    text_paragraph_append_inline(para3, run5);
    text_frame_append_block (frame, TEXT_BLOCK (para3));

    fixture->doc = text_document_new ();
    fixture->doc->frame = frame;

    fixture->editor = text_editor_new (fixture->doc);

    text_editor_move_first (fixture->editor, TEXT_EDITOR_CURSOR);

    fixture->run1 = run1;
    fixture->run2 = run2;
    fixture->run3 = run3;
    fixture->run4 = run4;
    fixture->run5 = run5;
    fixture->para1 = para1;
    fixture->para2 = para2;
    fixture->para3 = para3;
}

static void
delete_fixture_tear_down (DeleteFixture *fixture,
                          gconstpointer  user_data)
{
    g_object_unref (fixture->editor);
    g_object_unref (fixture->doc);
}

static void
test_within_run (DeleteFixture *fixture,
                 gconstpointer  user_data)
{
    gchar *text;

    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 2);
    text_editor_delete (fixture->editor, TEXT_EDITOR_CURSOR, 5);

    g_object_get (fixture->run1, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "abhij");
}

static void
test_whole_run_others_in_paragraph (DeleteFixture *fixture,
                                    gconstpointer  user_data)
{
    gchar *text;
    int length;

    // deletes the contents of run 2
    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 10);
    text_editor_delete (fixture->editor, TEXT_EDITOR_CURSOR, 10);

    // run 2 should no longer exist
    g_assert_true (text_editor_get_run (fixture->editor, TEXT_EDITOR_CURSOR) == fixture->run1);
    g_assert_cmpint (fixture->doc->cursor->index, ==, 10);

    // check length
    length = text_paragraph_get_length (fixture->para1);
    g_assert_cmpint (length, ==, 20);

    // check cursor position
    g_assert_cmpint (fixture->doc->cursor->index, ==, 10);

    // check text
    g_object_get (fixture->run1, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "abcdefghij");

    g_object_get (fixture->run3, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "!@#$%^&*()");
}

static void
test_whole_run_last_in_paragraph (DeleteFixture *fixture,
                                  gconstpointer  user_data)
{
    gchar *text;
    int length;

    // deletes the contents of run 4
    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 31);
    text_editor_delete (fixture->editor, TEXT_EDITOR_CURSOR, 10);

    // run 4 should still exist with a length of zero
    // but one index position
    length = text_paragraph_get_length (fixture->para2);
    g_assert_cmpint (length, ==, 0);

    // check cursor position
    g_assert_true (text_editor_get_run (fixture->editor, TEXT_EDITOR_CURSOR) == fixture->run4);
    g_assert_cmpint (fixture->doc->cursor->index, ==, 0);

    // check text
    g_object_get (fixture->run4, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "");
}

static void
test_across_runs_single (DeleteFixture *fixture,
                         gconstpointer  user_data)
{
    gchar *text;
    int length;

    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 8);
    text_editor_delete (fixture->editor, TEXT_EDITOR_CURSOR, 5);

    length = text_paragraph_get_length (fixture->para1);
    g_assert_cmpint (length, ==, 25);

    g_object_get (fixture->run1, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "abcdefgh");

    g_object_get (fixture->run2, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "4567890");
}

static void
test_across_runs_multiple (DeleteFixture *fixture,
                           gconstpointer  user_data)
{
    gchar *text;
    int length;

    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 8);
    text_editor_delete (fixture->editor, TEXT_EDITOR_CURSOR, 15);

    length = text_paragraph_get_length (fixture->para1);
    g_assert_cmpint (length, ==, 15);

    g_object_get (fixture->run1, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "abcdefgh");

    g_object_get (fixture->run3, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "$%^&*()");
}

static void
test_run_boundary (DeleteFixture *fixture,
                   gconstpointer  user_data)
{
    gchar *text;
    int length;

    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 10);
    text_editor_delete (fixture->editor, TEXT_EDITOR_CURSOR, 1);

    length = text_paragraph_get_length (fixture->para1);
    g_assert_cmpint (length, ==, 29);

    g_object_get (fixture->run1, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "abcdefghij");

    g_object_get (fixture->run2, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "234567890");
}

static void
test_across_paragraphs_single (DeleteFixture *fixture,
                               gconstpointer  user_data)
{
    gchar *text;
    int length;

    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 25);
    text_editor_delete (fixture->editor, TEXT_EDITOR_CURSOR, 11);

    // before:
    //     abcdefghij1234567890!@#$%^&*()
    //     zxcvbnm,./
    //     0987654321
    // after:
    //     abcdefghij1234567890!@#$%nm,./
    //     0987654321

    // check paragraphs have been merged
    length = text_paragraph_get_length (fixture->para1);
    g_assert_cmpint (length, ==, 30);

    // check text is unchanged
    g_object_get (fixture->run1, "text", &text, NULL);
    g_assert_cmpstr (text, ==, RUN1);

    g_object_get (fixture->run2, "text", &text, NULL);
    g_assert_cmpstr (text, ==, RUN2);

    // check modified correctly
    g_object_get (fixture->run3, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "!@#$%");

    g_object_get (fixture->run4, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "nm,./");
}

static void
test_across_paragraphs_multiple (DeleteFixture *fixture,
                                 gconstpointer  user_data)
{
    gchar *text;
    int length;

    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 25);
    text_editor_delete (fixture->editor, TEXT_EDITOR_CURSOR, 22);

    // before:
    //     abcdefghij1234567890!@#$%^&*()
    //     zxcvbnm,./
    //     0987654321
    // after:
    //     abcdefghij1234567890!@#$%54321

    // check paragraphs have been merged
    length = text_paragraph_get_length (fixture->para1);
    g_assert_cmpint (length, ==, 30);

    // check text is unchanged
    g_object_get (fixture->run1, "text", &text, NULL);
    g_assert_cmpstr (text, ==, RUN1);

    g_object_get (fixture->run2, "text", &text, NULL);
    g_assert_cmpstr (text, ==, RUN2);

    // check modified correctly
    g_object_get (fixture->run3, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "!@#$%");

    g_object_get (fixture->run5, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "54321");
}

static void
test_paragraph_boundary_to_next_index (DeleteFixture *fixture,
                                       gconstpointer  user_data)
{
    gchar *text;
    int length;

    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 30);
    text_editor_delete (fixture->editor, TEXT_EDITOR_CURSOR, 1);

    // before:
    //     abcdefghij1234567890!@#$%^&*()
    //     zxcvbnm,./
    //     0987654321
    // after:
    //     abcdefghij1234567890!@#$%^&*()zxcvbnm,./
    //     0987654321

    // check paragraphs have been merged
    length = text_paragraph_get_length (fixture->para1);
    g_assert_cmpint (length, ==, 40);

    // check text is unchanged
    g_object_get (fixture->run1, "text", &text, NULL);
    g_assert_cmpstr (text, ==, RUN1);

    g_object_get (fixture->run2, "text", &text, NULL);
    g_assert_cmpstr (text, ==, RUN2);

    g_object_get (fixture->run3, "text", &text, NULL);
    g_assert_cmpstr (text, ==, RUN3);

    g_object_get (fixture->run4, "text", &text, NULL);
    g_assert_cmpstr (text, ==, RUN4);
}

static void
test_paragraph_boundary_to_next_boundary (DeleteFixture *fixture,
                                          gconstpointer  user_data)
{
    gchar *text;
    int length;

    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 30);
    text_editor_delete (fixture->editor, TEXT_EDITOR_CURSOR, 11);

    // before:
    //     abcdefghij1234567890!@#$%^&*()
    //     zxcvbnm,./
    //     0987654321
    // after:
    //     abcdefghij1234567890!@#$%^&*()
    //     0987654321

    // check paragraphs remain the same
    length = text_paragraph_get_length (fixture->para1);
    g_assert_cmpint (length, ==, 30);

    length = text_paragraph_get_length (fixture->para3);
    g_assert_cmpint (length, ==, 10);

    // check text is unchanged
    g_object_get (fixture->run1, "text", &text, NULL);
    g_assert_cmpstr (text, ==, RUN1);

    g_object_get (fixture->run2, "text", &text, NULL);
    g_assert_cmpstr (text, ==, RUN2);

    g_object_get (fixture->run3, "text", &text, NULL);
    g_assert_cmpstr (text, ==, RUN3);

    g_object_get (fixture->run5, "text", &text, NULL);
    g_assert_cmpstr (text, ==, RUN5);
}

static void
test_nothing (DeleteFixture *fixture,
              gconstpointer  user_data)
{
    gchar *text;

    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 2);
    text_editor_delete (fixture->editor, TEXT_EDITOR_CURSOR, 0);

    g_object_get (fixture->run1, "text", &text, NULL);
    g_assert_cmpstr (text, ==, RUN1);
}

static void
test_inverse (DeleteFixture *fixture,
              gconstpointer  user_data)
{
    gchar *text;
    int length;

    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 8);
    text_editor_delete (fixture->editor, TEXT_EDITOR_CURSOR, -5);

    // before:
    //     abcdefghij1234567890!@#$%^&*()
    // after:
    //     abcij1234567890!@#$%^&*()

    // check paragraphs have been merged
    length = text_paragraph_get_length (fixture->para1);
    g_assert_cmpint (length, ==, 25);

    g_object_get (fixture->run1, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "abcij");
}

static void
test_inverse_bounds (DeleteFixture *fixture,
                     gconstpointer  user_data)
{
    gchar *text;
    int length;

    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 3);
    text_editor_delete (fixture->editor, TEXT_EDITOR_CURSOR, -5);

    // before:
    //     abcdefghij1234567890!@#$%^&*()
    // after:
    //     defghij1234567890!@#$%^&*()

    // check paragraphs have been merged
    length = text_paragraph_get_length (fixture->para1);
    g_assert_cmpint (length, ==, 27);

    g_object_get (fixture->run1, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "defghij");
}

static void
test_backspace (DeleteFixture *fixture,
                gconstpointer  user_data)
{
    gchar *text;
    int length;

    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 31);
    text_editor_delete (fixture->editor, TEXT_EDITOR_CURSOR, -1);

    // before:
    //     abcdefghij1234567890!@#$%^&*()
    //     zxcvbnm,./
    //     0987654321
    // after:
    //     abcdefghij1234567890!@#$%^&*()zxcvbnm,./
    //     0987654321

    // check paragraphs have been merged
    length = text_paragraph_get_length (fixture->para1);
    g_assert_cmpint (length, ==, 40);

    // check text is unchanged
    g_object_get (fixture->run1, "text", &text, NULL);
    g_assert_cmpstr (text, ==, RUN1);

    g_object_get (fixture->run2, "text", &text, NULL);
    g_assert_cmpstr (text, ==, RUN2);

    g_object_get (fixture->run3, "text", &text, NULL);
    g_assert_cmpstr (text, ==, RUN3);

    g_object_get (fixture->run4, "text", &text, NULL);
    g_assert_cmpstr (text, ==, RUN4);

}

int
main (int argc, char *argv[])
{
    setlocale (LC_ALL, "");

    g_test_init (&argc, &argv, NULL);

    // Test within run
    g_test_add ("/text-engine/editor/delete/test-within-run", DeleteFixture, NULL,
                delete_fixture_set_up, test_within_run,
                delete_fixture_tear_down);

    // Test whole run
    g_test_add ("/text-engine/editor/delete/test-whole-run-others-in-paragraph", DeleteFixture, NULL,
                delete_fixture_set_up, test_whole_run_others_in_paragraph,
                delete_fixture_tear_down);
    g_test_add ("/text-engine/editor/delete/test-whole-run-last-in-paragraph", DeleteFixture, NULL,
                delete_fixture_set_up, test_whole_run_last_in_paragraph,
                delete_fixture_tear_down);

    // Test across runs
    g_test_add ("/text-engine/editor/delete/test-across-runs-single", DeleteFixture, NULL,
                delete_fixture_set_up, test_across_runs_single,
                delete_fixture_tear_down);
    g_test_add ("/text-engine/editor/delete/test-across-runs-multiple", DeleteFixture, NULL,
                delete_fixture_set_up, test_across_runs_multiple,
                delete_fixture_tear_down);

    // Test run boundary
    g_test_add ("/text-engine/editor/delete/test-run-boundary", DeleteFixture, NULL,
                delete_fixture_set_up, test_run_boundary,
                delete_fixture_tear_down);

    // Test across paragraphs
    g_test_add ("/text-engine/editor/delete/test-across-paragraphs-single", DeleteFixture, NULL,
                delete_fixture_set_up, test_across_paragraphs_single,
                delete_fixture_tear_down);
    g_test_add ("/text-engine/editor/delete/test-across-paragraphs-multiple", DeleteFixture, NULL,
                delete_fixture_set_up, test_across_paragraphs_multiple,
                delete_fixture_tear_down);

    // Test paragraph boundary
    g_test_add ("/text-engine/editor/delete/test-paragraph-boundary-to-next-index", DeleteFixture, NULL,
                delete_fixture_set_up, test_paragraph_boundary_to_next_index,
                delete_fixture_tear_down);
    g_test_add ("/text-engine/editor/delete/test-paragraph-boundary-to-next-boundary", DeleteFixture, NULL,
                delete_fixture_set_up, test_paragraph_boundary_to_next_boundary,
                delete_fixture_tear_down);

    // Test nothing
    g_test_add ("/text-engine/editor/delete/test-nothing", DeleteFixture, NULL,
                delete_fixture_set_up, test_nothing,
                delete_fixture_tear_down);

    // Test inverse
    g_test_add ("/text-engine/editor/delete/test-inverse", DeleteFixture, NULL,
                delete_fixture_set_up, test_inverse,
                delete_fixture_tear_down);
    g_test_add ("/text-engine/editor/delete/test-inverse-bounds", DeleteFixture, NULL,
                delete_fixture_set_up, test_inverse_bounds,
                delete_fixture_tear_down);
    g_test_add ("/text-engine/editor/delete/test-backspace", DeleteFixture, NULL,
                delete_fixture_set_up, test_backspace,
                delete_fixture_tear_down);

    return g_test_run ();
}

