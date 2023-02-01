/* model.rs
 *
 * Copyright 2023 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is dual-licensed under the terms of the Mozilla Public
 * License 2.0 and the Lesser General Public License 2.1 (or any
 * later version).
 *
 * SPDX-License-Identifier: MPL-2.0 OR LGPL-2.1-or-later
 */

use std::collections::LinkedList;
use std::fmt::{Display, Formatter};

pub struct Run {
    pub(crate) start_index : usize,
    pub(crate) end_index : usize,
    pub(crate) append_buffer : bool
}

impl Run {
    pub fn length(&self) -> usize {
        self.end_index - self.start_index
    }
}

pub struct Document {
    pub(crate) buffer : String,
    pub(crate) append : String,
    runs: Vec<Run>
}

impl Document {
    pub fn new(initial: Option<&str>) -> Document {
        if let Some(initial) = initial {
            let run = Run {
                start_index: 0,
                end_index: initial.len(),
                append_buffer: false
            };

            return Document {
                buffer: String::from(initial),
                append: String::new(),
                runs: vec![run]
            }
        }

        Document {
            buffer: String::new(),
            append: String::new(),
            runs: vec![]
        }
    }

    pub fn insert(&mut self, byte_index: usize, text: &str) {
        let append_buffer_index = self.append.len();
        self.append.push_str(text);

        let run_to_split = self.runs.iter_mut()
            .position(|run| run.start_index <= byte_index && run.end_index > byte_index);

        if let Some(run_index) = run_to_split {

            let (split_index, end_index, append_buffer) = {
                let run = self.runs.get(run_index).unwrap();

                let index_within_run = byte_index - run.start_index;
                let split_index = run.start_index + index_within_run;

                (split_index, run.end_index, run.append_buffer)
            };

            if let Some(run) = self.runs.get_mut(run_index) {
                run.end_index = split_index;
            }

            let mut insert_run = Run {
                start_index: append_buffer_index,
                end_index: append_buffer_index + text.len(),
                append_buffer: true
            };
            self.runs.insert(run_index + 1, insert_run);

            let mut after_run = Run {
                start_index: split_index,
                end_index,
                append_buffer: append_buffer
            };
            self.runs.insert(run_index + 2, after_run);

            return
        }

        let mut new_run = Run {
            start_index: append_buffer_index,
            end_index: append_buffer_index + text.len(),
            append_buffer: true
        };

        self.runs.push(new_run);
    }

    pub fn get_run_at_index(&self, index: usize) -> Option<&Run> {
        self.runs.iter()
            .find(|run| run.start_index <= index && run.end_index > index)
    }

    pub fn get_all_text<'a>(&self) -> String {
        let mut string_builder = String::new();
        for run in &self.runs {
            let text = self.get_text(&run);
            string_builder.push_str(text);
        }
        string_builder
    }

    pub fn get_text(&self, run: &Run) -> &str {
        let range = run.start_index..run.end_index;
        match run.append_buffer {
            true => &self.append[range],
            false => &self.buffer[range]
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::model::Document;

    #[test]
    fn test_insert_at_start() {
        let mut document = Document::new(Some("Hell🌍 World"));
        document.insert(0, "Prefix: ");
        assert_eq!(document.get_all_text(), "Prefix: Hell🌍 World");
    }

    #[test]
    fn test_insert_at_end() {
        let mut document = Document::new(Some("Hell🌍 World"));
        document.insert(15, " (Suffix)");
        assert_eq!(document.get_all_text(), "Hell🌍 World (Suffix)");
    }

    #[test]
    fn test_insert_middle() {
        let mut document = Document::new(Some("Hell🌍 World"));
        document.insert(8, " 🎶🇫🇷😔");
        assert_eq!(document.get_all_text(), "Hell🌍 🎶🇫🇷😔 World");
    }
}