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
}

pub fn get_all_text<'a>(document: &Document) -> String {
    let mut string_builder = String::new();
    for run in &document.runs {
        let text = get_text(document, &run);
        string_builder.push_str(text);
    }
    string_builder
}

pub fn get_text<'a>(document: &'a Document, run: &Run) -> &'a str {
    let range = run.start_index..run.end_index;
    match run.append_buffer {
        true => &document.append[range],
        false => &document.buffer[range]
    }
}

pub fn insert(document: &mut Document, index: usize, text: &str) {
    let append_buffer_index = document.append.len();
    document.append.push_str(text);

    let run_to_split = document.runs.iter_mut()
        .position(|run| run.start_index <= index && run.end_index > index);

    if let Some(run_index) = run_to_split {

        let (split_index, end_index, append_buffer) = {
            let run = document.runs.get(run_index).unwrap();

            let index_within_run = index - run.start_index;
            let split_index = run.start_index + index_within_run;

            (split_index, run.end_index, run.append_buffer)
        };

        if let Some(run) = document.runs.get_mut(run_index) {
            run.end_index = split_index;
        }

        let mut insert_run = Run {
            start_index: append_buffer_index,
            end_index: append_buffer_index + text.len(),
            append_buffer: true
        };
        document.runs.insert(run_index + 1, insert_run);

        let mut after_run = Run {
            start_index: split_index,
            end_index,
            append_buffer: append_buffer
        };
        document.runs.insert(run_index + 2, after_run);

        return
    }

    let mut new_run = Run {
        start_index: append_buffer_index,
        end_index: append_buffer_index + text.len(),
        append_buffer: true
    };

    document.runs.push(new_run);
}