/* move.c
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
} MoveFixture;

#define RUN1 "Once upon a time there was a little dog, "
#define RUN2 "AND HIS NAME WAS ROVER."
#define RUN3 "By J. R. R. Tolkien"
#define RUN4 "Roverandom, 1920s"

static void
move_fixture_set_up_single (MoveFixture   *fixture,
                            gconstpointer  user_data)
{
    TextFrame *frame;
    TextParagraph *para1;
    TextRun *run1;

    frame = text_frame_new ();

    para1 = text_paragraph_new ();
    run1 = text_run_new (RUN1);
    text_paragraph_append_inline(para1, run1);
    text_frame_append_block (frame, TEXT_BLOCK (para1));

    fixture->doc = text_document_new ();
    fixture->doc->frame = frame;

    fixture->editor = text_editor_new (fixture->doc);

    text_editor_move_first (fixture->editor, TEXT_EDITOR_CURSOR);

    fixture->run1 = run1;
    fixture->run2 = NULL;
    fixture->run3 = NULL;
    fixture->run4 = NULL;
}

#define RUN5 "This is some text that is pa"
#define RUN6 "RT OF TWO DIFFE"
#define RUN7 "rent runs"

static void
move_fixture_set_up_runs (MoveFixture   *fixture,
                          gconstpointer  user_data)
{
    TextFrame *frame;
    TextParagraph *para1;
    TextRun *run1, *run2, *run3;

    frame = text_frame_new ();

    para1 = text_paragraph_new ();
    run1 = text_run_new (RUN5);
    run2 = text_run_new (RUN6);
    run3 = text_run_new (RUN7);
    text_paragraph_append_inline(para1, run1);
    text_paragraph_append_inline(para1, run2);
    text_paragraph_append_inline(para1, run3);
    text_frame_append_block (frame, TEXT_BLOCK (para1));

    fixture->doc = text_document_new ();
    fixture->doc->frame = frame;

    fixture->editor = text_editor_new (fixture->doc);

    text_editor_move_first (fixture->editor, TEXT_EDITOR_CURSOR);

    fixture->run1 = run1;
    fixture->run2 = run2;
    fixture->run3 = run3;
    fixture->run4 = NULL;
}

static void
move_fixture_set_up_paragraphs (MoveFixture   *fixture,
                                gconstpointer  user_data)
{
    TextFrame *frame;
    TextParagraph *para1, *para2, *para3;
    TextRun *run1, *run2, *run3, *run4;

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

    para3 = text_paragraph_new ();
    run4 = text_run_new (RUN4);
    text_paragraph_append_inline(para3, run4);
    text_frame_append_block (frame, TEXT_BLOCK (para3));

    fixture->doc = text_document_new ();
    fixture->doc->frame = frame;

    fixture->editor = text_editor_new (fixture->doc);

    text_editor_move_first (fixture->editor, TEXT_EDITOR_CURSOR);

    fixture->run1 = run1;
    fixture->run2 = run2;
    fixture->run3 = run3;
    fixture->run4 = run4;
}

static void
move_fixture_tear_down (MoveFixture   *fixture,
                        gconstpointer  user_data)
{
    g_object_unref (fixture->editor);
    g_object_unref (fixture->doc);
}

static void
test_left_guard (MoveFixture   *fixture,
                 gconstpointer  user_data)
{
    int amount;
    amount = (int)user_data;

    // test moving left at the start of the document
    text_editor_move_left (fixture->editor, TEXT_EDITOR_CURSOR, amount);

    // cursor position unchanged
    g_assert_cmpint (fixture->doc->cursor->index, ==, 0);
}

static void
test_right_guard (MoveFixture   *fixture,
                  gconstpointer  user_data)
{
    int amount;
    amount = (int)user_data;

    // go to end
    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 41);

    // test moving right at the end of the document
    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, amount);

    // cursor position unchanged
    g_assert_cmpint (fixture->doc->cursor->index, ==, 41);
}

// We have three runs in a single paragraph. Capitalisation indicates
// a different run for these test cases:
//
//                                   index 28
//                                 /
//     `This is some text that is paRT OF TWO DIFFErent runs`
//                                  ^
//                         index 29 /
//
// When traversing leftwards from index 29, we cross into a new
// run at index 28.

static void
test_left_traversal_across_run (MoveFixture   *fixture,
                                gconstpointer  user_data)
{
    int amount;
    amount = (int)user_data;

    // go to index 29 (run two)
    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 29);
    g_assert_true (text_editor_get_run (fixture->editor, TEXT_EDITOR_CURSOR) == fixture->run2);

    // test moving left by amount
    text_editor_move_left (fixture->editor, TEXT_EDITOR_CURSOR, amount);
    g_assert_true (text_editor_get_run (fixture->editor, TEXT_EDITOR_CURSOR) == fixture->run1);

    g_assert_cmpint (fixture->doc->cursor->index, ==, 29 - amount);
}

static void
test_right_traversal_across_run (MoveFixture   *fixture,
                                 gconstpointer  user_data)
{
    int amount;
    amount = (int)user_data;

    // go to index 28 (run one)
    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 28);
    g_assert_true (text_editor_get_run (fixture->editor, TEXT_EDITOR_CURSOR) == fixture->run1);

    // test moving right by amount
    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, amount);
    g_assert_true (text_editor_get_run (fixture->editor, TEXT_EDITOR_CURSOR) == fixture->run2);

    g_assert_cmpint (fixture->doc->cursor->index, ==, 28 + amount);
}

// We have four runs across three paragraphs. Again, capitalisation
// represents the start of a new run.
//
//                             first index in run 2              index 64
//                                                 \                     \
//   p1: `Once upon a time there was a little dog, AND HIS NAME WAS ROVER.`
//   p2: `By J. R. R. Tolkien`
//   p3: `Roverandom, 1920s`
//
// The end of p1 is index 64, after the full stop. The start of p2 is
// naturally index 0. When traversing one character from index 64, the
// cursor should move to index 0. Traversing 10 characters would move to
// index 9, etc.
//
// Traversing 21 characters should place the cursor at p3 index 0.

static void
test_left_traversal_across_paragraph (MoveFixture   *fixture,
                                      gconstpointer  user_data)
{
    int amount;
    amount = (int)user_data;

    // move to start of p2 (run3)
    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 65);
    g_assert_true (text_editor_get_run (fixture->editor, TEXT_EDITOR_CURSOR) == fixture->run3);
    g_assert_cmpint (fixture->doc->cursor->index, ==, 0);

    // move backwards by amount
    text_editor_move_left (fixture->editor, TEXT_EDITOR_CURSOR, amount);
    g_assert_true (text_editor_get_run (fixture->editor, TEXT_EDITOR_CURSOR) == fixture->run2);

    // check index
    g_assert_cmpint (fixture->doc->cursor->index, ==, (64 - (amount - 1)));
}

static void
test_right_traversal_across_paragraph (MoveFixture   *fixture,
                                       gconstpointer  user_data)
{
    int amount;
    amount = (int)user_data;

    // move to end of p1 (run2)
    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 64);
    g_assert_true (text_editor_get_run (fixture->editor, TEXT_EDITOR_CURSOR) == fixture->run2);

    // move forwards by amount
    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, amount);
    g_assert_true (text_editor_get_run (fixture->editor, TEXT_EDITOR_CURSOR) == fixture->run3);

    // check index
    g_assert_cmpint (fixture->doc->cursor->index, ==, amount - 1);
}

static void
test_left_traversal_across_several_paragraphs (MoveFixture   *fixture,
                                               gconstpointer  user_data)
{
    // move to start of p3 (run4)
    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 85);
    g_assert_true (text_editor_get_run (fixture->editor, TEXT_EDITOR_CURSOR) == fixture->run4);

    // move left by 62 characters (to run1)
    text_editor_move_left (fixture->editor, TEXT_EDITOR_CURSOR, 62);
    g_assert_true (text_editor_get_run (fixture->editor, TEXT_EDITOR_CURSOR) == fixture->run1);

    // check index is 23 in p1
    g_assert_cmpint (fixture->doc->cursor->index, ==, 23);
}

static void
test_right_traversal_across_several_paragraphs (MoveFixture   *fixture,
                                                gconstpointer  user_data)
{
    // move to p3, index 2 (run4)
    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 87);
    g_assert_true (text_editor_get_run (fixture->editor, TEXT_EDITOR_CURSOR) == fixture->run4);

    // check index is 2 in p3
    g_assert_cmpint (fixture->doc->cursor->index, ==, 2);
}

static void
test_balanced_traversal (MoveFixture   *fixture,
                         gconstpointer  user_data)
{
    int amount;
    int old_index;
    TextParagraph *old_paragraph;

    amount = (int)user_data;

    old_index = fixture->doc->cursor->index;
    old_paragraph = fixture->doc->cursor->paragraph;

    // ensure that equal left and right movements return to the same position
    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, amount);
    text_editor_move_left (fixture->editor, TEXT_EDITOR_CURSOR, amount);

    g_assert_true (old_index == fixture->doc->cursor->index);
    g_assert_true (old_paragraph == fixture->doc->cursor->paragraph);
}

int
main (int argc, char *argv[])
{
    setlocale (LC_ALL, "");

    g_test_init (&argc, &argv, NULL);

    // Document guard tests
    g_test_add ("/text-engine/editor/move/test-left-guard-one", MoveFixture, (void*)1,
                move_fixture_set_up_single, test_left_guard,
                move_fixture_tear_down);
    g_test_add ("/text-engine/editor/move/test-left-guard-ten", MoveFixture, (void*)10,
                move_fixture_set_up_single, test_left_guard,
                move_fixture_tear_down);
    g_test_add ("/text-engine/editor/move/test-right-guard-one", MoveFixture, (void*)1,
                move_fixture_set_up_single, test_right_guard,
                move_fixture_tear_down);
    g_test_add ("/text-engine/editor/move/test-right-guard-ten", MoveFixture, (void*)10,
                move_fixture_set_up_single, test_right_guard,
                move_fixture_tear_down);

    // Run boundary tests
    g_test_add ("/text-engine/editor/move/test-left-traversal-across-run-one", MoveFixture, (void*)1,
                move_fixture_set_up_runs, test_left_traversal_across_run,
                move_fixture_tear_down);
    g_test_add ("/text-engine/editor/move/test-left-traversal-across-run-ten", MoveFixture, (void*)10,
                move_fixture_set_up_runs, test_left_traversal_across_run,
                move_fixture_tear_down);
    g_test_add ("/text-engine/editor/move/test-right-traversal-across-run-one", MoveFixture, (void*)1,
                move_fixture_set_up_runs, test_right_traversal_across_run,
                move_fixture_tear_down);
    g_test_add ("/text-engine/editor/move/test-right-traversal-across-run-ten", MoveFixture, (void*)10,
                move_fixture_set_up_runs, test_right_traversal_across_run,
                move_fixture_tear_down);

    // Paragraph boundary tests
    g_test_add ("/text-engine/editor/move/test-left-traversal-across-paragraph-one", MoveFixture, (void*)1,
                move_fixture_set_up_paragraphs, test_left_traversal_across_paragraph,
                move_fixture_tear_down);
    g_test_add ("/text-engine/editor/move/test-right-traversal-across-paragraph-one", MoveFixture, (void*)1,
                move_fixture_set_up_paragraphs, test_right_traversal_across_paragraph,
                move_fixture_tear_down);
    g_test_add ("/text-engine/editor/move/test-left-traversal-across-paragraph-five", MoveFixture, (void*)5,
                move_fixture_set_up_paragraphs, test_left_traversal_across_paragraph,
                move_fixture_tear_down);
    g_test_add ("/text-engine/editor/move/test-right-traversal-across-paragraph-five", MoveFixture, (void*)5,
                move_fixture_set_up_paragraphs, test_right_traversal_across_paragraph,
                move_fixture_tear_down);

    // Multi-paragraph boundary tests
    g_test_add ("/text-engine/editor/move/test-left-traversal-across-several-paragraphs", MoveFixture, NULL,
                move_fixture_set_up_paragraphs, test_left_traversal_across_several_paragraphs,
                move_fixture_tear_down);
    g_test_add ("/text-engine/editor/move/test-right-traversal-across-several-paragraphs", MoveFixture, NULL,
                move_fixture_set_up_paragraphs, test_right_traversal_across_several_paragraphs,
                move_fixture_tear_down);

    // Balance tests
    g_test_add ("/text-engine/editor/move/test-balanced-traversal-one", MoveFixture, (void*)1,
                move_fixture_set_up_paragraphs, test_balanced_traversal,
                move_fixture_tear_down);
    g_test_add ("/text-engine/editor/move/test-balanced-traversal-five", MoveFixture, (void*)5,
                move_fixture_set_up_paragraphs, test_balanced_traversal,
                move_fixture_tear_down);
    g_test_add ("/text-engine/editor/move/test-balanced-traversal-ten", MoveFixture, (void*)10,
                move_fixture_set_up_paragraphs, test_balanced_traversal,
                move_fixture_tear_down);
    g_test_add ("/text-engine/editor/move/test-balanced-traversal-fifty", MoveFixture, (void*)50,
                move_fixture_set_up_paragraphs, test_balanced_traversal,
                move_fixture_tear_down);
    g_test_add ("/text-engine/editor/move/test-balanced-traversal-hundred", MoveFixture, (void*)100,
                move_fixture_set_up_paragraphs, test_balanced_traversal,
                move_fixture_tear_down);


    return g_test_run ();
}

