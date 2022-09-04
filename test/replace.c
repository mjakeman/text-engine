/* replace.c
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
} ReplaceFixture;

#define RUN1 "abcdefghij"
#define RUN2 "1234567890"
#define RUN3 "!@#$%^&*()"
#define RUN4 "zxcvbnm,./"
#define RUN5 "0987654321"

static void
replace_fixture_set_up (ReplaceFixture *fixture,
                        gconstpointer   user_data)
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
replace_fixture_tear_down (ReplaceFixture *fixture,
                           gconstpointer   user_data)
{
    g_object_unref (fixture->editor);
    g_object_unref (fixture->doc);
}

static void
test_within_run (ReplaceFixture *fixture,
                 gconstpointer   user_data)
{
    gchar *text;
    int length;

    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 15);

    // TODO: Don't call this here
    fixture->doc->selection = text_document_copy_mark (fixture->doc, fixture->doc->cursor);
    text_editor_move_right (fixture->editor, TEXT_EDITOR_SELECTION, 5);
    text_editor_replace (fixture->editor,
                         TEXT_EDITOR_CURSOR,
                         TEXT_EDITOR_SELECTION,
                         "ABC");

    // before:
    //     abcdefghij1234567890!@#$%^&*()
    // after:
    //     abcdefghij12345ABC!@#$%^&*()

    // check paragraph contents
    text = text_paragraph_get_text (fixture->para1);
    g_assert_cmpstr (text, ==, "abcdefghij12345ABC!@#$%^&*()");
    g_free (text);

    // check reduced length
    length = text_paragraph_get_length (fixture->para1);
    g_assert_cmpint (length, ==, 28);

    // ensure only run2 was changed
    g_object_get (fixture->run1, "text", &text, NULL);
    g_assert_cmpstr (text, ==, RUN1);

    g_object_get (fixture->run2, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "12345ABC");

    g_object_get (fixture->run3, "text", &text, NULL);
    g_assert_cmpstr (text, ==, RUN3);
}

static void
test_across_runs_single (ReplaceFixture *fixture,
                         gconstpointer   user_data)
{
    gchar *text;
    int length;

    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 8);

    // TODO: Don't call this here
    fixture->doc->selection = text_document_copy_mark (fixture->doc, fixture->doc->cursor);
    text_editor_move_right (fixture->editor, TEXT_EDITOR_SELECTION, 5);
    text_editor_replace (fixture->editor,
                         TEXT_EDITOR_CURSOR,
                         TEXT_EDITOR_SELECTION,
                         "TEXT ENGINE");

    // before:
    //     abcdefghij1234567890!@#$%^&*()
    // after:
    //     abcdefghTEXT ENGINE4567890!@#$%^&*()

    // check paragraph contents
    text = text_paragraph_get_text (fixture->para1);
    g_assert_cmpstr (text, ==, "abcdefghTEXT ENGINE4567890!@#$%^&*()");
    g_free (text);

    // check increased length
    length = text_paragraph_get_length (fixture->para1);
    g_assert_cmpint (length, ==, 36);

    // ensure insertion is part of run1
    g_object_get (fixture->run1, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "abcdefghTEXT ENGINE");

    // ensure deletion only affected run2
    g_object_get (fixture->run2, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "4567890");

    // unchanged
    g_object_get (fixture->run3, "text", &text, NULL);
    g_assert_cmpstr (text, ==, RUN3);
}

static void
test_across_runs_multiple (ReplaceFixture *fixture,
                           gconstpointer   user_data)
{
    gchar *text;
    int length;

    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 8);

    // TODO: Don't call this here
    fixture->doc->selection = text_document_copy_mark (fixture->doc, fixture->doc->cursor);
    text_editor_move_right (fixture->editor, TEXT_EDITOR_SELECTION, 15);
    text_editor_replace (fixture->editor,
                         TEXT_EDITOR_CURSOR,
                         TEXT_EDITOR_SELECTION,
                         "TEXT ENGINE");

    // before:
    //     abcdefghij1234567890!@#$%^&*()
    // after:
    //     abcdefghTEXT ENGINE$%^&*()

    // check paragraph contents
    text = text_paragraph_get_text (fixture->para1);
    g_assert_cmpstr (text, ==, "abcdefghTEXT ENGINE$%^&*()");
    g_free (text);

    // check reduced length
    length = text_paragraph_get_length (fixture->para1);
    g_assert_cmpint (length, ==, 26);

    // ensure insertion is part of run1
    g_object_get (fixture->run1, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "abcdefghTEXT ENGINE");

    // ensure deletion affects run3
    g_object_get (fixture->run3, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "$%^&*()");
}

static void
test_across_paragraphs_single (ReplaceFixture *fixture,
                               gconstpointer   user_data)
{
    gchar *text;
    int length;

    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 28);

    // TODO: Don't call this here
    fixture->doc->selection = text_document_copy_mark (fixture->doc, fixture->doc->cursor);
    text_editor_move_right (fixture->editor, TEXT_EDITOR_SELECTION, 5);
    text_editor_replace (fixture->editor,
                         TEXT_EDITOR_CURSOR,
                         TEXT_EDITOR_SELECTION,
                         "TEXT ENGINE");

    // before:
    //     abcdefghij1234567890!@#$%^&*()
    //     zxcvbnm,./
    //     0987654321
    // after:
    //     abcdefghij1234567890!@#$%^&*TEXT ENGINEcvbnm,./
    //     0987654321

    // check paragraph contents
    text = text_paragraph_get_text (fixture->para1);
    g_assert_cmpstr (text, ==, "abcdefghij1234567890!@#$%^&*TEXT ENGINEcvbnm,./");
    g_free (text);

    text = text_paragraph_get_text (fixture->para3);
    g_assert_cmpstr (text, ==, "0987654321");
    g_free (text);

    // check increased length
    length = text_paragraph_get_length (fixture->para1);
    g_assert_cmpint (length, ==, 47);

    length = text_paragraph_get_length (fixture->para3);
    g_assert_cmpint (length, ==, 10);

    // ensure insertion is part of run3
    g_object_get (fixture->run3, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "!@#$%^&*TEXT ENGINE");
}

static void
test_across_paragraphs_multiple (ReplaceFixture *fixture,
                                 gconstpointer   user_data)
{
    gchar *text;
    int length;

    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 28);

    // TODO: Don't call this here
    fixture->doc->selection = text_document_copy_mark (fixture->doc, fixture->doc->cursor);
    text_editor_move_right (fixture->editor, TEXT_EDITOR_SELECTION, 15);
    text_editor_replace (fixture->editor,
                         TEXT_EDITOR_CURSOR,
                         TEXT_EDITOR_SELECTION,
                         "TEXT ENGINE");

    // before:
    //     abcdefghij1234567890!@#$%^&*()
    //     zxcvbnm,./
    //     0987654321
    // after:
    //     abcdefghij1234567890!@#$%^&*TEXT ENGINE987654321

    // check paragraph contents
    text = text_paragraph_get_text (fixture->para1);
    g_assert_cmpstr (text, ==, "abcdefghij1234567890!@#$%^&*TEXT ENGINE987654321");
    g_free (text);

    // check increased length
    length = text_paragraph_get_length (fixture->para1);
    g_assert_cmpint (length, ==, 48);

    // ensure insertion is part of run3
    g_object_get (fixture->run3, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "!@#$%^&*TEXT ENGINE");

    // ensure deletion affects run 4
    g_object_get (fixture->run5, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "987654321");
}

static void
test_document_replace (ReplaceFixture *fixture,
                       gconstpointer   user_data)
{
    gchar *text;
    int length;

    // TODO: Don't call this here
    fixture->doc->selection = text_document_copy_mark (fixture->doc, fixture->doc->cursor);
    text_editor_move_last (fixture->editor, TEXT_EDITOR_SELECTION);
    text_editor_replace (fixture->editor,
                         TEXT_EDITOR_CURSOR,
                         TEXT_EDITOR_SELECTION,
                         "TEXT ENGINE");

    // before:
    //     abcdefghij1234567890!@#$%^&*()
    //     zxcvbnm,./
    //     0987654321
    // after:
    //     TEXT ENGINE

    // check paragraph contents
    text = text_paragraph_get_text (fixture->para1);
    g_assert_cmpstr (text, ==, "TEXT ENGINE");
    g_free (text);

    // check increased length
    length = text_paragraph_get_length (fixture->para1);
    g_assert_cmpint (length, ==, 11);

    // ensure insertion is part of run1
    g_object_get (fixture->run1, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "TEXT ENGINE");
}

static void
test_document_clear (ReplaceFixture *fixture,
                     gconstpointer   user_data)
{
    gchar *text;
    int length;

    // TODO: Don't call this here
    fixture->doc->selection = text_document_copy_mark (fixture->doc, fixture->doc->cursor);
    text_editor_move_last (fixture->editor, TEXT_EDITOR_SELECTION);
    text_editor_replace (fixture->editor,
                         TEXT_EDITOR_CURSOR,
                         TEXT_EDITOR_SELECTION,
                         "");

    // before:
    //     abcdefghij1234567890!@#$%^&*()
    //     zxcvbnm,./
    //     0987654321
    // after:
    //     <empty document>

    // check paragraph one is still present
    text = text_paragraph_get_text (fixture->para1);
    g_assert_cmpstr (text, ==, "");
    g_free (text);

    // check zero length
    length = text_paragraph_get_length (fixture->para1);
    g_assert_cmpint (length, ==, 0);

    // ensure run 1 is present and blank
    g_object_get (fixture->run1, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "");
}

static void
test_equal_marks_insert (ReplaceFixture *fixture,
                         gconstpointer   user_data)
{
    gchar *text;
    int length;

    // TODO: Don't call this here
    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 5);
    fixture->doc->selection = text_document_copy_mark (fixture->doc, fixture->doc->cursor);
    text_editor_replace (fixture->editor,
                         TEXT_EDITOR_CURSOR,
                         TEXT_EDITOR_SELECTION,
                         "hello");

    // before:
    //     abcdefghij1234567890!@#$%^&*()
    // after:
    //     abcdehellofghij1234567890!@#$%^&*()

    // check paragraph contents
    text = text_paragraph_get_text (fixture->para1);
    g_assert_cmpstr (text, ==, "abcdehellofghij1234567890!@#$%^&*()");
    g_free (text);

    // check increased length
    length = text_paragraph_get_length (fixture->para1);
    g_assert_cmpint (length, ==, 35);

    // ensure insertion is part of run1
    g_object_get (fixture->run1, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "abcdehellofghij");
}

static void
test_equal_marks_nothing (ReplaceFixture *fixture,
                          gconstpointer   user_data)
{
    gchar *text;
    int length;

    // TODO: Don't call this here
    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 5);
    text_editor_replace (fixture->editor,
                         TEXT_EDITOR_CURSOR,
                         TEXT_EDITOR_CURSOR,
                         "");

    // before:
    //     abcdefghij1234567890!@#$%^&*()
    // after:
    //     abcdefghij1234567890!@#$%^&*()

    // contents unchanged
    text = text_paragraph_get_text (fixture->para1);
    g_assert_cmpstr (text, ==, "abcdefghij1234567890!@#$%^&*()");
    g_free (text);

    // length unchanged
    length = text_paragraph_get_length (fixture->para1);
    g_assert_cmpint (length, ==, 30);
}

static void
test_reverse (ReplaceFixture *fixture,
              gconstpointer   user_data)
{
    // NOTE: Keep same as `test_across_paragraphs_single` but
    // with marks reversed (i.e. SELECTION before CURSOR)

    gchar *text;
    int length;

    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 28);

    // TODO: Don't call this here
    fixture->doc->selection = text_document_copy_mark (fixture->doc, fixture->doc->cursor);
    text_editor_move_right (fixture->editor, TEXT_EDITOR_SELECTION, 5);
    text_editor_replace (fixture->editor,
                         TEXT_EDITOR_SELECTION,
                         TEXT_EDITOR_CURSOR,
                         "TEXT ENGINE");

    // before:
    //     abcdefghij1234567890!@#$%^&*()
    //     zxcvbnm,./
    //     0987654321
    // after:
    //     abcdefghij1234567890!@#$%^&*TEXT ENGINEcvbnm,./
    //     0987654321

    // check paragraph contents
    text = text_paragraph_get_text (fixture->para1);
    g_assert_cmpstr (text, ==, "abcdefghij1234567890!@#$%^&*TEXT ENGINEcvbnm,./");
    g_free (text);

    text = text_paragraph_get_text (fixture->para3);
    g_assert_cmpstr (text, ==, "0987654321");
    g_free (text);

    // check increased length
    length = text_paragraph_get_length (fixture->para1);
    g_assert_cmpint (length, ==, 47);

    length = text_paragraph_get_length (fixture->para3);
    g_assert_cmpint (length, ==, 10);

    // ensure insertion is part of run3
    g_object_get (fixture->run3, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "!@#$%^&*TEXT ENGINE");
}

int
main (int argc, char *argv[])
{
    setlocale (LC_ALL, "");

    g_test_init (&argc, &argv, NULL);

    // Define the tests.
    g_test_add ("/text-engine/editor/replace/test-within-run", ReplaceFixture, NULL,
                replace_fixture_set_up, test_within_run,
                replace_fixture_tear_down);
    g_test_add ("/text-engine/editor/replace/test-across-runs-single", ReplaceFixture, NULL,
                replace_fixture_set_up, test_across_runs_single,
                replace_fixture_tear_down);
    g_test_add ("/text-engine/editor/replace/test-across-runs-multiple", ReplaceFixture, NULL,
                replace_fixture_set_up, test_across_runs_multiple,
                replace_fixture_tear_down);
    g_test_add ("/text-engine/editor/replace/test-across-paragraphs-single", ReplaceFixture, NULL,
                replace_fixture_set_up, test_across_paragraphs_single,
                replace_fixture_tear_down);
    g_test_add ("/text-engine/editor/replace/test-across-paragraphs-multiple", ReplaceFixture, NULL,
                replace_fixture_set_up, test_across_paragraphs_multiple,
                replace_fixture_tear_down);
    g_test_add ("/text-engine/editor/replace/test-document-replace", ReplaceFixture, NULL,
                replace_fixture_set_up, test_document_replace,
                replace_fixture_tear_down);
    g_test_add ("/text-engine/editor/replace/test-document-clear", ReplaceFixture, NULL,
                replace_fixture_set_up, test_document_clear,
                replace_fixture_tear_down);
    g_test_add ("/text-engine/editor/replace/test-equal-marks-insert", ReplaceFixture, NULL,
                replace_fixture_set_up, test_equal_marks_insert,
                replace_fixture_tear_down);
    g_test_add ("/text-engine/editor/replace/test-equal-marks-nothing", ReplaceFixture, NULL,
                replace_fixture_set_up, test_equal_marks_nothing,
                replace_fixture_tear_down);
    g_test_add ("/text-engine/editor/replace/test-reverse", ReplaceFixture, NULL,
                replace_fixture_set_up, test_reverse,
                replace_fixture_tear_down);

    return g_test_run ();
}

